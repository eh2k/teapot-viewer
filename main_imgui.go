package main

import (
	"log"
	"unsafe"
	"github.com/go-gl/gl/v2.1/gl"
	"github.com/inkyblackness/imgui-go"
)

func Render(displaySize [2]float32, framebufferSize [2]float32, drawData imgui.DrawData) {
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	displayWidth, displayHeight := displaySize[0], displaySize[1]
	fbWidth, fbHeight := framebufferSize[0], framebufferSize[1]
	if (fbWidth <= 0) || (fbHeight <= 0) {
		return
	}
	drawData.ScaleClipRects(imgui.Vec2{
		X: fbWidth / displayWidth,
		Y: fbHeight / displayHeight,
	})

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers, polygon fill.
	var lastTexture int32
	gl.GetIntegerv(gl.TEXTURE_BINDING_2D, &lastTexture)
	var lastPolygonMode [2]int32
	gl.GetIntegerv(gl.POLYGON_MODE, &lastPolygonMode[0])
	var lastViewport [4]int32
	gl.GetIntegerv(gl.VIEWPORT, &lastViewport[0])
	var lastScissorBox [4]int32
	gl.GetIntegerv(gl.SCISSOR_BOX, &lastScissorBox[0])
	gl.PushAttrib(gl.ENABLE_BIT | gl.COLOR_BUFFER_BIT | gl.TRANSFORM_BIT)
	gl.Enable(gl.BLEND)
	gl.BlendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
	gl.Disable(gl.CULL_FACE)
	gl.Disable(gl.DEPTH_TEST)
	gl.Disable(gl.LIGHTING)
	gl.Disable(gl.COLOR_MATERIAL)
	gl.Enable(gl.SCISSOR_TEST)
	gl.EnableClientState(gl.VERTEX_ARRAY)
	gl.EnableClientState(gl.TEXTURE_COORD_ARRAY)
	gl.EnableClientState(gl.COLOR_ARRAY)
	gl.Enable(gl.TEXTURE_2D)
	gl.PolygonMode(gl.FRONT_AND_BACK, gl.FILL)

	// You may want this if using this code in an OpenGL 3+ context where shaders may be bound
	gl.UseProgram(0)

	// Setup viewport, orthographic projection matrix
	// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right).
	// DisplayMin is typically (0,0) for single viewport apps.
	gl.Viewport(0, 0, int32(fbWidth), int32(fbHeight))
	gl.MatrixMode(gl.PROJECTION)
	gl.PushMatrix()
	gl.LoadIdentity()
	gl.Ortho(0, float64(displayWidth), float64(displayHeight), 0, -1, 1)
	gl.MatrixMode(gl.MODELVIEW)
	gl.PushMatrix()
	gl.LoadIdentity()

	vertexSize, vertexOffsetPos, vertexOffsetUv, vertexOffsetCol := imgui.VertexBufferLayout()
	indexSize := imgui.IndexBufferLayout()

	drawType := gl.UNSIGNED_SHORT
	if indexSize == 4 {
		drawType = gl.UNSIGNED_INT
	}

	// Render command lists
	for _, commandList := range drawData.CommandLists() {
		vertexBuffer, _ := commandList.VertexBuffer()
		indexBuffer, _ := commandList.IndexBuffer()
		indexBufferOffset := uintptr(indexBuffer)

		gl.VertexPointer(2, gl.FLOAT, int32(vertexSize), unsafe.Pointer(uintptr(vertexBuffer)+uintptr(vertexOffsetPos)))
		gl.TexCoordPointer(2, gl.FLOAT, int32(vertexSize), unsafe.Pointer(uintptr(vertexBuffer)+uintptr(vertexOffsetUv)))
		gl.ColorPointer(4, gl.UNSIGNED_BYTE, int32(vertexSize), unsafe.Pointer(uintptr(vertexBuffer)+uintptr(vertexOffsetCol)))

		for _, command := range commandList.Commands() {
			if command.HasUserCallback() {
				command.CallUserCallback(commandList)
			} else {
				clipRect := command.ClipRect()
				gl.Scissor(int32(clipRect.X), int32(fbHeight)-int32(clipRect.W), int32(clipRect.Z-clipRect.X), int32(clipRect.W-clipRect.Y))
				gl.BindTexture(gl.TEXTURE_2D, uint32(command.TextureID()))
				gl.DrawElements(gl.TRIANGLES, int32(command.ElementCount()), uint32(drawType), unsafe.Pointer(indexBufferOffset))
			}

			indexBufferOffset += uintptr(command.ElementCount() * indexSize)
		}
	}

	err := gl.GetError()
	if err != 0 {
		log.Panic(err)
	}
	// Restore modified state
	gl.DisableClientState(gl.COLOR_ARRAY)
	gl.DisableClientState(gl.TEXTURE_COORD_ARRAY)
	gl.DisableClientState(gl.VERTEX_ARRAY)
	gl.BindTexture(gl.TEXTURE_2D, uint32(lastTexture))
	gl.MatrixMode(gl.MODELVIEW)
	gl.PopMatrix()
	gl.MatrixMode(gl.PROJECTION)
	gl.PopMatrix()
	gl.PopAttrib()
	gl.PolygonMode(gl.FRONT, uint32(lastPolygonMode[0]))
	gl.PolygonMode(gl.BACK, uint32(lastPolygonMode[1]))
	gl.Viewport(lastViewport[0], lastViewport[1], lastViewport[2], lastViewport[3])
	gl.Scissor(lastScissorBox[0], lastScissorBox[1], lastScissorBox[2], lastScissorBox[3])
}

