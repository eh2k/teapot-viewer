package main

// #cgo linux pkg-config: assimp
// #cgo CFLAGS:
// #cgo CXXFLAGS: -std=c++17 
// #cgo CXXFLAGS: -Icore/
// #cgo windows LDFLAGS: -static -static-libgcc -static-libstdc++ -lstdc++fs -lassimp
// #cgo linux LDFLAGS: -ldl -lstdc++ -lstdc++fs -lassimp
// void* CreateAssimpLoader();
import "C"
import (
	"fmt"
	"./core"
)

func init() {
	p := C.CreateAssimpLoader()
	fmt.Println("AssimpLoader:", p)
	core.RegisterImporter(p)
}
