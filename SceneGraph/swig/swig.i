
%module(directors="1") "eh"
%{
		#include "..\src\math3d.hpp"
        #include "swig.h"
%}

%include <windows.i>
%include <std_shared_ptr.i>
%include <std_string.i>
%include <std_wstring.i>
%include <std_vector.i>
%include <cs_struct.i>

%cs_struct(HWND, System.IntPtr)
%cs_struct(HBITMAP, System.IntPtr) 
%cs_struct(eh::IDriver, System.IntPtr) 

%apply void *VOID_INT_PTR { void * }

%shared_ptr(swig::ISceneNode);
%shared_ptr(swig::IShapeNode);
%shared_ptr(swig::IGroupNode);

%shared_ptr(swig::IPlugIn);
%shared_ptr(swig::IViewport);
%shared_ptr(swig::IGeometry);
%shared_ptr(swig::IMaterial);

%template(IntVector) std::vector<int>;
%template(StringVector) std::vector<std::wstring>;
%template(Vec3Vector) std::vector<math3D::Vec3>;

%feature("director") Callback; //https://github.com/swig/swig/tree/master/Examples/csharp/callback

%typemap(imtype, 
	inattributes="[global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.LPWStr)]",
	directoroutattributes="[return: global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.LPWStr)]") std::wstring "string"

%feature("director") IPlugIn;

%include "..\src\math3d.hpp"
%include "..\src\ViewportModes.h"
%include "swig.h"
