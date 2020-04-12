#include <stdio.h>
#include <assert.h>
#include <filesystem>

#include "IDriver.h"
#include "Viewport.h"
#include "SceneIO.h"
#include "Controller.h"

#include <windows.h>
#include <gl/gl.h>

using namespace eh;

extern "C" IDriver *CreateOpenGL1Driver(int *pWindow);
extern "C" SceneIO::IPlugIn *XcreateOBJPlugIn(); //OBJ

void *_pBits;
HBITMAP _hBitmap;
HDC _hDC;
HGLRC _hRC;
int width = 800;
int height = 600;

#define VR(r) assert(r)

IDriver *InitGL()
{
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight * 3;

    void *pBits = NULL; //Todo: Prüfen ob Release notwendig
    VR(_hBitmap = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &pBits, NULL, (DWORD)0));
    _pBits = pBits;

    // Create the memory DC for the OpenGL context
    VR(_hDC = ::CreateCompatibleDC(NULL));

    ::SelectObject(_hDC, _hBitmap);

    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR); // size of this pfd
    pfd.nVersion = 1;                          // version numbe
    pfd.dwFlags = PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL | PFD_SUPPORT_GDI;
    pfd.iPixelType = PFD_TYPE_RGBA;  // RGBA type
    pfd.cColorBits = 24;             // 24-bit color depth
    pfd.cAlphaBits = 8;              // 8-Bit alpha buffer
    pfd.cDepthBits = 24;             // z-buffer
    pfd.cStencilBits = 8;            // stencil buffer
    pfd.iLayerType = PFD_MAIN_PLANE; // main layer

    int iPixelFormat = 0;

    VR(iPixelFormat = ChoosePixelFormat(_hDC, &pfd));

    VR(SetPixelFormat(_hDC, iPixelFormat, &pfd));

    VR(_hRC = wglCreateContext(_hDC));
    VR(wglMakeCurrent(_hDC, _hRC));

    return CreateOpenGL1Driver(nullptr);
}

extern "C" __declspec(dllexport) bool SaveBitmap(char *szPathName)
{
    // Create a new file for writing
    FILE *pFile = fopen(szPathName, "wb"); // wb -> w: writable b: binary, open as writable and binary
    if (pFile == NULL)
    {
        return false;
    }

    BITMAPINFOHEADER BMIH; // BMP header
    BMIH.biSize = sizeof(BITMAPINFOHEADER);
    BMIH.biSizeImage = width * height * 3;
    // Create the bitmap for this OpenGL context
    BMIH.biSize = sizeof(BITMAPINFOHEADER);
    BMIH.biWidth = width;
    BMIH.biHeight = height;
    BMIH.biPlanes = 1;
    BMIH.biBitCount = 24;
    BMIH.biCompression = BI_RGB;
    BMIH.biSizeImage = width * height * 3;

    BITMAPFILEHEADER bmfh; // Other BMP header
    int nBitsOffset = sizeof(BITMAPFILEHEADER) + BMIH.biSize;
    LONG lImageSize = BMIH.biSizeImage;
    LONG lFileSize = nBitsOffset + lImageSize;
    bmfh.bfType = 'B' + ('M' << 8);
    bmfh.bfOffBits = nBitsOffset;
    bmfh.bfSize = lFileSize;
    bmfh.bfReserved1 = bmfh.bfReserved2 = 0;

    // Write the bitmap file header               // Saving the first header to file
    UINT nWrittenFileHeaderSize = fwrite(&bmfh, 1, sizeof(BITMAPFILEHEADER), pFile);

    // And then the bitmap info header            // Saving the second header to file
    UINT nWrittenInfoHeaderSize = fwrite(&BMIH, 1, sizeof(BITMAPINFOHEADER), pFile);

    // Finally, write the image data itself
    //-- the data represents our drawing          // Saving the file content in lpBits to file
    UINT nWrittenDIBDataSize = fwrite(_pBits, 1, lImageSize, pFile);
    fclose(pFile); // closing the file.

    return true;
}

Viewport *_vp = nullptr;

extern "C" __declspec(dllexport) void *GetBitmap()
{
    _vp->drawScene();
    return _pBits;
}


extern "C" __declspec(dllexport) int LoadModel(const char *path, void *window)
{
    std::shared_ptr<SceneIO::IPlugIn> pPlugIn(XcreateOBJPlugIn());
    SceneIO::getInstance().RegisterPlugIn(pPlugIn);

    auto scene = eh::Scene::create();

    std::filesystem::path fpath(path);

    if (SceneIO::getInstance().read(fpath.wstring(), scene) == false)
        return false;

    IDriver *gl = nullptr;
    if (window == nullptr)
        gl = InitGL();
    else
        gl = CreateOpenGL1Driver((int *)window);

    _vp = new Viewport(gl);

    _vp->setScene(scene);
    _vp->setDisplayRect(0, 0, width, height);

    return true;
}

extern "C" __declspec(dllexport) void DrawScene(int width, int height)
{
    if (_vp != nullptr)
    {
        _vp->setDisplayRect(0, 0, width, height);
        _vp->drawScene();
    }
}

extern "C" __declspec(dllexport) void MouseDown(int button, int x, int y)
{
    _vp->control().OnMouseDown(button, x, y);
}

extern "C" __declspec(dllexport) void MouseUp(int button, int x, int y)
{
    _vp->control().OnMouseUp(button, x, y);
}

extern "C" __declspec(dllexport) void MouseMove(int button, int x, int y)
{
    _vp->control().OnMouseMove(button, x, y);
}

extern "C" __declspec(dllexport) void MouseWheel(int button, int zDelta, int x, int y)
{
    _vp->control().OnMouseWheel(button, zDelta, x, y);
}

int main()
{
    printf("teapot-viewer!\n");

    if (LoadModel("bin\\teapot.obj.zip", nullptr) == false)
        return -1;

    SaveBitmap("bin\\out.bmp");
    printf("END\n");

    return 0;
}