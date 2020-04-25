//go build -v -ldflags=all='-H windowsgui -s -w' -o ../bin/TeapotViewer.exe

package core

// // #cgo CFLAGS: -static
// #cgo CXXFLAGS: -std=c++17
// #cgo windows LDFLAGS: -static -static-libgcc -static-libstdc++ -lminizip -lz  -lopengl32
// #cgo linux LDFLAGS: -ldl -lstdc++ -lminizip -lz -lstdc++fs -lGL
// #include <stdlib.h>
// #include "core.h"
import "C"
import (
	"unsafe"
	"syscall"
	"fmt"
)

type (
	MODEL unsafe.Pointer
)

var loadModelCB func(p float32) = func(p float32){
	fmt.Println("loadModelCB", p)
}

//export goLoadModelProgressCB
func goLoadModelProgressCB(p float32) {
	loadModelCB(p)
}

func LoadModel(path string, progressCB func(p float32)) MODEL {
	cs := C.CString(path)
	defer C.free(unsafe.Pointer(cs))

	if progressCB != nil{
		loadModelCB = progressCB
	}

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

func GetSupportedFormats() string{
	r := C.GetSupportedFormats()
	ptrwchar := uintptr(unsafe.Pointer(r))
	gostr := syscall.UTF16ToString((*[1 << 20]uint16)(unsafe.Pointer(ptrwchar))[:])
	return gostr
}