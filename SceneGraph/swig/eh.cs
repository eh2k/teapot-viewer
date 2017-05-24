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

public class eh {
  public static float PI {
    get {
      float ret = ehPINVOKE.PI_get();
      return ret;
    } 
  }

  public static bool fequal(float a, float b) {
    bool ret = ehPINVOKE.fequal(a, b);
    return ret;
  }

  public static float fmin(float a, float b) {
    float ret = ehPINVOKE.fmin(a, b);
    return ret;
  }

  public static float fmax(float a, float b) {
    float ret = ehPINVOKE.fmax(a, b);
    return ret;
  }

  public static void fswap(SWIGTYPE_p_float a, SWIGTYPE_p_float b) {
    ehPINVOKE.fswap(SWIGTYPE_p_float.getCPtr(a), SWIGTYPE_p_float.getCPtr(b));
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
  }

  public static float DEG2RAD(float x) {
    float ret = ehPINVOKE.DEG2RAD(x);
    return ret;
  }

  public static float RAD2DEG(float x) {
    float ret = ehPINVOKE.RAD2DEG(x);
    return ret;
  }

  public static float dot(Vec3 v1, Vec3 v2) {
    float ret = ehPINVOKE.dot(Vec3.getCPtr(v1), Vec3.getCPtr(v2));
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static float distance(Vec3 v1, Vec3 v2) {
    float ret = ehPINVOKE.distance(Vec3.getCPtr(v1), Vec3.getCPtr(v2));
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static float distanceSq(Vec3 v1, Vec3 v2) {
    float ret = ehPINVOKE.distanceSq(Vec3.getCPtr(v1), Vec3.getCPtr(v2));
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static float angle(Vec3 v1, Vec3 v2) {
    float ret = ehPINVOKE.angle(Vec3.getCPtr(v1), Vec3.getCPtr(v2));
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static Vec3 cross(Vec3 v1, Vec3 v2) {
    Vec3 ret = new Vec3(ehPINVOKE.cross(Vec3.getCPtr(v1), Vec3.getCPtr(v2)), true);
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static Vec3 calcNormal(Vec3 a, Vec3 b, Vec3 c) {
    Vec3 ret = new Vec3(ehPINVOKE.calcNormal(Vec3.getCPtr(a), Vec3.getCPtr(b), Vec3.getCPtr(c)), true);
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static Vec3 transform(Vec3 v, Matrix m) {
    Vec3 ret = new Vec3(ehPINVOKE.transform__SWIG_0(Vec3.getCPtr(v), Matrix.getCPtr(m)), true);
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static Vec4 transform(Vec4 v, Matrix m) {
    Vec4 ret = new Vec4(ehPINVOKE.transform__SWIG_1(Vec4.getCPtr(v), Matrix.getCPtr(m)), true);
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static Vec3 project(Vec3 v, Rect viewport, Matrix proj, Matrix view) {
    Vec3 ret = new Vec3(ehPINVOKE.project(Vec3.getCPtr(v), Rect.getCPtr(viewport), Matrix.getCPtr(proj), Matrix.getCPtr(view)), true);
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static Vec3 unproject(Vec3 v, Rect viewport, Matrix proj, Matrix view) {
    Vec3 ret = new Vec3(ehPINVOKE.unproject(Vec3.getCPtr(v), Rect.getCPtr(viewport), Matrix.getCPtr(proj), Matrix.getCPtr(view)), true);
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static Ray transform(Ray ray, Matrix m) {
    Ray ret = new Ray(ehPINVOKE.transform__SWIG_2(Ray.getCPtr(ray), Matrix.getCPtr(m)), true);
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static AABBox transform(AABBox box, Matrix m) {
    AABBox ret = new AABBox(ehPINVOKE.transform__SWIG_3(AABBox.getCPtr(box), Matrix.getCPtr(m)), true);
    if (ehPINVOKE.SWIGPendingException.Pending) throw ehPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}

}
