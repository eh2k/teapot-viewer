// +build http

package main

import (
	"bytes"
	"fmt"
	"image"
	"image/jpeg"
	"io"
	"log"
	"net/http"
	"strconv"

)

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

func serve(){
	
	fmt.Println("todo")
	// if false {
	// 	u := C.GetBitmap(context)
	// 	r := bytes.NewReader((*[800 * 600 * 3]byte)(unsafe.Pointer(u))[:])

	// 	img, err := core.decodeRGB(r, 800, 600)

	// 	if err != nil {
	// 		log.Fatal(err)
	// 	}

	// 	var iimg image.Image = img

	// 	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
	// 		writeImage(w, &iimg)
	// 	})

	// 	log.Fatal(http.ListenAndServe(":8081", nil))
	// }
}