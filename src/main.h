#ifdef __cplusplus
  extern "C" {
#endif

#define CONTEXT void*

__declspec(dllimport) CONTEXT LoadModel(const char* path);
__declspec(dllimport) void* GetBitmap(CONTEXT context);
__declspec(dllimport) void  DrawScene(CONTEXT context, int width, int height, void* window);

__declspec(dllimport) void MouseButton(CONTEXT context, int button, int x, int y, int down);
__declspec(dllimport) void MouseMove(CONTEXT context, int button, int x, int y);
__declspec(dllimport) void MouseWheel(CONTEXT context, int button, int zDelta, int x, int y);

__declspec(dllimport) int ViewMode(CONTEXT context, int mode, int enable);

#ifdef __cplusplus
  }
#endif

