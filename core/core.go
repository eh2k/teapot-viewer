//go build -v -ldflags=all='-H windowsgui -s -w' -o ../bin/TeapotViewer.exe

package core

// // #cgo CFLAGS: -static
// #cgo CXXFLAGS: -std=c++17
// #cgo LDFLAGS: -static -static-libgcc -static-libstdc++ -lopengl32 -lminizip -lz
// #include <stdlib.h>
// #include "core.h"
import "C"
import (
	"unsafe"
)

type (
	MODEL unsafe.Pointer
)

func LoadModel(path string) MODEL {
	cs := C.CString(path)
	defer C.free(unsafe.Pointer(cs))
	model := C.LoadModel(cs)
	return MODEL(model)
}

func MouseWheel(context MODEL, x, y, xoff, yoff int) {
	C.MouseWheel(unsafe.Pointer(context), C.int(0), C.int(yoff), C.int(x), C.int(y))
}

func MouseButton(context MODEL, button, x, y, action int) {
	C.MouseButton(unsafe.Pointer(context), C.int(button), C.int(x), C.int(y), C.int(action))
}

func MouseMove(context MODEL, button, x, y int) {
	C.MouseMove(unsafe.Pointer(context), C.int(button), C.int(x), C.int(y))
}

func ViewMode(context MODEL, mode, enabled int) int {
	return int(C.ViewMode(unsafe.Pointer(context), C.int(mode), C.int(enabled)))
}

func SetCamera(context MODEL, num int) {
	C.SetCamera(unsafe.Pointer(context), C.int(num))
}

func DrawScene(context MODEL, width, height int, windowHandle unsafe.Pointer) {
	C.DrawScene(unsafe.Pointer(context), C.int(width), C.int(height), windowHandle)
}
