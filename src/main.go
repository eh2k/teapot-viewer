//go build -v -ldflags=all='-H windowsgui -s -w' -o ../bin/TeapotViewer.exe

package main
 
// // #cgo CFLAGS: -static
// #cgo CXXFLAGS: -std=c++17
// #cgo LDFLAGS: -static -static-libgcc -static-libstdc++ -lopengl32 -lminizip -lz
// #include <stdlib.h>
// #include "main.h"
import "C"
import (
	"bytes"
	"fmt"
	"image"
	"image/jpeg"
	"io"
	"log"
	"math"
	"net/http"

	"os"
	"os/exec"
	"runtime"
	"strconv"
	"unsafe"

	"github.com/go-gl/gl/v2.1/gl"
	"github.com/go-gl/glfw/v3.3/glfw"
	"github.com/inkyblackness/imgui-go"
	"github.com/sqweek/dialog"
)

func init() {
	// This is needed to arrange that main() runs on main thread.
	// See documentation for functions that are only allowed to be called from the main thread.
	runtime.LockOSThread()
}

func writeImage(w http.ResponseWriter, img *image.Image) {

	buffer := new(bytes.Buffer)
	if err := jpeg.Encode(buffer, *img, nil); err != nil {
		log.Println("unable to encode image.")
	}

	w.Header().Set("Content-Type", "image/jpeg")
	w.Header().Set("Content-Length", strconv.Itoa(len(buffer.Bytes())))
	if _, err := w.Write(buffer.Bytes()); err != nil {
		log.Println("unable to write image.")
	}
}

func decodeRGB(r io.Reader, width int, height int) (image.Image, error) {
	rgba := image.NewRGBA(image.Rect(0, 0, width, height))
	if width == 0 || height == 0 {
		return rgba, nil
	}
	// There are 3 bytes per pixel, and each row is 4-byte aligned.
	b := make([]byte, (3*width+3)&^3)
	y0, y1, yDelta := height-1, -1, -1

	topDown := false
	if topDown {
		y0, y1, yDelta = 0, height, +1
	}
	for y := y0; y != y1; y += yDelta {
		if _, err := io.ReadFull(r, b); err != nil {
			return nil, err
		}
		p := rgba.Pix[y*rgba.Stride : y*rgba.Stride+width*4]
		for i, j := 0, 0; i < len(p); i, j = i+4, j+3 {
			// BMP images are stored in BGR order rather than RGB order.
			p[i+0] = b[j+2]
			p[i+1] = b[j+1]
			p[i+2] = b[j+0]
			p[i+3] = 0xFF
		}
	}
	return rgba, nil
}

func init() {
	// This is needed to arrange that main() runs on main thread.
	// See documentation for functions that are only allowed to be called from the main thread.
	runtime.LockOSThread()
}

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

func createFontsTexture(io imgui.IO) uint32 {
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

func destroyFontsTexture(fontTexture uint32) {
	if fontTexture != 0 {
		gl.DeleteTextures(1, &fontTexture)
		imgui.CurrentIO().Fonts().SetTextureID(0)
		fontTexture = 0
	}
}

func initImguiStyle() {

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

func imguiHyperLink(name string, url string) {

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
			exec.Command("cmd.exe", "/C", "start", url).Start()
		}
		//AddUnderLine(imgui.PackedColor(0x0000ff))
		imgui.SetTooltip("Open in browser... ")
	} else {
		//AddUnderLine(imgui.PackedColor(0x0000ff))
	}

}

func imguiAboutView() {

	if imgui.BeginPopupModalV("About", nil, imgui.WindowFlagsNoResize|imgui.WindowFlagsNoSavedSettings) {
		imgui.Spacing()
		imgui.Spacing()
		imgui.Text("Teapot-Viewer 1.1a ")
		imgui.Spacing()
		imgui.Spacing()
		imgui.Separator()
		imgui.Spacing()
		imgui.Spacing()
		imgui.Text("Copyright (C) 2010-2020 by E.Heidt")
		imguiHyperLink("https://github.com/eh2k/teapot-viewer", "https://github.com/eh2k/teapot-viewer")
		imgui.Spacing()
		imgui.Spacing()
		imgui.Separator()
		imgui.Text("OpenGL " + gl.GoStr(gl.GetString(gl.VERSION)))
		imgui.Separator()
		if imgui.Button("OK") {
			imgui.CloseCurrentPopup()
		}
		imgui.EndPopup()
	}

}

