// +build oce

package core

// #cgo CFLAGS:
// #cgo CXXFLAGS: -IC:/git-sdk-64/mingw64/include/oce/ -Doce
// #cgo windows LDFLAGS: -lTKSTEP.dll -lTKIGES.dll -lTKBO.dll -lTKXSBase.dll -lTKXCAF.dll -lTKXDESTEP.dll -lTKXDEIGES.dll -lTKTopAlgo.dll -lTKPrim.dll -lTKLCAF.dll -lTKernel.dll -lTKBREP.dll -lTKMath.dll -lTKMesh.dll -lTKG3d.dll -lTKGeomBase.dll -lTKG2D.dll
// #cgo linux LDFLAGS: -ldl -lstdc++ -lstdc++fs
// void* CreateOCLoader();
// #include "core.h"
import "C"

func init() {
	
	{
		p := C.CreateOCLoader()
		C.RegisterImporter2(p)
	}
}
