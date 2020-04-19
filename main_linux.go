package main

import (
	"github.com/go-gl/glfw/v3.3/glfw"
	"os"
	"path/filepath"
	"log"
	"os/exec"
	"bytes"
	"strings"
)

func SetWindowIcon(window *glfw.Window) {

}

func OpenFileDialog(a, b string) (string, error){

	 exePath, err := os.Executable()
	 if err != nil {
	 	log.Println(err)
	}

	buf := new(bytes.Buffer)
	cmd := exec.Command("zenity", "--file-selection", filepath.Dir(exePath))
	cmd.Stdout = buf
	err = cmd.Run()
	f:= strings.Trim(buf.String(), "\n")
	return f, err //return filepath.Join(filepath.Dir(exePath), "happy1.obj.zip"), nil
}

func OpenUrl(url string){
	exec.Command("xdg-open", url).Start()
}