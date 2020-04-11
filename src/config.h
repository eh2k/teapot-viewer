#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef __linux__
#define _MSC_VER 1200
#endif

#define API_3D

// #if defined(_WIN32) || defined(__CYGWIN__)
//   #ifdef _API_EXPORT
//     #ifdef __GNUC__
//       #define API_3D __attribute__((dllexport))
//     #else
//       #define API_3D __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
//     #endif
//   #else
//     #ifdef __GNUC__
//       #define API_3D __attribute__((dllimport))
//     #else
//       #define API_3D __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
// 	#endif
//   #endif
// #else
//   #if __GNUC__ >= 4
//     #define API_3D __attribute__ ((visibility("default")))
//   #else
//     #define API_3D
//   #endif
// #endif


#include "math3d.hpp"

namespace eh{
    using namespace math3D;
}

