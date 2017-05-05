
%module(directors="1") "eh"
%{
        #include "swig.h"
%}

%include <windows.i>
%include <std_shared_ptr.i>
%include <std_string.i>
%include <std_wstring.i>
%include <std_vector.i>
%include <cs_struct.i>

%feature("director") Callback; //https://github.com/swig/swig/tree/master/Examples/csharp/callback

%cs_struct(HWND, System.IntPtr)
%cs_struct(HBITMAP, System.IntPtr) 
%cs_struct(eh::IDriver, System.IntPtr) 

%apply void *VOID_INT_PTR { void * }

%shared_ptr(eh::IViewport);

%template(IntVector) std::vector<int>;
%template(StringVector) std::vector<std::wstring>;


%include "swig.h"
