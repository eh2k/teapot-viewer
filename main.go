//go build -x -ldflags=all='-H windowsgui -s -w' -o ./bin/teapot-viewer.exe
//go build -x -ldflags=all='-H windowsgui -s -w' -tags oce -o ./bin/teapot-viewer.exe

package main

import (
	"./core" //github.com/eh2k/teapot-viewer/tree/experimental/core"
	"fmt"
	"github.com/eh2k/imgui-glfw-go-app"
	"github.com/eh2k/imgui-glfw-go-app/imgui-go"
	"github.com/eh2k/osdialog-go"
	"github.com/go-gl/gl/v2.1/gl"
	"github.com/go-gl/glfw/v3.3/glfw"
	"log"
	"math"
	"os"
	"path/filepath"
	"runtime"
)

func init() {
	// This is needed to arrange that main() runs on main thread.
	// See documentation for functions that are only allowed to be called from the main thread.
	runtime.LockOSThread()
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
var showProgress = false
var loadProgress float32 = 0

func loop(window *glfw.Window, displaySize imgui.Vec2) {

	width, height := displaySize.X, displaySize.Y

	//gl.ClearColor(clearColor[0], clearColor[1], clearColor[2], 1)
	gl.Clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT)

	core.DrawScene(context, int(width), int(height), window.Handle())

	gl.Clear(gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT)

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
			imgui.EndMenu()
		}

		imgui.EndMainMenuBar()
	}

	app.ShowAboutPopup(&showAboutWindow, "Teapot-Viewer", "1.0.10", "Copyright (C) 2010-2020 by E.Heidt", "https://github.com/eh2k/teapot-viewer")

	if showDemoWindow {
		imgui.SetNextWindowPosV(imgui.Vec2{X: 650, Y: 20}, imgui.ConditionFirstUseEver, imgui.Vec2{})
		imgui.ShowDemoWindow(&showDemoWindow)
	}

	if showProgress {
		showProgress = false
		imgui.OpenPopup("Load")
	}

	if imgui.BeginPopupModalV("Load", nil, imgui.WindowFlagsNoResize|imgui.WindowFlagsNoSavedSettings|imgui.WindowFlagsNoTitleBar) {
		imgui.Text("loading...")
		imgui.ProgressBarV(loadProgress, imgui.Vec2{X: 300, Y: 22}, "")
		if loadProgress >= 1 {
			imgui.CloseCurrentPopup()
		}

		imgui.EndPopup()
	}

	if openFileDialog {
		openFileDialog = false

		filters := core.GetSupportedFormats()
		filename, err := osdialog.ShowOpenFileDialog(".", "", filters)
		if err == nil {
			LoadModel(filename)
		}
	}
}

func LoadModel(path string) {

	showProgress = true
	loadProgress = 0.1
	go func() {

		tmp := core.LoadModel(path, func(p float32) {
			loadProgress = p
		})

		if tmp != nil {
			context = tmp
			core.ViewMode(context, 0x8, 0)
		} else {
			osdialog.ShowMessageBox(osdialog.Error, osdialog.Ok, "Loading "+path+" failed.")
			context = core.LoadTeapot()
		}

		loadProgress = 1
	}()
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

	window := app.NewAppWindow(640, 400)
	window.SetTitle("Teapot-Viewer")
	defer app.Dispose()

	io := imgui.CurrentIO()

	context = core.LoadTeapot()

	app.InitMyImguiStyle()

	if len(os.Args) > 1 {
		LoadModel(os.Args[1]) //LoadModel("F40.dae.zip")
	}

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

			if !io.WantCaptureMouse() {
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

	app.Run(func(displaySize imgui.Vec2) {
		loop(window, displaySize)
	})

	fmt.Println("END")
}
