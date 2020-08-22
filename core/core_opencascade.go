// +build oce

package core

// #cgo CFLAGS:
// #cgo windows CXXFLAGS: -IC:/git-sdk-64/mingw64/include/oce/ -Doce
// #cgo linux CXXFLAGS: -I/usr/include/oce/ -Doce
// #cgo windows LDFLAGS: -lTKSTEP.dll -lTKIGES.dll -lTKBO.dll -lTKXSBase.dll -lTKXCAF.dll -lTKXDESTEP.dll -lTKXDEIGES.dll -lTKTopAlgo.dll -lTKPrim.dll -lTKLCAF.dll -lTKernel.dll -lTKBRep.dll -lTKMath.dll -lTKMesh.dll -lTKG3d.dll -lTKGeomBase.dll -lTKG2D.dll
// #cgo linux LDFLAGS: -lTKSTEP -lTKIGES -lTKBO -lTKXSBase -lTKXCAF -lTKXDESTEP -lTKXDEIGES -lTKTopAlgo -lTKPrim -lTKLCAF -lTKernel -lTKBRep -lTKMath -lTKMesh -lTKG3d -lTKGeomBase -lTKG2d
// void* CreateOCLoader();
// #include "core.h"
import "C"

func init() {

	{
		p := C.CreateOCLoader()
		C.RegisterImporter2(p)
	}
}
