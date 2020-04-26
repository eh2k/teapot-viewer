//go build -x -ldflags=all='-H windowsgui -s -w' -o ./bin/teapot-viewer.exe

package main

import (
	"fmt"
	//"io"
	"./core" //github.com/eh2k/teapot-viewer/tree/experimental/core"
	"github.com/go-gl/gl/v2.1/gl"
	"github.com/go-gl/glfw/v3.3/glfw"
	"github.com/inkyblackness/imgui-go"
	"log"
	"math"
	"os"
	"path/filepath"
	"runtime"
	"time"
)

func init() {
	// This is needed to arrange that main() runs on main thread.
	// See documentation for functions that are only allowed to be called from the main thread.
	runtime.LockOSThread()
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
		HyperLink("https://github.com/eh2k/teapot-viewer", "https://github.com/eh2k/teapot-viewer", OpenUrl)
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

var context core.MODEL

func imguiViewModeMenuItem(text string, shortcut string, mode int) {

	if mode > 0x8000 {
		on := core.ViewMode(context, mode, -1) == 1
		if imgui.MenuItemV(text, shortcut, !on, true) {
			if on {
				core.ViewMode(context, mode, 0)
			} else {
				core.ViewMode(context, mode, 1)
			}
		}
	} else {
		on := core.ViewMode(context, mode, -1) == 0
		if imgui.MenuItemV(text, shortcut, !on, true) {
			if on {
				core.ViewMode(context, mode, 1)
			} else {
				core.ViewMode(context, mode, 0)
			}
		}
	}

}

var showDemoWindow = false
var openFileDialog = false
var showAboutWindow = false
var loadProgress float32 = 0

var laodProgressCb = func(p float32) {
	fmt.Println("laodProgressCb", p)
}

func loop(window *glfw.Window) {
	glfw.PollEvents()

	io := imgui.CurrentIO()
	width, height := window.GetSize()
	size := [2]float32{0.0, 0.0}
	size[0] = float32(width)
	size[1] = float32(height)
	displaySize := imgui.Vec2{float32(width), float32(height)}

	//gl.ClearColor(clearColor[0], clearColor[1], clearColor[2], 1)
	gl.Clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT)

	core.DrawScene(context, width, height, window.Handle())

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

			imguiViewModeMenuItem("Wireframe", "W", 1)
			imguiViewModeMenuItem("Lighting", "L", 0x4)
			imguiViewModeMenuItem("Shadow", "S", 0x8)
			imguiViewModeMenuItem("Background", "G", 0x10)
			imgui.Separator()
			imguiViewModeMenuItem("BoundingBoxes", "B", 0x200)
			imguiViewModeMenuItem("Scene-ABB-Tree", "N", 0x100)
			//imgui.Separator()
			//imguiMenuItem("FPS", "F", 0x800)
			imgui.EndMenu()
		}
		if imgui.BeginMenu("Camera") {

			imguiViewModeMenuItem("Perspective Projection", "P", 0x8002)
			imguiViewModeMenuItem("Orthogonal Projection", "O", 0x2)

			imgui.Separator()
			if imgui.MenuItemV("Default", "1", false, true) {
				core.SetCamera(context, 0)
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

	if showAboutWindow {
		imgui.OpenPopup("About")
		showAboutWindow = false
	}

	imguiAboutView()

	if showDemoWindow {
		imgui.SetNextWindowPosV(imgui.Vec2{X: 650, Y: 20}, imgui.ConditionFirstUseEver, imgui.Vec2{})
		imgui.ShowDemoWindow(&showDemoWindow)
	}

	if loadProgress > 0 {
		imgui.SetNextWindowPos(imgui.Vec2{X: size[0]/2 - 150.0, Y: size[1]/2 - 20.0})
		imgui.BeginV("loading...", nil, imgui.WindowFlagsNoResize|imgui.WindowFlagsNoSavedSettings|imgui.WindowFlagsNoTitleBar)
		imgui.Text("loading...")
		imgui.ProgressBarV(loadProgress, imgui.Vec2{300, 22}, "")
		imgui.End()
	}

	imgui.Render()

	Render(size, size, imgui.RenderedDrawData())
	//C.DrawScene(context, C.int(width), C.int(height), window.Handle())

	window.SwapBuffers()
}

func main() {

	log.SetFlags(log.LstdFlags | log.Lshortfile)

	exePath, err := os.Executable()
	if err != nil {
		log.Println(err)
	}

	err3 := os.Chdir(filepath.Dir(exePath))
	if err3 != nil {
		log.Println(err3)
	}

	imguic := imgui.CreateContext(nil)
	defer imguic.Destroy()
	io := imgui.CurrentIO()

	io.Fonts().TextureDataAlpha8()
	InitImguiStyle()

	err = glfw.Init()
	if err != nil {
		panic(err)
	}
	defer glfw.Terminate()

	window, err := glfw.CreateWindow(800, 600, "TepotViewer", nil, nil)
	if err != nil {
		panic(err)
	}

	SetWindowIcon(window)

	window.MakeContextCurrent()
	glfw.SwapInterval(1)

	if err := gl.Init(); err != nil {
		log.Fatalln("failed to initialize glfw:", err)
	}

	t := time.Now()
	laodProgressCb = func(p float32) {

		if time.Now().Sub(t).Nanoseconds() > (50 * 1e6) {
			loop(window)
			t = time.Now()
			loadProgress = p
		}

		if p >= 1 {
			loadProgress = 0
		}
	}

	//context = core.LoadModel("teapot.obj.zip", laodProgressCb)
	context = core.LoadModel("F40.dae.zip", laodProgressCb)
	
	if context == nil {
		log.Fatal("load failed")
	} else {

		log.Println("OK")

		window.SetScrollCallback(func(w *glfw.Window, xoff float64, yoff float64) {
			x, y := w.GetCursorPos()
			core.MouseWheel(context, int(x), int(y), 0, int(yoff))
		})

		window.SetMouseButtonCallback(func(w *glfw.Window, button glfw.MouseButton, action glfw.Action, mods glfw.ModifierKey) {

			if io.WantCaptureMouse() {
				io.SetMouseButtonDown(int(button), action == 1)
			} else {
				x, y := w.GetCursorPos()
				core.MouseButton(context, 1, int(x), int(y), int(action))
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
					core.MouseMove(context, 1, int(x), int(y))
				}
				if w.GetMouseButton(1) == 1 {
					core.MouseMove(context, 2, int(x), int(y))
				}
			}

		})

		io.KeyMap(imgui.KeyV, int(glfw.KeyV))
		io.KeyMap(imgui.KeyX, int(glfw.KeyX))
		io.KeyMap(imgui.KeyY, int(glfw.KeyY))
		io.KeyMap(imgui.KeyZ, int(glfw.KeyZ))

		window.SetCharCallback(func(window *glfw.Window, char rune) {
			io.AddInputCharacters(string(char))
		})

		window.SetKeyCallback(func(window *glfw.Window, key glfw.Key, scancode int, action glfw.Action, mods glfw.ModifierKey) {
			if action == glfw.Press {
				io.KeyPress(int(key))
			}
			if action == glfw.Release {
				io.KeyRelease(int(key))
			}

			// Modifiers are not reliable across systems
			io.KeyCtrl(int(glfw.KeyLeftControl), int(glfw.KeyRightControl))
			io.KeyShift(int(glfw.KeyLeftShift), int(glfw.KeyRightShift))
			io.KeyAlt(int(glfw.KeyLeftAlt), int(glfw.KeyRightAlt))
			io.KeySuper(int(glfw.KeyLeftSuper), int(glfw.KeyRightSuper))

			if action == glfw.Press {
				io.KeyPress(int(key))

				switch int(key) {
				case 87:
					core.ViewMode(context, 0x1, -2)
				case 76:
					core.ViewMode(context, 0x4, -2)
				case 83:
					core.ViewMode(context, 0x8, -2)
				case 71:
					core.ViewMode(context, 0x10, -2)
				case 66:
					core.ViewMode(context, 0x200, -2)
				case 78:
					core.ViewMode(context, 0x100, -2)
				case 80:
					core.ViewMode(context, 0x2, -2) //Perspective
				case 79:
					core.ViewMode(context, 0x2, -2) //Ortho
				case 49:
					core.SetCamera(context, 0) //Camera default
				default:
					fmt.Println(key)
				}
			}
		})
	}

	fontTexture := CreateFontsTexture(io)
	defer DestroyFontsTexture(fontTexture)

	for !window.ShouldClose() {
		loop(window)

		if openFileDialog {
			openFileDialog = false
			filename, err := OpenFileDialog(core.GetSupportedFormats())
			if err == nil {
				context = core.LoadModel(filename, laodProgressCb)
			}
		}
	}

	fmt.Println("END")

}
