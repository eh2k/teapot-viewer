
//mingw-w64-x86_64-assimp
#define XcreatePlugIn CreateAssimpLoader
#include "../assimp_loader/src/AssimpLoader.cpp"

#define XcreatePlugIn Create3DSLoader
#include "../lib3ds_loader/src/io3DS.cpp"
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_atmosphere.c"
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_background.c" 
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_camera.c"
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_chunk.c"
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_chunktable.c" 
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_file.c"
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_io.c"
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_light.c" 
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_material.c" 
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_math.c"
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_matrix.c" 
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_mesh.c"
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_node.c"
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_quat.c"
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_shadow.c" 
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_track.c"
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_util.c"
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_vector.c" 
#include "../lib3ds_loader/lib3ds-20080909/src/lib3ds_viewport.c"
