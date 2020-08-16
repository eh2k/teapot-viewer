package core

import (
	//"errors"
	"github.com/go-gl/gl/v2.1/gl"
	"github.com/disintegration/imaging"
	"image"
	//"image/draw"
	_ "image/jpeg"
	_ "image/png"
	"io"
	"os"
)

//var errUnsupportedStride = errors.New("unsupported stride, only 32-bit colors supported")
//var errTextureNotBound = errors.New("texture not bound")

func NewTextureFromFile(file string, wrapR, wrapS int32) (uint32, error) {
	imgFile, err := os.Open(file)
	if err != nil {
		return 0, err
	}
	defer imgFile.Close()

	// Decode detexts the type of image as long as its image/<type> is imported
	img, _, err := image.Decode(imgFile)
	if err != nil {
		return 0, err
	}
	return NewTexture(img, wrapR, wrapS)
}

func NewTextureFromMemory(reader io.Reader, wrapR, wrapS int32) (uint32, error) {

	// Decode detexts the type of image as long as its image/<type> is imported
	img, _, err := image.Decode(reader)
	if err != nil {
		return 0, err
	}
	return NewTexture(img, wrapR, wrapS)
}

func NewTexture(img image.Image, wrapR, wrapS int32) (uint32, error) {
	// rgba := image.NewRGBA(img.Bounds())

	// draw.Draw(rgba, rgba.Bounds(), img, image.Pt(0, 0), draw.Src)
	// if rgba.Stride != rgba.Rect.Size().X*4 { // TODO-cs: why?
	// 	return 0, errUnsupportedStride
	// }

	rgba := imaging.FlipV(img)

	var handle uint32
	gl.GenTextures(1, &handle)

	target := uint32(gl.TEXTURE_2D)
	//internalFmt := int32(gl.SRGB_ALPHA)
	format := uint32(gl.RGBA)
	width := int32(rgba.Rect.Size().X)
	height := int32(rgba.Rect.Size().Y)
	pixType := uint32(gl.UNSIGNED_BYTE)
	dataPtr := gl.Ptr(rgba.Pix)

	gl.ActiveTexture(gl.TEXTURE0)
	gl.BindTexture(target, handle)
	defer gl.BindTexture(target, 0)

	gl.TexParameteri(target, gl.TEXTURE_WRAP_S, gl.REPEAT)
	gl.TexParameteri(target, gl.TEXTURE_WRAP_T, gl.REPEAT)

	gl.TexEnvi(gl.TEXTURE_ENV, gl.TEXTURE_ENV_MODE, gl.MODULATE)
	gl.TexParameteri(target, gl.TEXTURE_MAG_FILTER, gl.LINEAR)
	gl.TexParameteri(target, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
	gl.PixelStorei(gl.UNPACK_ALIGNMENT, 1)
	gl.PixelStorei(gl.UNPACK_SKIP_ROWS, 0)
	gl.PixelStorei(gl.UNPACK_SKIP_PIXELS, 0)
	gl.PixelStorei(gl.UNPACK_ROW_LENGTH, 0)
	gl.PixelStorei(gl.UNPACK_SWAP_BYTES, 0)

	gl.TexImage2D(target, 0, gl.RGBA, width, height, 0, format, pixType, dataPtr)

	//gl.GenerateMipmap(handle)

	return handle, nil
}
