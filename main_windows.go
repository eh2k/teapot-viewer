package main

/*
#include <windows.h>
void SetWindowIcon(HWND hwnd)
{
    HANDLE hbicon = LoadImage(
        GetModuleHandle(0),
        MAKEINTRESOURCE(100),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXICON),
        GetSystemMetrics(SM_CYICON),
        0);
	if (hbicon)
	{
		LRESULT r = SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hbicon);
	}

    HANDLE hsicon = LoadImage(
        GetModuleHandle(0),
        MAKEINTRESOURCE(100),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        0);
	if (hsicon)
	{
		LRESULT r = SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hsicon);
	}       
}
*/
import "C"

import (
	"unsafe"
    "github.com/go-gl/glfw/v3.3/glfw"
    "github.com/sqweek/dialog"
    "os/exec"
)

func SetWindowIcon(window *glfw.Window) {

	hwnd := unsafe.Pointer(window.GetWin32Window())
	C.SetWindowIcon(C.HWND(hwnd))
}

func OpenFileDialog(a, b string) string, error{
    return dialog.File().Filter(a, b).Load()
}

func OpenUrl(url string){
	exec.Command("explorer.exe", "start", url).Start()
}