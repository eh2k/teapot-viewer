#ifdef __cplusplus
extern "C"
{
#endif

#define IMPORT
    //#define IMPORT __declspec(dllimport)

#define CONTEXT void *

    IMPORT void RegisterImporter(void* importer);
    IMPORT void RegisterImporter2(void* importer);

    IMPORT CONTEXT LoadTeapot();
    IMPORT CONTEXT TryLoadModel(const char *path);
    IMPORT void *GetBitmap(CONTEXT context);
    IMPORT void DrawScene(CONTEXT context, int width, int height, void *window);

    IMPORT void MouseButton(CONTEXT context, int button, int x, int y, int down);
    IMPORT void MouseMove(CONTEXT context, int button, int x, int y);
    IMPORT void MouseWheel(CONTEXT context, int button, int zDelta, int x, int y);

    IMPORT int ViewMode(CONTEXT context, int mode, int enable);
    IMPORT void SetCamera(CONTEXT context, int num);
    IMPORT const char *GetCamera(CONTEXT context, int num);
    IMPORT const char* GetSupportedFormats();

#ifdef __cplusplus
}
#endif