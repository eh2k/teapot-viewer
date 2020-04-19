package main

import (
	"github.com/go-gl/glfw/v3.3/glfw"
	"os"
	"path/filepath"
	"log"
	"os/exec"
)

func SetWindowIcon(window *glfw.Window) {

}

func OpenFileDialog(a, b string) (string, error){

	exePath, err := os.Executable()
	if err != nil {
		log.Println(err)
	}

    return filepath.Join(filepath.Dir(exePath), "happy1.obj.zip"), nil
}

func OpenUrl(url string){
	exec.Command("xdg-open", url).Start()
}