package core

// #cgo linux pkg-config: assimp
// #cgo CFLAGS: 
// #cgo CXXFLAGS: -fpermissive -I../lib3ds_loader/lib3ds-20080909/src/
// #cgo CXXFLAGS: -std=c++17 -fpermissive
// #cgo windows LDFLAGS: -static -static-libgcc -static-libstdc++ -lstdc++fs -lassimp -lz -lIrrXML 
// #cgo linux LDFLAGS: -ldl -lstdc++ -lstdc++fs -lassimp 
// void* CreateAssimpLoader();
// void* Create3DSLoader();
// #include "core.h"
import "C"

func init() {
	
	{
		p := C.Create3DSLoader()
		C.RegisterImporter(p)
	}
	
	{
		p := C.CreateAssimpLoader()
		C.RegisterImporter(p)
	}

}