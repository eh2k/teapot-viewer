#include <stdio.h>
#include <assert.h>
#include "IDriver.h"
#include "Viewport.h"
#include "SceneIO.h"
#include "Controller.h"
#include "core.h"
#include "plugin.h"
#include "../res/teapot_obj.h"
#include "_cgo_export.h"

#define CONTEXT void *

using namespace eh;

Uint NewOpenGLTexture(void *ptr, size_t size)
{
    return goNewOpenGLTexture(ptr, size);
}

size_t TryReadFromZip(const char* path, void** data)
{
    return goTryReadFromZip(path, data);
}

namespace eh{
Ptr<Scene> loadOBJfromStream(std::istream& stream, SceneIO::progress_callback& progress);
}

extern "C"
{

    IDriver *CreateOpenGL1Driver(int *pWindow);

    API_3D void RegisterImporter2(void* plugIn)
    {
        auto p = (SceneIO::IPlugIn*)plugIn;
        std::wcout << p->about() << std::endl;
        SceneIO::getInstance().RegisterPlugIn(std::shared_ptr<SceneIO::IPlugIn>(p));
    }

    API_3D void RegisterImporter(void* plugIn)
    {
        auto p = (IImportPlugIn*)plugIn;
        std::wcout << p->about() << std::endl;
        SceneIO::getInstance().RegisterPlugIn(p);
    }

    API_3D CONTEXT LoadTeapot()
    {
        struct dummy {static void callback(float f){} };
        SceneIO::progress_callback dummy_cb(dummy::callback);

        std::stringstream teapot;
        teapot.write((const char*)teapot_obj, teapot_obj_len); //Todo: Optimize
        teapot.seekg(0);

        auto scene = loadOBJfromStream(teapot, dummy_cb);

        auto _vp = new Viewport(nullptr);
        _vp->setScene(scene);
        return _vp;
    }

    API_3D CONTEXT TryLoadModel(const char *path)
    {
        auto scene = eh::Scene::create();

        std::filesystem::path fpath(path);

        if (SceneIO::getInstance().read(fpath.wstring(), scene, goLoadModelProgressCB) == false)
            return nullptr;

        auto _vp = new Viewport(nullptr);
        _vp->setScene(scene);
        return _vp;
    }

    API_3D const char *GetSupportedFormats()
    {
        static std::string tmp;
        auto wtmp = SceneIO::getInstance().getFileWildcards();
        tmp = std::string(wtmp.begin(), wtmp.end());
        return tmp.c_str();
    }

    API_3D void DrawScene(CONTEXT context, int width, int height, void *window)
    {
        auto _vp = static_cast<Viewport *>(context);
        if (_vp != nullptr)
        {
            if (_vp->getDriver() == nullptr)
            {
                auto gl = CreateOpenGL1Driver((int *)window);
                _vp->setDriver(gl);
            }

            _vp->setDisplayRect(0, 0, width, height);
            _vp->drawScene();
        }
    }

    API_3D int ViewMode(CONTEXT context, int mode, int enable)
    {
        auto _vp = static_cast<Viewport *>(context);
        if (_vp != nullptr)
        {
            int r = (int)_vp->getModeFlag((Mode)mode);
            if (enable >= 0)
                _vp->setModeFlag((Mode)mode, enable);
            else if (enable == -2)
                _vp->setModeFlag((Mode)mode, !r);

            _vp->invalidate();

            return r;
        }

        return -1;
    }

    API_3D void MouseButton(CONTEXT context, int button, int x, int y, int down)
    {
        auto _vp = static_cast<Viewport *>(context);
        if (_vp != nullptr)
        {
            if (down)
                _vp->control().OnMouseDown(button, x, y);
            else
                _vp->control().OnMouseUp(button, x, y);
        }
    }

    API_3D void MouseMove(CONTEXT context, int button, int x, int y)
    {
        auto _vp = static_cast<Viewport *>(context);
        if (_vp != nullptr)
            _vp->control().OnMouseMove(button, x, y);
    }

    API_3D void MouseWheel(CONTEXT context, int button, int zDelta, int x, int y)
    {
        auto _vp = static_cast<Viewport *>(context);
        if (_vp != nullptr)
            _vp->control().OnMouseWheel(button, zDelta, x, y);
    }

    API_3D void SetCamera(CONTEXT context, int num)
    {
        auto _vp = static_cast<Viewport *>(context);
        if (_vp != nullptr)
        {
            if (num == 0)
            {
                _vp->setScene(_vp->getScene(), _vp->getScene()->createOrbitalCamera());
            }
            else
            {
                _vp->setScene(_vp->getScene(), _vp->getScene()->getCameras()[num - 1]);
            }

            _vp->invalidate();
        }
    }

    API_3D const char *GetCamera(CONTEXT context, int num)
    {
        auto _vp = static_cast<Viewport *>(context);
        if (_vp != nullptr)
        {
            if (num == 0)
            {
                return "Default";
            }
            else if ((num - 1) < _vp->getScene()->getCameras().size())
            {
                return _vp->getScene()->getCameras()[num - 1]->getName().c_str();
            }
        }

        return nullptr;
    }
}

#if 0

#include <windows.h>
#include <gl/gl.h>

void *_pBits;
HBITMAP _hBitmap;
HDC _hDC;
HGLRC _hRC;
int width = 800;
int height = 600;

#define VR(r) assert(r)

void InitGL()
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
}

extern "C" API_3D bool SaveBitmap(char *szPathName)
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

extern "C" API_3D void *GetBitmap(CONTEXT context)
{
    static_cast<Viewport *>(context)->drawScene();
    return _pBits;
}

int main()
{
    printf("teapot-viewer!\n");

    if (LoadModel("bin\\teapot.obj.zip") == false)
        return -1;

    SaveBitmap("bin\\out.bmp");
    printf("END\n");

    return 0;
}

#else

void InitGL()
{
}

extern "C" API_3D bool SaveBitmap(char *szPathName)
{
    return false;
}

extern "C" API_3D void *GetBitmap(CONTEXT context)
{
    static_cast<Viewport *>(context)->drawScene();
    return nullptr;
}

#endif