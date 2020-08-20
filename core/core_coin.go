// +build coin

package core

// #cgo CFLAGS:
// #cgo CXXFLAGS: -Dcoin
// #cgo windows LDFLAGS: -lCoin.dll
// #cgo linux LDFLAGS: -lCoin
// void* CreateCoinLoader();
// #include "core.h"
import "C"

func init() {
	
	{
		p := C.CreateCoinLoader()
		C.RegisterImporter2(p)
	}
}
