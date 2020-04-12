#ifdef __cplusplus
  extern "C" {
#endif

__declspec(dllimport) int   LoadModel(const char* path, void* window);
__declspec(dllimport) void* GetBitmap();
__declspec(dllimport) void  DrawScene(int width, int height);

__declspec(dllimport) void MouseDown(int button, int x, int y);
__declspec(dllimport) void MouseUp(int button, int x, int y);
__declspec(dllimport) void MouseMove(int button, int x, int y);
__declspec(dllimport) void MouseWheel(int button, int zDelta, int x, int y);

#ifdef __cplusplus
  }
#endif