func main() {

	log.SetFlags(log.LstdFlags | log.Lshortfile)

	imguic := imgui.CreateContext(nil)
	defer imguic.Destroy()
	io := imgui.CurrentIO()

	io.Fonts().TextureDataAlpha8()
	//imgui.StyleColorsDark()

	initImguiStyle()

	err := glfw.Init()
	if err != nil {
		panic(err)
	}
	defer glfw.Terminate()

	window, err := glfw.CreateWindow(800, 600, "TepotViewer", nil, nil)
	if err != nil {
		panic(err)
	}

	window.MakeContextCurrent()
	glfw.SwapInterval(1)

	if err := gl.Init(); err != nil {
		log.Fatalln("failed to initialize glfw:", err)
	}

	path, err := os.Executable()
	if err != nil {
		log.Println(err)
	}

	cs := C.CString(path + "/../teapot.obj.zip")
	defer C.free(unsafe.Pointer(cs))

	context := C.LoadModel(cs)
	if context == nil {
		log.Fatal("load failed")
	} else {

		log.Println("OK")

		window.SetScrollCallback(func(w *glfw.Window, xoff float64, yoff float64) {
			x, y := w.GetCursorPos()
			C.MouseWheel(context, 0, C.int(yoff), C.int(x), C.int(y))
		})

		window.SetMouseButtonCallback(func(w *glfw.Window, button glfw.MouseButton, action glfw.Action, mods glfw.ModifierKey) {

			if io.WantCaptureMouse() {
				io.SetMouseButtonDown(int(button), action == 1)
			} else {
				x, y := w.GetCursorPos()
				C.MouseButton(context, 1, C.int(x), C.int(y), C.int(action))
			}
		})

		window.SetCursorPosCallback(func(w *glfw.Window, x float64, y float64) {

			if w.GetAttrib(glfw.Focused) != 0 {
				io.SetMousePosition(imgui.Vec2{X: float32(x), Y: float32(y)})
			} else {
				io.SetMousePosition(imgui.Vec2{X: -math.MaxFloat32, Y: -math.MaxFloat32})
			}

			if io.WantCaptureMouse() == false {
				if w.GetMouseButton(0) == 1 {
					C.MouseMove(context, 1, C.int(x), C.int(y))
				}
				if w.GetMouseButton(1) == 1 {
					C.MouseMove(context, 2, C.int(x), C.int(y))
				}
			}

		})
	}

	log.Println("OK")

	fontTexture := createFontsTexture(io)
	defer destroyFontsTexture(fontTexture)

	showDemoWindow := false
	openFileDialog := false
	showAboutWindow := false

	for !window.ShouldClose() {

		glfw.PollEvents()

		width, height := window.GetSize()
		size := [2]float32{0.0, 0.0}
		size[0] = float32(width)
		size[1] = float32(height)
		displaySize := imgui.Vec2{float32(width), float32(height)}

		//gl.ClearColor(clearColor[0], clearColor[1], clearColor[2], 1)
		gl.Clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT)

		C.DrawScene(context, C.int(width), C.int(height), window.Handle())

		gl.Clear(gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT)
		io.SetDisplaySize(displaySize)

		imgui.NewFrame()

		if imgui.BeginMainMenuBar() {
			if imgui.BeginMenu("File") {
				if imgui.MenuItem("Open...") {
					openFileDialog = true
				}
				imgui.Separator()
				if imgui.MenuItemV("Exit", "Alt+F4", false, true) {
					os.Exit(0)
				}
				imgui.EndMenu()
			}
			if imgui.BeginMenu("View") {

				imguiMenuItem := func(text string, shortcut string, mode int) {
					on := C.ViewMode(context, C.int(mode), -1) == C.int(1)
					if imgui.MenuItemV(text, shortcut, on, true) {
						if on {
							C.ViewMode(context, C.int(mode), 0)
						} else {
							C.ViewMode(context, C.int(mode), 1)
						}
					}
				}

				imguiMenuItem("Wireframe", "W", 1)
				imguiMenuItem("Lighting", "L", 0x4)
				imguiMenuItem("Shadow", "S", 0x8)
				imguiMenuItem("Background", "G", 0x10)
				imgui.Separator()
				imguiMenuItem("BoundingBoxes", "B", 0x200)
				imguiMenuItem("Scene-ABB-Tree", "N", 0x100)
				//imgui.Separator()
				//imguiMenuItem("FPS", "F", 0x800)
				imgui.EndMenu()
			}
			if imgui.BeginMenu("Camera") {
				if imgui.MenuItemV("Perspective Projection", "P", false, true) {

				}
				if imgui.MenuItemV("Orthogonal Projection", "O", false, true) {

				}
				imgui.Separator()
				if imgui.MenuItemV("Default", "1", false, true) {

				}
				imgui.EndMenu()
			}
			if imgui.BeginMenu("Help") {
				if imgui.MenuItem("About...") {
					showAboutWindow = true
				}
				imgui.Separator()
				if imgui.MenuItem("Imgui Demo...") {
					showDemoWindow = true
				}
				imgui.EndMenu()
			}

			imgui.EndMainMenuBar()
		}

		if openFileDialog {
			openFileDialog = false
			filename, err := dialog.File().Filter("ZIP", "zip").Load()
			if err == nil {
				cs := C.CString(filename)
				defer C.free(unsafe.Pointer(cs))
				context = C.LoadModel(cs)
			}
		}

		if showAboutWindow {
			imgui.OpenPopup("About")
			showAboutWindow = false
		}

		imguiAboutView()

		if showDemoWindow {
			imgui.SetNextWindowPosV(imgui.Vec2{X: 650, Y: 20}, imgui.ConditionFirstUseEver, imgui.Vec2{})
			imgui.ShowDemoWindow(&showDemoWindow)
		}

		imgui.Render()

		Render(size, size, imgui.RenderedDrawData())
		//C.DrawScene(context, C.int(width), C.int(height), window.Handle())

		window.SwapBuffers()
	}

	fmt.Println("END")

	if false {
		u := C.GetBitmap(context)
		r := bytes.NewReader((*[800 * 600 * 3]byte)(unsafe.Pointer(u))[:])

		img, err := decodeRGB(r, 800, 600)

		if err != nil {
			log.Fatal(err)
		}

		var iimg image.Image = img

		http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
			writeImage(w, &iimg)
		})

		log.Fatal(http.ListenAndServe(":8081", nil))
	}
}
