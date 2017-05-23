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

public class Ray : global::System.IDisposable {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal Ray(global::System.IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(Ray obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  ~Ray() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != global::System.IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          ehPINVOKE.delete_Ray(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
      global::System.GC.SuppressFinalize(this);
    }
  }

  public Ray(Vec3 origin, Vec3 direction) : this(ehPINVOKE.new_Ray__SWIG_0(Vec3.getCPtr(origin), Vec3.getCPtr(direction)), true) {
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
  }

  public Ray(int x, int y, Rect viewport, Matrix proj, Matrix view) : this(ehPINVOKE.new_Ray__SWIG_1(x, y, Rect.getCPtr(viewport), Matrix.getCPtr(proj), Matrix.getCPtr(view)), true) {
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
  }

  public Vec3 getOrigin() {
    Vec3 ret = new Vec3(ehPINVOKE.Ray_getOrigin(swigCPtr), false);
    return ret;
  }

  public Vec3 getDirection() {
    Vec3 ret = new Vec3(ehPINVOKE.Ray_getDirection(swigCPtr), false);
    return ret;
  }

  public bool getIntersectionWithTriangle(Vec3 vert0, Vec3 vert1, Vec3 vert2, SWIGTYPE_p_float t) {
    bool ret = ehPINVOKE.Ray_getIntersectionWithTriangle(swigCPtr, Vec3.getCPtr(vert0), Vec3.getCPtr(vert1), Vec3.getCPtr(vert2), SWIGTYPE_p_float.getCPtr(t));
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool getIntersectionWithAABBox(AABBox box, SWIGTYPE_p_float t, float t0, float t1) {
    bool ret = ehPINVOKE.Ray_getIntersectionWithAABBox__SWIG_0(swigCPtr, AABBox.getCPtr(box), SWIGTYPE_p_float.getCPtr(t), t0, t1);
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool getIntersectionWithAABBox(AABBox box, SWIGTYPE_p_float t, float t0) {
    bool ret = ehPINVOKE.Ray_getIntersectionWithAABBox__SWIG_1(swigCPtr, AABBox.getCPtr(box), SWIGTYPE_p_float.getCPtr(t), t0);
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool getIntersectionWithAABBox(AABBox box, SWIGTYPE_p_float t) {
    bool ret = ehPINVOKE.Ray_getIntersectionWithAABBox__SWIG_2(swigCPtr, AABBox.getCPtr(box), SWIGTYPE_p_float.getCPtr(t));
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public float getSqDistanceToPoint(Vec3 p, SWIGTYPE_p_float t) {
    float ret = ehPINVOKE.Ray_getSqDistanceToPoint(swigCPtr, Vec3.getCPtr(p), SWIGTYPE_p_float.getCPtr(t));
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool getIntersectionWithSphere(Vec3 center, float radius, SWIGTYPE_p_float t) {
    bool ret = ehPINVOKE.Ray_getIntersectionWithSphere(swigCPtr, Vec3.getCPtr(center), radius, SWIGTYPE_p_float.getCPtr(t));
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public float getSqDistanceToRay(Ray other, SWIGTYPE_p_float pT) {
    float ret = ehPINVOKE.Ray_getSqDistanceToRay__SWIG_0(swigCPtr, Ray.getCPtr(other), SWIGTYPE_p_float.getCPtr(pT));
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public float getSqDistanceToRay(Ray other) {
    float ret = ehPINVOKE.Ray_getSqDistanceToRay__SWIG_1(swigCPtr, Ray.getCPtr(other));
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void getClosestPointToRay(Ray other, SWIGTYPE_p_float t) {
    ehPINVOKE.Ray_getClosestPointToRay(swigCPtr, Ray.getCPtr(other), SWIGTYPE_p_float.getCPtr(t));
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
  }

  public Vec3 getPointAt(float t) {
    Vec3 ret = new Vec3(ehPINVOKE.Ray_getPointAt(swigCPtr, t), true);
    return ret;
  }

}

}
