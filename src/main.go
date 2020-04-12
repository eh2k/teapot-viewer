package main

// #cgo LDFLAGS: -L${SRCDIR}/../bin/ -ltv
// #include <stdlib.h>
// #include "main.h"
import "C"
import (
	"bytes"
	"fmt"
	"image"
	"image/jpeg"
	"io"
	"log"
	"net/http"
	"os"
	"runtime"
	"strconv"
	"unsafe"
	"github.com/go-gl/glfw/v3.3/glfw"
	"time"
	"sync"
)

func init() {
	// This is needed to arrange that main() runs on main thread.
	// See documentation for functions that are only allowed to be called from the main thread.
	runtime.LockOSThread()
}

func writeImage(w http.ResponseWriter, img *image.Image) bool {

	buffer := new(bytes.Buffer)
	if err := jpeg.Encode(buffer, *img, nil); err != nil {
		log.Println("unable to encode image.")
		return false
	}

	w.Header().Set("Content-Type", "image/jpeg")
	w.Header().Set("Content-Length", strconv.Itoa(len(buffer.Bytes())))
	if _, err := w.Write(buffer.Bytes()); err != nil {
		log.Println("unable to write image.")
		return false
	}

	return true
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

	if false {
		err := glfw.Init()
		if err != nil {
			panic(err)
		}
		defer glfw.Terminate()

		window, err := glfw.CreateWindow(800, 600, "TepotViewer", nil, nil)
		if err != nil {
			panic(err)
		}

		window.SetScrollCallback(func(w *glfw.Window, xoff float64, yoff float64) {
			x, y := w.GetCursorPos()
			C.MouseWheel(0, C.int(yoff), C.int(x), C.int(y))
		})

		window.SetMouseButtonCallback(func(w *glfw.Window, button glfw.MouseButton, action glfw.Action, mods glfw.ModifierKey) {
			x, y := w.GetCursorPos()
			if action == 1 {
				C.MouseDown(1, C.int(x), C.int(y))
			} else {
				C.MouseUp(1, C.int(x), C.int(y))
			}
		})

		window.SetCursorPosCallback(func(w *glfw.Window, x float64, y float64) {

			if w.GetMouseButton(0) == 1 {
				C.MouseMove(1, C.int(x), C.int(y))
			}
			if w.GetMouseButton(1) == 1 {
				C.MouseMove(2, C.int(x), C.int(y))
			}
		})

		window.MakeContextCurrent()

		path, err := os.Executable()
		if err != nil {
			log.Println(err)
		}

		cs := C.CString(path + "/../teapot.obj.zip")
		defer C.free(unsafe.Pointer(cs))
		if C.LoadModel(cs, window.Handle()) == 0 {
			log.Fatal("load failed")
		}

		for !window.ShouldClose() {

			width, height := window.GetSize()
			C.DrawScene(C.int(width), C.int(height))

			window.SwapBuffers()
			glfw.PollEvents()
		}

	} else {

		path, err := os.Executable()
		if err != nil {
			log.Println(err)
		}

		cs := C.CString(path + "/../teapot.obj.zip")
		defer C.free(unsafe.Pointer(cs))
		
		

		x := 10
		y := 10

		loaded := false

		var mutex = &sync.Mutex{}

		http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
			
			runtime.LockOSThread()
			w.Header().Set("Content-Type", "multipart/x-mixed-replace; boundary=frame")

			if loaded == false{
				if C.LoadModel(cs, nil) == 0 {
					log.Fatal("load failed")
				}
				loaded = true
			} else {
				return
			}

			for {

				mutex.Lock()

				C.MouseMove(1, C.int(x), C.int(y))

				u := C.GetBitmap()

				reader := bytes.NewReader((*[800 * 600 * 3]byte)(unsafe.Pointer(u))[:])

				img, err := decodeRGB(reader, 800, 600)
		
				if err != nil {
					log.Fatal(err)
				}
		
				var iimg image.Image = img

				w.Write([]byte("--frame\r\n  Content-Type: image/jpeg\r\n\r\n"))

				if writeImage(w, &iimg) == false{
					break
				}

				w.Write([]byte("\r\n\r\n"))

				x += 100
				C.GetBitmap()

				time.Sleep(200 * time.Millisecond)

				mutex.Unlock()
			}
		})

		
		fmt.Println("http://localhost:8081")
		log.Fatal(http.ListenAndServe(":8081", nil))
	}

	fmt.Println("END")
}
