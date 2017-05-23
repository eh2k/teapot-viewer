/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.12
 *
 * This file is not intended to be easily readable and contains a number of
 * coding conventions designed to improve portability and efficiency. Do not make
 * changes to this file unless you know what you are doing--modify the SWIG
 * interface file instead.
 * ----------------------------------------------------------------------------- */

#ifndef SWIG_eh_WRAP_H_
#define SWIG_eh_WRAP_H_

struct SwigDirector_Callback : public swig::Callback, public Swig::Director {

public:
    SwigDirector_Callback();
    virtual ~SwigDirector_Callback();
    virtual void call(float value);

    typedef void (SWIGSTDCALL* SWIG_Callback0_t)(float);
    void swig_connect_director(SWIG_Callback0_t callbackcall);

private:
    SWIG_Callback0_t swig_callbackcall;
    void swig_init_callbacks();
};

struct SwigDirector_IPlugIn : public swig::IPlugIn, public Swig::Director {

public:
    SwigDirector_IPlugIn();
    virtual std::wstring about() const;
    virtual int file_type_count() const;
    virtual std::wstring file_type(int i);
    virtual std::wstring file_exts(int i);
    virtual bool canWrite(int i) const;
    virtual bool canRead(int i) const;
    virtual bool readFile(std::wstring aFile, void *sceneHandle, swig::Callback *callback = nullptr);
    virtual bool writeFile(std::wstring sFile, void *sceneHandle, swig::Callback *callback = nullptr);

    typedef wchar_t * (SWIGSTDCALL* SWIG_Callback0_t)();
    typedef int (SWIGSTDCALL* SWIG_Callback1_t)();
    typedef wchar_t * (SWIGSTDCALL* SWIG_Callback2_t)(int);
    typedef wchar_t * (SWIGSTDCALL* SWIG_Callback3_t)(int);
    typedef unsigned int (SWIGSTDCALL* SWIG_Callback4_t)(int);
    typedef unsigned int (SWIGSTDCALL* SWIG_Callback5_t)(int);
    typedef unsigned int (SWIGSTDCALL* SWIG_Callback6_t)(void *, void *, void *);
    typedef unsigned int (SWIGSTDCALL* SWIG_Callback7_t)(void *, void *);
    typedef unsigned int (SWIGSTDCALL* SWIG_Callback8_t)(void *, void *, void *);
    typedef unsigned int (SWIGSTDCALL* SWIG_Callback9_t)(void *, void *);
    void swig_connect_director(SWIG_Callback0_t callbackabout, SWIG_Callback1_t callbackfile_type_count, SWIG_Callback2_t callbackfile_type, SWIG_Callback3_t callbackfile_exts, SWIG_Callback4_t callbackcanWrite, SWIG_Callback5_t callbackcanRead, SWIG_Callback6_t callbackreadFile__SWIG_0, SWIG_Callback7_t callbackreadFile__SWIG_1, SWIG_Callback8_t callbackwriteFile__SWIG_0, SWIG_Callback9_t callbackwriteFile__SWIG_1);

private:
    SWIG_Callback0_t swig_callbackabout;
    SWIG_Callback1_t swig_callbackfile_type_count;
    SWIG_Callback2_t swig_callbackfile_type;
    SWIG_Callback3_t swig_callbackfile_exts;
    SWIG_Callback4_t swig_callbackcanWrite;
    SWIG_Callback5_t swig_callbackcanRead;
    SWIG_Callback6_t swig_callbackreadFile__SWIG_0;
    SWIG_Callback7_t swig_callbackreadFile__SWIG_1;
    SWIG_Callback8_t swig_callbackwriteFile__SWIG_0;
    SWIG_Callback9_t swig_callbackwriteFile__SWIG_1;
    void swig_init_callbacks();
};


#endif
