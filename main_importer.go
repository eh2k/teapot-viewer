package main

// #cgo linux pkg-config: assimp
// #cgo CFLAGS: 
// #cgo CXXFLAGS: -std=c++17  -fpermissive 
// #cgo CXXFLAGS: -Icore/  -I./lib3ds_loader/lib3ds-20080909/src/ 
// #cgo windows LDFLAGS: -static -static-libgcc -static-libstdc++ -lstdc++fs -lassimp -lz -lIrrXML
// #cgo linux LDFLAGS: -ldl -lstdc++ -lstdc++fs -lassimp 
// void* CreateAssimpLoader();
// void* CreateLib3DSLoader();
import "C"
import (
	"fmt"
	"./core"
)

func init() {
	p := C.CreateLib3DSLoader()
	fmt.Println("Lib3DSLoader:", p)
	core.RegisterImporter(p)
	p2 := C.CreateAssimpLoader()
	fmt.Println("AssimpLoader:", p2)
	core.RegisterImporter(p2)
}
