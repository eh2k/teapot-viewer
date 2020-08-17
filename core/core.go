//go build -v -ldflags=all='-H windowsgui -s -w' -o ../bin/TeapotViewer.exe

package core

// // #cgo CFLAGS: -static
// #cgo CXXFLAGS: -std=c++17
// #cgo windows LDFLAGS: -static -static-libgcc -static-libstdc++  -lopengl32
// #cgo linux LDFLAGS: -ldl -lstdc++ -lstdc++fs -lGL
// #include <stdlib.h>
// #include <string.h>
// typedef const char const_char;
// #include "core.h"
import "C"
import (
	"archive/zip"
	"bytes"
	"fmt"
	"io/ioutil"
	"log"
	"path/filepath"
	"reflect"
	"strings"
	"unsafe"
)

type (
	MODEL unsafe.Pointer
)

var loadModelCB func(p float32) = func(p float32) {
	fmt.Println("loadModelCB", p)
}

//export goTryReadFromZip
func goTryReadFromZip(path *C.const_char, data *unsafe.Pointer) C.ulonglong{

	archive := 	strings.SplitAfter(C.GoString(path), ".zip") 
	r, err := zip.OpenReader(archive[0]) // Open a zip archive for reading.
	if err != nil {
		log.Fatal(err)
	}
	defer r.Close()

	file := strings.ToUpper(archive[1])
	for _, f := range r.File {

		if strings.HasSuffix(file, strings.ToUpper(f.Name)) {
			fr, err := f.Open()
			if err != nil {
				log.Fatal(err)
			}
			defer fr.Close()

			fc, err := ioutil.ReadAll(fr)
			if err != nil {
				log.Fatal(err)
			}

			len := C.ulonglong(len(fc))
			*data = C.malloc(len)
			C.memcpy(*data, unsafe.Pointer(&fc[0]), len)
			return len
		}
	}

	return C.ulonglong(0)
}

//export goLoadModelProgressCB
func goLoadModelProgressCB(p float32) {
	loadModelCB(p)
}

//export goNewOpenGLTexture
func goNewOpenGLTexture(p unsafe.Pointer, size int) uint32{

	var data []byte

	sh := (*reflect.SliceHeader)(unsafe.Pointer(&data))
	sh.Data = uintptr(p)
	sh.Len = size
	sh.Cap = size

	r := bytes.NewReader(data)
	texId, err := NewTextureFromMemory(r, 0, 0)
	if err != nil {
		log.Println(err)
		return 0
	}

	return texId
}

func LoadModel(path string, progressCB func(p float32)) MODEL {

	if progressCB != nil {
		loadModelCB = progressCB
	}

	if strings.HasSuffix(path, ".zip"){
		r, err := zip.OpenReader(path) // Open a zip archive for reading.
		if err != nil {
			log.Fatal(err)
		}
		defer r.Close()
	
		for _, f := range r.File {
			cs := C.CString(filepath.Join(path, f.Name))
			defer C.free(unsafe.Pointer(cs))

			model := C.TryLoadModel(cs)
			if model != unsafe.Pointer(nil) {
				return MODEL(model)
			}

		}

		return nil

	} else {
		cs := C.CString(path)
		defer C.free(unsafe.Pointer(cs))
	
		model := C.TryLoadModel(cs)
		return MODEL(model)
	}
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

func GetSupportedFormats() string {
	r := C.GetSupportedFormats()
	return C.GoString(r)
}
