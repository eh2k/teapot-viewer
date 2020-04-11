package main

import (
	//"encoding/json"
	"fmt"
	//"net/http"
	//"os"
	//"io"
	//"bytes"
	//"strings"
	//"regexp"
	//"io/ioutil"
	//"path"
	"syscall"
	"unsafe"
	"image"
	//"testing"
	"net/http"
	"log"
	"image/jpeg"
	//"golang.org/x/image/bmp"
	"strconv"
	"bytes"
	//"os"
	"io"
)

func writeImage(w http.ResponseWriter, img *image.Image) {

	buffer := new(bytes.Buffer)
	if err := jpeg.Encode(buffer, *img, nil); err != nil {
		log.Println("unable to encode image.")
	}

	w.Header().Set("Content-Type", "image/jpeg")
	w.Header().Set("Content-Length", strconv.Itoa(len(buffer.Bytes())))
	if _, err := w.Write(buffer.Bytes()); err != nil {
		log.Println("unable to write image.")
	}
}

func decodeRGB(r io.Reader, width int, height int) (image.Image, error) {
	rgba := image.NewRGBA(image.Rect(0, 0, width, height))
	if width == 0 || height == 0 {
		return rgba, nil
	}
	// There are 3 bytes per pixel, and each row is 4-byte aligned.
	b := make([]byte, (3*width+3)&^3)
	y0, y1, yDelta := height-1, -1, -1

	topDown := false
	if topDown {
		y0, y1, yDelta = 0, height, +1
	}
	for y := y0; y != y1; y += yDelta {
		if _, err := io.ReadFull(r, b); err != nil {
			return nil, err
		}
		p := rgba.Pix[y*rgba.Stride : y*rgba.Stride+width*4]
		for i, j := 0, 0; i < len(p); i, j = i+4, j+3 {
			// BMP images are stored in BGR order rather than RGB order.
			p[i+0] = b[j+2]
			p[i+1] = b[j+1]
			p[i+2] = b[j+0]
			p[i+3] = 0xFF
		}
	}
	return rgba, nil
}

func main() {

	fmt.Println("start");

	DLL, err := syscall.LoadLibrary("tv.dll")
	if err != nil{
		panic(err)
	}
	defer syscall.FreeLibrary(DLL)
	
	LoadModel := func (file string) {
		f, _ := syscall.GetProcAddress(DLL, "LoadModel")
		b := append([]byte(file), 0)
		syscall.Syscall(uintptr(f), 1, uintptr(unsafe.Pointer(&b[0])), 0, 0)
	}

	// SaveBitmap := func (file string) {
	// 	f, _ := syscall.GetProcAddress(DLL, "SaveBitmap")
	// 	b := append([]byte(file), 0)
	// 	syscall.Syscall(uintptr(f), 1, uintptr(unsafe.Pointer(&b[0])), 0, 0)
	// }

	GetBitmap := func () io.Reader {
		f, _ := syscall.GetProcAddress(DLL, "GetBitmap")
		u, _, _ := syscall.Syscall(uintptr(f), 0, 0, 0, 0)
		const sizeOfUintPtr = unsafe.Sizeof(uintptr(0))
		fmt.Println(u)
		return bytes.NewReader((*[800*600*3]byte)(unsafe.Pointer(u))[:])

	}

	LoadModel("asdf")
	// SaveBitmap("test.bmp")

	r := GetBitmap()

	img, err := decodeRGB(r, 800, 600) 

	if err != nil {
		log.Fatal(err)
	}

	var iimg image.Image = img 

    http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		writeImage(w, &iimg)
    })

    log.Fatal(http.ListenAndServe(":8081", nil))

	
}

