//------------------------------------------------------------------------------
// <auto-generated />
//
// This file was automatically generated by SWIG (http://www.swig.org).
// Version 3.0.12
//
// Do not make changes to this file unless you know what you are doing--modify
// the SWIG interface file instead.
//------------------------------------------------------------------------------

namespace eh {

public class Plane : global::System.IDisposable {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal Plane(global::System.IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(Plane obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  ~Plane() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != global::System.IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          ehPINVOKE.delete_Plane(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
      global::System.GC.SuppressFinalize(this);
    }
  }

  public Plane(Vec3 p1, Vec3 p2, Vec3 p3) : this(ehPINVOKE.new_Plane__SWIG_0(Vec3.getCPtr(p1), Vec3.getCPtr(p2), Vec3.getCPtr(p3)), true) {
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
  }

  public Plane(float nx, float ny, float nz, float dd) : this(ehPINVOKE.new_Plane__SWIG_1(nx, ny, nz, dd), true) {
  }

  public Plane(float nx, float ny, float nz) : this(ehPINVOKE.new_Plane__SWIG_2(nx, ny, nz), true) {
  }

  public Plane(float nx, float ny) : this(ehPINVOKE.new_Plane__SWIG_3(nx, ny), true) {
  }

  public Plane(float nx) : this(ehPINVOKE.new_Plane__SWIG_4(nx), true) {
  }

  public Plane() : this(ehPINVOKE.new_Plane__SWIG_5(), true) {
  }

  public Vec3 getNormal() {
    Vec3 ret = new Vec3(ehPINVOKE.Plane_getNormal(swigCPtr), false);
    return ret;
  }

  public float getD() {
    float ret = ehPINVOKE.Plane_getD(swigCPtr);
    return ret;
  }

  public bool getIntersectionWithLine(Vec3 p1, Vec3 p2, SWIGTYPE_p_float t) {
    bool ret = ehPINVOKE.Plane_getIntersectionWithLine(swigCPtr, Vec3.getCPtr(p1), Vec3.getCPtr(p2), SWIGTYPE_p_float.getCPtr(t));
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool getIntersectionWithPlane(Plane other, Vec3 outLinePoint, Vec3 outLineVect) {
    bool ret = ehPINVOKE.Plane_getIntersectionWithPlane(swigCPtr, Plane.getCPtr(other), Vec3.getCPtr(outLinePoint), Vec3.getCPtr(outLineVect));
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}

}