func CreateFontsTexture(io imgui.IO) uint32 {
	// Build texture atlas
	image := io.Fonts().TextureDataRGBA32()

	// Upload texture to graphics system
	var lastTexture int32
	var fontTexture uint32
	gl.GetIntegerv(gl.TEXTURE_BINDING_2D, &lastTexture)
	gl.GenTextures(1, &fontTexture)
	gl.BindTexture(gl.TEXTURE_2D, fontTexture)
	gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
	gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR)
	gl.PixelStorei(gl.UNPACK_ROW_LENGTH, 0)
	gl.TexImage2D(gl.TEXTURE_2D, 0, gl.RGBA, int32(image.Width), int32(image.Height), 0, gl.RGBA, gl.UNSIGNED_BYTE, image.Pixels)

	// Store our identifier
	io.Fonts().SetTextureID(imgui.TextureID(fontTexture))

	// Restore state
	gl.BindTexture(gl.TEXTURE_2D, uint32(lastTexture))

	return fontTexture
}

func DestroyFontsTexture(fontTexture uint32) {
	if fontTexture != 0 {
		gl.DeleteTextures(1, &fontTexture)
		imgui.CurrentIO().Fonts().SetTextureID(0)
		fontTexture = 0
	}
}

func InitImguiStyle() {

	//https://github.com/inkyblackness/hacked/blob/b8ec8e13df6f9dc4a416d4b375139e78d8120035/editor/Application.go

	imgui.CurrentStyle().ScaleAllSizes(1)

	style := imgui.CurrentStyle()
	style.SetColor(imgui.StyleColorText, imgui.Vec4{0.00, 0.00, 0.00, 1.00})
	style.SetColor(imgui.StyleColorTextDisabled, imgui.Vec4{0.60, 0.60, 0.60, 1.00})
	style.SetColor(imgui.StyleColorWindowBg, imgui.Vec4{0.94, 0.94, 0.94, 1.00})
	style.SetColor(imgui.StyleColorChildBg, imgui.Vec4{0.00, 0.00, 0.00, 0.00})
	style.SetColor(imgui.StyleColorPopupBg, imgui.Vec4{1.00, 1.00, 1.00, 0.98})
	style.SetColor(imgui.StyleColorBorder, imgui.Vec4{0.00, 0.00, 0.00, 0.30})
	style.SetColor(imgui.StyleColorBorderShadow, imgui.Vec4{0.00, 0.00, 0.00, 0.00})
	style.SetColor(imgui.StyleColorFrameBg, imgui.Vec4{1.00, 1.00, 1.00, 1.00})
	style.SetColor(imgui.StyleColorFrameBgHovered, imgui.Vec4{0.26, 0.59, 0.98, 0.40})
	style.SetColor(imgui.StyleColorFrameBgActive, imgui.Vec4{0.26, 0.59, 0.98, 0.67})
	style.SetColor(imgui.StyleColorTitleBg, imgui.Vec4{0.96, 0.96, 0.96, 1.00})
	style.SetColor(imgui.StyleColorTitleBgActive, imgui.Vec4{0.82, 0.82, 0.82, 1.00})
	style.SetColor(imgui.StyleColorTitleBgCollapsed, imgui.Vec4{1.00, 1.00, 1.00, 0.51})
	style.SetColor(imgui.StyleColorMenuBarBg, imgui.Vec4{0.86, 0.86, 0.86, 1.00})
	style.SetColor(imgui.StyleColorScrollbarBg, imgui.Vec4{0.98, 0.98, 0.98, 0.53})
	style.SetColor(imgui.StyleColorScrollbarGrab, imgui.Vec4{0.69, 0.69, 0.69, 0.80})
	style.SetColor(imgui.StyleColorScrollbarGrabHovered, imgui.Vec4{0.49, 0.49, 0.49, 0.80})
	style.SetColor(imgui.StyleColorScrollbarGrabActive, imgui.Vec4{0.49, 0.49, 0.49, 1.00})
	style.SetColor(imgui.StyleColorCheckMark, imgui.Vec4{0.26, 0.59, 0.98, 1.00})
	style.SetColor(imgui.StyleColorSliderGrab, imgui.Vec4{0.26, 0.59, 0.98, 0.78})
	style.SetColor(imgui.StyleColorSliderGrabActive, imgui.Vec4{0.46, 0.54, 0.80, 0.60})
	style.SetColor(imgui.StyleColorButton, imgui.Vec4{0.26, 0.59, 0.98, 0.40})
	style.SetColor(imgui.StyleColorButtonHovered, imgui.Vec4{0.26, 0.59, 0.98, 1.00})
	style.SetColor(imgui.StyleColorButtonActive, imgui.Vec4{0.06, 0.53, 0.98, 1.00})
	style.SetColor(imgui.StyleColorHeader, imgui.Vec4{0.26, 0.59, 0.98, 0.31})
	style.SetColor(imgui.StyleColorHeaderHovered, imgui.Vec4{0.26, 0.59, 0.98, 0.80})
	style.SetColor(imgui.StyleColorTabHovered, imgui.Vec4{0.26, 0.59, 0.98, 0.80})
	style.SetColor(imgui.StyleColorNavHighlight, imgui.Vec4{0.26, 0.59, 0.98, 0.80})
	style.SetColor(imgui.StyleColorHeaderActive, imgui.Vec4{0.26, 0.59, 0.98, 1.00})
	style.SetColor(imgui.StyleColorSeparator, imgui.Vec4{0.39, 0.39, 0.39, 0.62})
	style.SetColor(imgui.StyleColorSeparatorHovered, imgui.Vec4{0.14, 0.44, 0.80, 0.78})
	style.SetColor(imgui.StyleColorSeparatorActive, imgui.Vec4{0.14, 0.44, 0.80, 1.00})
	style.SetColor(imgui.StyleColorResizeGrip, imgui.Vec4{0.80, 0.80, 0.80, 0.56})
	style.SetColor(imgui.StyleColorResizeGripHovered, imgui.Vec4{0.26, 0.59, 0.98, 0.67})
	style.SetColor(imgui.StyleColorResizeGripActive, imgui.Vec4{0.26, 0.59, 0.98, 0.95})
	style.SetColor(imgui.StyleColorPlotLines, imgui.Vec4{0.39, 0.39, 0.39, 1.00})
	style.SetColor(imgui.StyleColorPlotLinesHovered, imgui.Vec4{1.00, 0.43, 0.35, 1.00})
	style.SetColor(imgui.StyleColorPlotHistogram, imgui.Vec4{0.90, 0.70, 0.00, 1.00})
	style.SetColor(imgui.StyleColorPlotHistogramHovered, imgui.Vec4{1.00, 0.45, 0.00, 1.00})
	style.SetColor(imgui.StyleColorTextSelectedBg, imgui.Vec4{0.26, 0.59, 0.98, 0.35})
	style.SetColor(imgui.StyleColorDragDropTarget, imgui.Vec4{0.26, 0.59, 0.98, 0.95})
	style.SetColor(imgui.StyleColorNavWindowingHighlight, imgui.Vec4{0.70, 0.70, 0.70, 0.70})
	//style.SetColor(imgui.StylTab,                    = ImLerp(style.SetColor(imgui.StylHeader,,       style.SetColor(imgui.StylTitleBgActive,, 0.90})
	//style.SetColor(imgui.StylTabActive,              = ImLerp(style.SetColor(imgui.StylHeaderActive,, style.SetColor(imgui.StylTitleBgActive,, 0.60})
	//style.SetColor(imgui.StylTabUnfocused,           = ImLerp(style.SetColor(imgui.StylTab,,          style.SetColor(imgui.StylTitleBg,, 0.80})
	//style.SetColor(imgui.StylTabUnfocusedActive,     = ImLerp(style.SetColor(imgui.StylTabActive,,    style.SetColor(imgui.StylTitleBg,, 0.40})
	//style.SetColor(imgui.StyleColorNavWindowingDimBg,      imgui.Vec4{0.20, 0.20, 0.20, 0.20})
	//style.SetColor(imgui.StyleColorModalWindowDimBg,       imgui.Vec4{0.20, 0.20, 0.20, 0.35})
}

func HyperLink(name string, url string, onClick func(url string)) {

	// AddUnderLine := func(color imgui.PackedColor) {
	// 	min := imgui.CalcItemWidth()
	// 	max := imgui.GetItemRectMax()
	// 	min.y = max.y
	// 	imgui.WindowDrawList().AddLine(min, max, color, 1.0)
	// }

	imgui.PushStyleColor(imgui.StyleColorText, imgui.Vec4{0.5, 0.5, 1, 1})
	imgui.Text(name)
	imgui.PopStyleColor()
	if imgui.IsItemHovered() {
		if imgui.IsMouseClicked(0) {
			onClick(url)
		}
		//AddUnderLine(imgui.PackedColor(0x0000ff))
		imgui.SetTooltip("Open in browser... ")
	} else {
		//AddUnderLine(imgui.PackedColor(0x0000ff))
	}

}