/*
    Copyright (C) 2007-2010 by E.Heidt
    All rights reserved.

    This program is free  software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 2.1 of the License, or
    (at your option) any later version.

    Thisprogram  is  distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should  have received a copy of the GNU Lesser General Public License
    along with  this program; If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <math.h>
#include <limits>
#include <vector>
#include <iostream>

namespace math3D
{

	class Rect;
	class Matrix;
	class AABBox;
	class Plane;

	typedef float    Float;
	typedef unsigned Uint;

	static const Float PI = 3.14159265358979323846f;

	typedef std::vector<Uint> Uint_vec;

	inline bool fequal(Float a, Float b)
	{
		return fabs(a - b) <= std::numeric_limits<Float>::epsilon();
	}

	inline Float fmin(Float a, Float b)
	{
		return (a<b)?a:b;
	}
	inline Float fmax(Float a, Float b)
	{
		return (a>b)?a:b;
	}

	inline void fswap(Float& a, Float& b)
	{
		Float tmp = a;
		a = b;
		b = tmp;
	}

	inline Float DEG2RAD(Float x)
	{
		return x/180.f*PI;
	}

	inline Float RAD2DEG(Float x)
	{
		return x*180.f/PI;
	}

	// Vec3 //
	class Point
	{
	public:
		int x;
		int y;
		Point(int _x = 0, int _y = 0):x(_x), y(_y) {}
	};

	// Vec3 //
	class Vec3
	{
	public:
		Float x, y, z;

		Vec3():x(0),y(0),z(0)
		{}
		Vec3(Float x, Float y, Float z):x(x),y(y),z(z)
		{}
		Vec3(const Vec3 &b):x(b.x),y(b.y),z(b.z)
		{}

		static const Vec3& Null()
		{
			static Vec3 null(0,0,0);
			return null;
		}

		Float& operator [] (unsigned idx)
		{
			switch(idx)
			{
			case 0: return x;
			case 1: return y;
			default: return z;
			}
		};

		bool operator<(const Vec3& other) const
		{
			return getSqLen() < other.getSqLen();
		}
		bool operator==(const Vec3 &v2) const
		{
			return fequal(x,v2.x) && fequal(y,v2.y) && fequal(z,v2.z);
		}
		bool operator!=(const Vec3 &v2) const
		{
			return !(*this == v2);
		}
		Float getLen() const
		{
			return sqrt(getSqLen());
		}
		Float getSqLen() const
		{
			return ((x*x)+(y*y)+(z*z));
		}
		Vec3 operator/(const Float &f) const
		{
			return Vec3((x/f),(y/f),(z/f));
		}
		Vec3 operator*(const Float &f) const
		{
			return Vec3((x*f),(y*f),(z*f));
		}
		Vec3 operator*(const Vec3& v) const
		{
			return Vec3((x*v.x),(y*v.y),(z*v.z));
		}
		void operator *=(const Float &f)
		{
			*this = *this * f;
		}
		Vec3 operator+(const Vec3 &v) const
		{
			return Vec3(x+v.x, y+v.y, z+v.z);
		}
		void operator +=(const Vec3 &v)
		{
			*this = *this + v;
		}
		Vec3 operator-(const Vec3 &v) const
		{
			return Vec3(x-v.x, y-v.y, z-v.z);
		}
		void operator -=(const Vec3 &v)
		{
			*this = *this - v;
		}
		Vec3 abs()
		{
			return Vec3(fabs(x), fabs(y), fabs(z));
		}
		Vec3 normalized() const
		{
			Float len = getLen();
			if( !fequal(len, 0.f) )
				return *this * (1.f/len);
			else
				return Vec3::Null();
		}
	};
	inline Float dot(const Vec3& v1, const Vec3& v2)
	{
		return ((v1.x*v2.x)+(v1.y*v2.y)+(v1.z*v2.z));
	}

	inline Float distance(const Vec3& v1, const Vec3& v2)
	{
		return (v2-v1).getLen();
	}

	inline Float distanceSq(const Vec3& v1, const Vec3 &v2)
	{
		return (v2-v1).getSqLen();
	}

	inline Float angle(const Vec3& v1, const Vec3& v2)
	{
		Float c = dot(v1, v2)/(v1.getLen()*v2.getLen());

		if(c>-1 && c<1)
			return RAD2DEG(acos(c));

		if(c<=-1)
			return 180;

		return 0;
	}

	inline Vec3 cross(const Vec3 &v1, const Vec3 &v2)
	{
		return Vec3(v1.y*v2.z-v1.z*v2.y, v1.z*v2.x-v1.x*v2.z, v1.x*v2.y-v1.y*v2.x);
	}

	inline Vec3 calcNormal(const Vec3& a, const Vec3& b, const Vec3& c)
	{
		return cross(b-a, c-a).normalized();
	}

	// Vec4 //
	class Vec4
	{
	public:
		Float x, y, z, w;
		Vec4(Float x, Float y, Float z, Float w):x(x),y(y),z(z),w(w){}
		Vec4(const Vec3& v, Float w):x(v.x),y(v.y),z(v.z),w(w){}
		Vec4(const Vec4& v):x(v.x),y(v.y),z(v.z),w(v.w){}
	};

	// Rect //
	class Rect
	{
		Float x;
		Float y;
		Float dx;
		Float dy;

	public:
		Rect():x(0),y(0),dx(0),dy(0){}
		Rect(Float _x, Float _y, Float _dx, Float _dy)
			:x(_x),y(_y),dx(_dx),dy(_dy){}

		void OffsetRect(Float _x, Float _y)
		{
			x += _x;
			y += _y;
		}

		void setLTRB(Float l, Float t, Float r, Float b)
		{
			x = l;
			y = t;
			dx = r-l;
			dy = b-t;
		}

		void setRight(Float r)
		{
			dx = r - x;
		}

		void setBottom(Float b)
		{
			dy = b - y;
		}

		Float Width() const { return dx; }
		Float Height() const { return dy; }
		Float Left() const { return x; }
		Float Right() const { return x + dx; }
		Float Top() const { return y; }
		Float Bottom() const { return y + dy; }
	};

	// Matrix //
	class Matrix
	{
	private:
		Float data[16];
	public:
		Matrix(bool bInit = true)
		{
			if(bInit)
				loadIdentity();
		}
		Matrix(const Matrix &m)
		{
		    for(int i = 0; i < 16; i++)
			data[i] = m.data[i];
		}
		Matrix(Float a1, Float a2, Float a3, Float a4,
		       Float b1, Float b2, Float b3, Float b4,
		       Float c1, Float c2, Float c3, Float c4,
		       Float d1, Float d2, Float d3, Float d4 )
		{
			data[0] = a1;
			data[1] = a2;
			data[2] = a3;
			data[3] = a4;
			data[4] = b1;
			data[5] = b2;
			data[6] = b3;
			data[7] = b4;
			data[8] = c1;
			data[9] = c2;
			data[10] = c3;
			data[11] = c4;
			data[12] = d1;
			data[13] = d2;
			data[14] = d3;
			data[15] = d4;
		}
		~Matrix()
		{};

		inline void loadIdentity()
		{
			for (int i=0;i<4;i++)
				for (int j=0;j<4;j++)
					data[i + 4 * j] = (j==i)?1.0f:0.0f;
		}

		static const Matrix& Identity()
		{
			static Matrix identity(true);
			return identity;
		}
		static Matrix Inverse(const Matrix &m);
		static Matrix Translation(const Vec3 &t);
		static Matrix Scale(const Vec3 &s);
		static Matrix Rotation(const Vec3& axis, const Float angle);
		static Matrix Frustum(Float l, Float r, Float b, Float t, Float n, Float f);
		static Matrix Ortho(Float l, Float r, Float b, Float t, Float n, Float f);
		static Matrix Lookat(const Vec3& eye, const Vec3& center, const Vec3& up);
		static Matrix Shadow(const Plane& groundplane, const Vec3& light);

		Matrix operator*( const Matrix &B) const
		{
			Matrix ret(false);

			ret.data[0] = data[0]*B.data[0] + data[1]*B.data[4] + data[2]*B.data[8] + data[3]*B.data[12];
			ret.data[1] = data[0]*B.data[1] + data[1]*B.data[5] + data[2]*B.data[9] + data[3]*B.data[13];
			ret.data[2] = data[0]*B.data[2] + data[1]*B.data[6] + data[2]*B.data[10] + data[3]*B.data[14];
			ret.data[3] = data[0]*B.data[3] + data[1]*B.data[7] + data[2]*B.data[11] + data[3]*B.data[15];

			ret.data[4] = data[4]*B.data[0] + data[5]*B.data[4] + data[6]*B.data[8] + data[7]*B.data[12];
			ret.data[5] = data[4]*B.data[1] + data[5]*B.data[5] + data[6]*B.data[9] + data[7]*B.data[13];
			ret.data[6] = data[4]*B.data[2] + data[5]*B.data[6] + data[6]*B.data[10] + data[7]*B.data[14];
			ret.data[7] = data[4]*B.data[3] + data[5]*B.data[7] + data[6]*B.data[11] + data[7]*B.data[15];

			ret.data[8] = data[8]*B.data[0] + data[9]*B.data[4] + data[10]*B.data[8] + data[11]*B.data[12];
			ret.data[9] = data[8]*B.data[1] + data[9]*B.data[5] + data[10]*B.data[9] + data[11]*B.data[13];
			ret.data[10] = data[8]*B.data[2] + data[9]*B.data[6] + data[10]*B.data[10] + data[11]*B.data[14];
			ret.data[11] = data[8]*B.data[3] + data[9]*B.data[7] + data[10]*B.data[11] + data[11]*B.data[15];

			ret.data[12] = data[12]*B.data[0] + data[13]*B.data[4] + data[14]*B.data[8] + data[15]*B.data[12];
			ret.data[13] = data[12]*B.data[1] + data[13]*B.data[5] + data[14]*B.data[9] + data[15]*B.data[13];
			ret.data[14] = data[12]*B.data[2] + data[13]*B.data[6] + data[14]*B.data[10] + data[15]*B.data[14];
			ret.data[15] = data[12]*B.data[3] + data[13]*B.data[7] + data[14]*B.data[11] + data[15]*B.data[15];

			return ret;
		}

		//Float& at(Uint row, Uint col)
		//{
		//	return data[(col<<2)+row];
		//}

		//const Float& at(Uint row, Uint col) const
		//{
		//	return data[(col<<2)+row];
		//}

		Float& operator [] (int idx)
		{
			return data[idx];
		}
		const Float& operator [] (int idx) const
		{
			return data[idx];
		};

		bool operator==(const Matrix& other) const
		{
			for(int i = 0; i < 16; i++)
				if(fequal(this->data[i],other.data[i]) == false)
					return false;
			return true;
		}

		bool operator!=(const Matrix& other) const
		{
			return (*this == other) == false;
		}

		Matrix getInverted() const
		{
			return Inverse(*this);
		}

		class Stack;
	};

	class Matrix::Stack
	{
		std::vector<Matrix> stack;
	public:
		Stack()
		{
			stack.push_back(Matrix());
		}
		~Stack()
		{
		}

		void clear(const Matrix& mat)
		{
			stack.resize(0);
			stack.push_back(mat);
		}

		size_t getStackSize() const
		{
			return stack.size();
		}

		const Matrix& getTop() const
		{
			return stack.back();
		}

		void set(const Matrix& m)
		{
			stack.back() = m;
		}
		void translate(const Vec3& v)
		{
			stack.back() = Matrix::Translation(v) * stack.back();
		}
		void rotate(const Vec3& v, const Float a)
		{
			stack.back() = Matrix::Rotation(v,a) * stack.back();
		}
		void scale(const Vec3& v)
		{
			stack.back() = Matrix::Scale(v) * stack.back();
		}
		void mult(const Matrix& m)
		{
			stack.back() = m * stack.back();
		}
		void push()
		{
			stack.push_back(getTop());
		}
		void pop()
		{
			stack.pop_back();
		}
		void loadIdentity()
		{
			set(Matrix());
		}
	};

	inline Vec3 transform(const Vec3& v, const Matrix &m)
	{
		Float xx = v.x*m[0] + v.y*m[4] + v.z*m[ 8] + 1.f*m[12];
		Float yy = v.x*m[1] + v.y*m[5] + v.z*m[ 9] + 1.f*m[13];
		Float zz = v.x*m[2] + v.y*m[6] + v.z*m[10] + 1.f*m[14];
		Float aa = v.x*m[3] + v.y*m[7] + v.z*m[11] + 1.f*m[15];

		Vec3 ret(xx/aa,yy/aa,zz/aa);

		return ret;
	}

	inline Vec4 transform(const Vec4& v, const Matrix &m)
	{
		Float xx = v.x*m[0] + v.y*m[4] + v.z*m[ 8] + v.w*m[12];
		Float yy = v.x*m[1] + v.y*m[5] + v.z*m[ 9] + v.w*m[13];
		Float zz = v.x*m[2] + v.y*m[6] + v.z*m[10] + v.w*m[14];
		Float ww = v.x*m[3] + v.y*m[7] + v.z*m[11] + v.w*m[15];
		return Vec4(xx,yy,zz,ww);
	}

	inline Vec3 project(const Vec3& v, const Rect& viewport, const Matrix& proj, const Matrix& view)
	{
		Vec4 tmp = transform( transform( Vec4(v, 1.f), view), proj);

		/* d'ou le resultat normalise entre -1 et 1 */
		if( fequal(tmp.w, 0.f) )
			return Vec3::Null();

		tmp.x /= tmp.w;
		tmp.y /= tmp.w;
		tmp.z /= tmp.w;

		Float winx = viewport.Left() + (1 + tmp.x) * viewport.Width() / 2;
		Float winy = viewport.Top() + (1 + tmp.y) * viewport.Height() / 2;
		Float winz = (1 + tmp.z) / 2;

		return Vec3(winx, winy, winz);
	}
	inline Vec3 unproject(const Vec3& v, const Rect& viewport, const Matrix& proj, const Matrix& view)
	{
		Vec4 in((v.x - viewport.Left()) * 2.f / viewport.Width() - 1.f,
			(v.y - viewport.Top()) * 2.f / viewport.Height() - 1.f,
			2.f * v.z - 1.f,
			1.f);

		const Matrix& m = (proj * view).getInverted(); //matmul(A, proj, model); invert_matrix(A, m);

		Vec4 out = transform(in, m);

		if ( fequal(out.w,0.f) )
			return Vec3::Null();

		return Vec3(out.x/out.w, out.y/out.w, out.z/out.w);
	}

	// Plane //
	class Plane
	{
	private:
		Vec3 normal;
		Float d;		//Distanz vom Ursprung
	public:
		Plane(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3):
			normal(cross(p2-p1, p3-p1).normalized())
		{
			d = - dot(p1, normal);
		}
		Plane(Float nx = 0.f, Float ny = 0.f, Float nz = 1.f, Float dd = 0.f):
			normal(Vec3(nx,ny,nz).normalized()),
			d(dd)
		{
		}
		virtual ~Plane(){}

		inline const Vec3& getNormal() const
		{
			return normal;
		}

		inline Float getD() const
		{
			return d;
		}

		inline const Float& operator [] (int idx) const
		{
			switch(idx)
			{
			case 0: return normal.x;
			case 1: return normal.y;
			case 2: return normal.z;
			case 3: return d;
			default:
				return d;
			}
		};

		inline Float& operator [] (int idx)
		{
			switch(idx)
			{
			case 0: return normal.x;
			case 1: return normal.y;
			case 2: return normal.z;
			case 3: return d;
			default:
				return d;
			}
		};

		inline bool getIntersectionWithLine(const Vec3& p1,const Vec3& p2, Float& t) const
		{
			const Vec3 LineDir(p2-p1);
			const Float Denominator = ((normal.x * LineDir.x) + (normal.y * LineDir.y) + (normal.z * LineDir.z));

			t =  -((normal.x * p1.x) + (normal.y * p1.y) + (normal.z * p1.z) + d) / Denominator;

			if( t < 0.0f || t > 1.0f )	//Au�erhalb von [0; 1],
				return false;		//Schnittpunkt au�erhalb der Linie..

			return true;
		}

		inline bool getIntersectionWithPlane(const Plane& other, Vec3& outLinePoint, Vec3& outLineVect) const
		{
			Float fn00 = normal.getLen();
			Float fn01 = dot(normal, other.normal);
			Float fn11 = other.normal.getLen();
			Float det = fn00*fn11 - fn01*fn01;

			if (fequal(det, 0))
				return false;

			det = 1.f / det;
			Float fc0 = (fn11*-d + fn01*other.d) * det;
			Float fc1 = (fn00*-other.d + fn01*d) * det;

			outLineVect = cross(normal, other.normal);
			outLinePoint = normal*fc0 + other.normal*fc1;
			return true;
		}
	};

	// Ray //
	class Ray
	{
		Vec3 m_origin;
		Vec3 m_direction;
	public:
		Ray(const Vec3 &origin, const Vec3 &direction):
	      m_origin(origin),
		      m_direction(direction)
	      {
	      }
	      Ray(int x, int y, const Rect& viewport, const Matrix& proj, const Matrix& view)
	      {
		      y = ((int)viewport.Height()) - y;

		      Float fx = (2*(((Float)x)/(viewport.Width())))-1;
		      Float fy = (2*(((Float)y)/(viewport.Height())))-1;

		      const Matrix& mi = ( view * proj ).getInverted();

		      m_origin =  transform(Vec3(fx, fy, -1), mi);
		      m_direction  = transform(Vec3(fx, fy, 1), mi) - m_origin;
	      }
	      virtual ~Ray()
	      {}

	      const Vec3& getOrigin() const { return m_origin; }
	      const Vec3& getDirection() const { return m_direction; }

	      bool getIntersectionWithTriangle(const Vec3& vert0, const Vec3& vert1, const Vec3& vert2, Float &t) const;
	      bool getIntersectionWithAABBox(const AABBox& box, Float& t, Float t0 = 0, Float t1 = 1) const;
	      Float getSqDistanceToPoint(const Vec3& p, Float& t) const
	      {
		      t = 0;

		      Vec3 diff = p - m_origin;
		      t = dot(diff, m_direction);


		      if ( t <= 0.0f )
		      {
			      t = 0;
		      }
		      else
		      {
			      Float fDotMM = m_direction.getSqLen();

			      t = t / fDotMM;
			      diff = diff - m_direction*t;
		      }

		      return diff.getSqLen();
	      }

	      bool getIntersectionWithSphere(const Vec3& center, Float radius, Float& t) const
	      {

		      if(getSqDistanceToPoint(center, t) <= (radius*radius) )
		      {
			      return true;
		      }
		      else
			      return false;
	      }

	      Float getSqDistanceToRay(const Ray& other, Float* pT = NULL ) const
	      {
		      //Kann noch optimiert werden...

		      Float t = 0, t2 = 0;
		      if(pT == NULL)
			      pT = &t;
		      this->getClosestPointToRay(other, *pT);
		      other.getClosestPointToRay(*this, t2);

		      Vec3 a = this->getPointAt(*pT);
		      Vec3 b = other.getPointAt(t);

		      return distance(a,b);
	      }

	      void getClosestPointToRay(const Ray& other, Float& t) const
	      {
		      Vec3 cp = cross(m_direction, other.m_direction).normalized();

		      // Float distu = -dot(cp, m_origin);
		      // Float distv = -dot(cp, other.m_origin);
		      // Float dist = (Float)fabs(distu-distv);

		      Vec3 n = cross(other.m_direction, cp).normalized();
		      Float d = -dot(n, other.m_origin);

		      Float dn = dot(n, m_direction);
		      t = -(d-dot(n, m_origin))/dn;
	      }


	      Vec3 getPointAt(Float t) const { return m_origin + m_direction*t; }
	};


	inline Ray transform(const Ray& ray, const Matrix& m)
	{
		const Vec3& o = transform(ray.getOrigin(), m);
		return Ray(o, transform(ray.getOrigin()+ray.getDirection(), m)-o);
	}

	// AABBox //
	class AABBox
	{
	protected:
		Vec3 m_min;	//BoundingBox min
		Vec3 m_max;	//BoundingBox max
		Vec3 m_center;
		Vec3 m_size;
	public:
		enum INSIDE_RESULT
		{
			OUTSIDE   = 0,
			INSIDE    = 1,
			INTERSECT = 2,
		};

		AABBox(){}
		AABBox(const Vec3& min, const Vec3& max)
		{
			m_min.x = fmin(min.x, max.x);
			m_min.y = fmin(min.y, max.y);
			m_min.z = fmin(min.z, max.z);

			m_max.x = fmax(min.x, max.x);
			m_max.y = fmax(min.y, max.y);
			m_max.z = fmax(min.z, max.z);

			m_size = m_max-m_min;
			m_center = (m_min+m_max)*0.5f;
		}
		virtual ~AABBox(){}

		const Vec3& getMin() const {return m_min;}
		const Vec3& getMax() const {return m_max;}
		const Vec3& getSize() const {return m_size;}
		const Vec3& getCenter()const {return m_center;}
		Float getRadius() const { return getSize().getLen()/2; }

		const std::vector<Vec3> getCorners() const
		{
			std::vector<Vec3> ret;

			Vec3 size = getSize()*Float(0.5);

			ret.push_back( getCenter() + Vec3( size.x, size.y, size.z) );
			ret.push_back( getCenter() + Vec3( size.x, size.y,-size.z) );
			ret.push_back( getCenter() + Vec3( size.x,-size.y, size.z) );
			ret.push_back( getCenter() + Vec3( size.x,-size.y,-size.z) );
			ret.push_back( getCenter() + Vec3(-size.x, size.y, size.z) );
			ret.push_back( getCenter() + Vec3(-size.x, size.y,-size.z) );
			ret.push_back( getCenter() + Vec3(-size.x,-size.y, size.z) );
			ret.push_back( getCenter() + Vec3(-size.x,-size.y,-size.z) );

			return ret;
		}

		INSIDE_RESULT isInside(const Vec3 &p) const
		{
			if (p.x<m_min.x || p.x>m_max.x) return AABBox::OUTSIDE;
			if (p.y<m_min.y || p.y>m_max.y) return AABBox::OUTSIDE;
			if (p.z<m_min.z || p.z>m_max.z) return AABBox::OUTSIDE;
			return INSIDE; // Inside
		}

		INSIDE_RESULT isInside(const AABBox& box) const
		{
			INSIDE_RESULT a = isInside(box.m_min);
			INSIDE_RESULT b = isInside(box.m_max);

			if(a && b)
				return AABBox::INSIDE;	// Inside
			if(a || b)
				return AABBox::INTERSECT;	// Intersect
			return  AABBox::OUTSIDE;		// Outside
		}

		bool intersect(const AABBox& aab) const
		{
			return (fmax(aab.m_min.x,m_min.x) < fmin(aab.m_max.x,m_max.x)  &&
				fmax(aab.m_min.y,m_min.y) < fmin(aab.m_max.y,m_max.y)  &&
				fmax(aab.m_min.z,m_min.z) < fmin(aab.m_max.z,m_max.z));
		}

		AABBox operator+(const AABBox& aabb) const
		{
			if(m_min == m_max)
				return aabb;

			Vec3 min, max;
			min.x = fmin(this->m_min.x, aabb.m_min.x);
			min.y = fmin(this->m_min.y, aabb.m_min.y);
			min.z = fmin(this->m_min.z, aabb.m_min.z);

			max.x = fmax(this->m_max.x, aabb.m_max.x);
			max.y = fmax(this->m_max.y, aabb.m_max.y);
			max.z = fmax(this->m_max.z, aabb.m_max.z);

			return AABBox(min, max);
		}


		AABBox operator&(const AABBox& aabb) const
		{
			if(this->intersect(aabb) == false)
				return AABBox();	//Null Box

			Vec3 min, max;
			min.x = fmax(this->m_min.x, aabb.m_min.x);
			min.y = fmax(this->m_min.y, aabb.m_min.y);
			min.z = fmax(this->m_min.z, aabb.m_min.z);

			max.x = fmin(this->m_max.x, aabb.m_max.x);
			max.y = fmin(this->m_max.y, aabb.m_max.y);
			max.z = fmin(this->m_max.z, aabb.m_max.z);

			return AABBox(min, max);
		}
		void divide(AABBox& a, AABBox& b) const
		{
			Vec3 d;
			if(m_size.x >= m_size.y && m_size.x > m_size.z)	// in x wird Geteilt
				d = Vec3(m_size.x/2,0,0);
			else if (m_size.y > m_size.x)		// in y wird Getielt
				d = Vec3(0,m_size.y/2,0);
			else					// in z wird Getielt
				d = Vec3(0,0,m_size.z/2);

			Vec3 amin = m_min;
			Vec3 amax = m_max - d;

			Vec3 bmin = m_min + d;
			Vec3 bmax = m_max;

			a = AABBox(amin, amax);
			b = AABBox(bmin, bmax);
		}

		Float getVolume() const
		{
			return m_size.x * m_size.y * m_size.z;
		}

		bool valid() const
		{
			return (fequal(m_size.x, 0) && fequal(m_size.y, 0) && fequal(m_size.z, 0)) == false;
		}
	};

	inline AABBox transform(const AABBox& box, const Matrix& m)
	{
		bool first = true;
		Vec3 b_min, b_max;
		const std::vector<Vec3>& corners = box.getCorners();
		for(std::vector<Vec3>::const_iterator it = corners.begin(); it != corners.end(); it++)
		{
			Vec3 p = transform((*it), m);
			if(first)
			{
				b_min = p;
				b_max = p;
				first = false;
			}
			else
			{
				b_min.x = fmin(p.x, b_min.x);
				b_min.y = fmin(p.y, b_min.y);
				b_min.z = fmin(p.z, b_min.z);

				b_max.x = fmax(p.x, b_max.x);
				b_max.y = fmax(p.y, b_max.y);
				b_max.z = fmax(p.z, b_max.z);
			}
		}
		return AABBox(b_min, b_max);
	}

	// Frustum //
	class Frustum
	{
	private:
		Plane	 planeEqs[6];
	public:
		void extractFrom(const Matrix& proj, const Matrix& modl )
		{
			Float   clip[16];
			Float   t = 0;

			/* Combine the two matrices (multiply projection by modelview) */
			clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
			clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
			clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
			clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

			clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
			clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
			clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
			clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

			clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
			clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
			clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
			clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

			clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
			clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
			clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
			clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

			/* Extract the numbers for the RIGHT plane */
			planeEqs[0][0] = clip[ 3] - clip[ 0];
			planeEqs[0][1] = clip[ 7] - clip[ 4];
			planeEqs[0][2] = clip[11] - clip[ 8];
			planeEqs[0][3] = clip[15] - clip[12];

			/* Normalize the result */
			t = sqrt( planeEqs[0][0] * planeEqs[0][0] + planeEqs[0][1] * planeEqs[0][1] + planeEqs[0][2] * planeEqs[0][2] );
			planeEqs[0][0] /= t;
			planeEqs[0][1] /= t;
			planeEqs[0][2] /= t;
			planeEqs[0][3] /= t;

			/* Extract the numbers for the LEFT plane */
			planeEqs[1][0] = clip[ 3] + clip[ 0];
			planeEqs[1][1] = clip[ 7] + clip[ 4];
			planeEqs[1][2] = clip[11] + clip[ 8];
			planeEqs[1][3] = clip[15] + clip[12];

			/* Normalize the result */
			t = sqrt( planeEqs[1][0] * planeEqs[1][0] + planeEqs[1][1] * planeEqs[1][1] + planeEqs[1][2] * planeEqs[1][2] );
			planeEqs[1][0] /= t;
			planeEqs[1][1] /= t;
			planeEqs[1][2] /= t;
			planeEqs[1][3] /= t;

			/* Extract the BOTTOM plane */
			planeEqs[2][0] = clip[ 3] + clip[ 1];
			planeEqs[2][1] = clip[ 7] + clip[ 5];
			planeEqs[2][2] = clip[11] + clip[ 9];
			planeEqs[2][3] = clip[15] + clip[13];

			/* Normalize the result */
			t = sqrt( planeEqs[2][0] * planeEqs[2][0] + planeEqs[2][1] * planeEqs[2][1] + planeEqs[2][2] * planeEqs[2][2] );
			planeEqs[2][0] /= t;
			planeEqs[2][1] /= t;
			planeEqs[2][2] /= t;
			planeEqs[2][3] /= t;

			/* Extract the TOP plane */
			planeEqs[3][0] = clip[ 3] - clip[ 1];
			planeEqs[3][1] = clip[ 7] - clip[ 5];
			planeEqs[3][2] = clip[11] - clip[ 9];
			planeEqs[3][3] = clip[15] - clip[13];

			/* Normalize the result */
			t = sqrt( planeEqs[3][0] * planeEqs[3][0] + planeEqs[3][1] * planeEqs[3][1] + planeEqs[3][2] * planeEqs[3][2] );
			planeEqs[3][0] /= t;
			planeEqs[3][1] /= t;
			planeEqs[3][2] /= t;
			planeEqs[3][3] /= t;

			/* Extract the FAR plane */
			planeEqs[4][0] = clip[ 3] - clip[ 2];
			planeEqs[4][1] = clip[ 7] - clip[ 6];
			planeEqs[4][2] = clip[11] - clip[10];
			planeEqs[4][3] = clip[15] - clip[14];

			/* Normalize the result */
			t = sqrt( planeEqs[4][0] * planeEqs[4][0] + planeEqs[4][1] * planeEqs[4][1] + planeEqs[4][2] * planeEqs[4][2] );
			planeEqs[4][0] /= t;
			planeEqs[4][1] /= t;
			planeEqs[4][2] /= t;
			planeEqs[4][3] /= t;

			/* Extract the NEAR plane */
			planeEqs[5][0] = clip[ 3] + clip[ 2];
			planeEqs[5][1] = clip[ 7] + clip[ 6];
			planeEqs[5][2] = clip[11] + clip[10];
			planeEqs[5][3] = clip[15] + clip[14];

			/* Normalize the result */
			t = sqrt( planeEqs[5][0] * planeEqs[5][0] + planeEqs[5][1] * planeEqs[5][1] + planeEqs[5][2] * planeEqs[5][2] );
			planeEqs[5][0] /= t;
			planeEqs[5][1] /= t;
			planeEqs[5][2] /= t;
			planeEqs[5][3] /= t;
		}


		bool isPointInside(const Vec3& v) const
		{
			for(int p = 0; p < 6; p++ )
				if( planeEqs[p][0] * v.x + planeEqs[p][1] * v.y + planeEqs[p][2] * v.z + planeEqs[p][3] <= 0 )
					return false;
			return true;
		}

		unsigned isAABBInside(const Vec3 &pos, const Vec3 &size) const
		{
			const Vec3& bm = pos;
			const Vec3& bd = size;
			unsigned result = 1;	//// INNERHALB !!!
			double m, n;

			for (int i = 0; i < 6; i++)
			{
				m = (bm.x * planeEqs[i][0]) + (bm.y * planeEqs[i][1]) + (bm.z * planeEqs[i][2]) + planeEqs[i][3];
				n = (bd.x * fabs(planeEqs[i][0])) + (bd.y * fabs(planeEqs[i][1])) + (bd.z * fabs(planeEqs[i][2]));

				if (m + n < 0) return 0;   // AU�ERHALB !!!
				if (m - n < 0) result = 2; // TEILWEISE !!!
			}

			return result;
		}

		unsigned  isAABBInside(const AABBox& aab) const
		{
			return isAABBInside(aab.getMin(), aab.getSize());
		}

		bool isSphereInside( Float x, Float y, Float z, Float radius ) const
		{
			for(int p = 0; p < 6; p++ )
				if( planeEqs[p][0] * x + planeEqs[p][1] * y + planeEqs[p][2] * z + planeEqs[p][3] <= -radius )
					return false;
			return true;
		}

		bool isCubeInside( Float x, Float y, Float z, Float size ) const
		{
			for(int p = 0; p < 6; p++ )
			{
				if( planeEqs[p][0] * (x - size) + planeEqs[p][1] * (y - size) + planeEqs[p][2] * (z - size) + planeEqs[p][3] > 0 )
					continue;
				if( planeEqs[p][0] * (x + size) + planeEqs[p][1] * (y - size) + planeEqs[p][2] * (z - size) + planeEqs[p][3] > 0 )
					continue;
				if( planeEqs[p][0] * (x - size) + planeEqs[p][1] * (y + size) + planeEqs[p][2] * (z - size) + planeEqs[p][3] > 0 )
					continue;
				if( planeEqs[p][0] * (x + size) + planeEqs[p][1] * (y + size) + planeEqs[p][2] * (z - size) + planeEqs[p][3] > 0 )
					continue;
				if( planeEqs[p][0] * (x - size) + planeEqs[p][1] * (y - size) + planeEqs[p][2] * (z + size) + planeEqs[p][3] > 0 )
					continue;
				if( planeEqs[p][0] * (x + size) + planeEqs[p][1] * (y - size) + planeEqs[p][2] * (z + size) + planeEqs[p][3] > 0 )
					continue;
				if( planeEqs[p][0] * (x - size) + planeEqs[p][1] * (y + size) + planeEqs[p][2] * (z + size) + planeEqs[p][3] > 0 )
					continue;
				if( planeEqs[p][0] * (x + size) + planeEqs[p][1] * (y + size) + planeEqs[p][2] * (z + size) + planeEqs[p][3] > 0 )
					continue;
				return false;
			}
			return true;
		}




		const Plane& getTopPlane() const { return planeEqs[3]; }
		const Plane& getBottomPlane() const { return planeEqs[2]; }
		const Plane& getLeftPlane() const { return planeEqs[1]; }
		const Plane& getRightPlane() const { return planeEqs[0]; }
		const Plane& getBackPlane() const { return planeEqs[4]; }
		const Plane& getFrontPlane() const { return planeEqs[5]; }
	};






	// INLINE //

	inline Matrix Matrix::Inverse(const Matrix &m)
	{
		Matrix out(false);

		Float wtmp[4][8];
		Float m0, m1, m2, m3, s;
		Float *r0, *r1, *r2, *r3;

		r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

		r0[0] = m.data[0];
		r0[1] = m.data[1];
		r0[2] = m.data[2];
		r0[3] = m.data[3];
		r0[4] = 1.0;
		r0[5] = r0[6] = r0[7] = 0.0;
		r1[0] = m.data[4];
		r1[1] = m.data[5];
		r1[2] = m.data[6];
		r1[3] = m.data[7];
		r1[5] = 1.0;
		r1[4] = r1[6] = r1[7] = 0.0;
		r2[0] = m.data[8];
		r2[1] = m.data[9],
		r2[2] = m.data[10];
		r2[3] = m.data[11];
		r2[6] = 1.0;
		r2[4] = r2[5] = r2[7] = 0.0;
		r3[0] = m.data[12];
		r3[1] = m.data[13];
		r3[2] = m.data[14];
		r3[3] = m.data[15];
		r3[7] = 1.0;
		r3[4] = r3[5] = r3[6] = 0.0;

		/* choose pivot - or die */
		if (fabs(r3[0]) > fabs(r2[0]))
			std::swap(r3, r2);
		if (fabs(r2[0]) > fabs(r1[0]))
			std::swap(r2, r1);
		if (fabs(r1[0]) > fabs(r0[0]))
			std::swap(r1, r0);
		if (0.0 == r0[0])
		{
			std::cerr << "math3d warning: Matrix::Inverse failed" << std::endl;
			return m;
		}

		/* eliminate first variable */
		m1 = r1[0] / r0[0];
		m2 = r2[0] / r0[0];
		m3 = r3[0] / r0[0];

		s = r0[1];
		r1[1] -= m1 * s;
		r2[1] -= m2 * s;
		r3[1] -= m3 * s;
		s = r0[2];
		r1[2] -= m1 * s;
		r2[2] -= m2 * s;
		r3[2] -= m3 * s;
		s = r0[3];
		r1[3] -= m1 * s;
		r2[3] -= m2 * s;
		r3[3] -= m3 * s;
		s = r0[4];

		if (s != 0.0)
		{
			r1[4] -= m1 * s;
			r2[4] -= m2 * s;
			r3[4] -= m3 * s;
		}
		s = r0[5];
		if (s != 0.0)
		{
			r1[5] -= m1 * s;
			r2[5] -= m2 * s;
			r3[5] -= m3 * s;
		}
		s = r0[6];
		if (s != 0.0)
		{
			r1[6] -= m1 * s;
			r2[6] -= m2 * s;
			r3[6] -= m3 * s;
		}
		s = r0[7];
		if (s != 0.0)
		{
			r1[7] -= m1 * s;
			r2[7] -= m2 * s;
			r3[7] -= m3 * s;
		}

		/* choose pivot - or die */
		if (fabs(r3[1]) > fabs(r2[1]))
			std::swap(r3, r2);
		if (fabs(r2[1]) > fabs(r1[1]))
			std::swap(r2, r1);
		if (0.0 == r1[1])
		{
			std::cerr << "math3d warning: Matrix::Inverse failed" << std::endl;
			return m;
		}

		/* eliminate second variable */
		m2 = r2[1] / r1[1];
		m3 = r3[1] / r1[1];
		r2[2] -= m2 * r1[2];
		r3[2] -= m3 * r1[2];
		r2[3] -= m2 * r1[3];
		r3[3] -= m3 * r1[3];
		s = r1[4];

		if (0.0 != s)
		{
			r2[4] -= m2 * s;
			r3[4] -= m3 * s;
		}
		s = r1[5];
		if (0.0 != s)
		{
			r2[5] -= m2 * s;
			r3[5] -= m3 * s;
		}
		s = r1[6];
		if (0.0 != s)
		{
			r2[6] -= m2 * s;
			r3[6] -= m3 * s;
		}
		s = r1[7];
		if (0.0 != s)
		{
			r2[7] -= m2 * s;
			r3[7] -= m3 * s;
		}

		/* choose pivot - or die */
		if (fabs(r3[2]) > fabs(r2[2]))
			std::swap(r3, r2);
		if (0.0 == r2[2])
		{
			std::cerr << "math3d warning: Matrix::Inverse failed" << std::endl;
			return m;
		}

		/* eliminate third variable */
		m3 = r3[2] / r2[2];
		r3[3] -= m3 * r2[3];
		r3[4] -= m3 * r2[4];
		r3[5] -= m3 * r2[5];
		r3[6] -= m3 * r2[6];
		r3[7] -= m3 * r2[7];

		/* last check */
		if (0.0 == r3[3])
		{
			std::cerr << "math3d warning: Matrix::Inverse failed" << std::endl;
			return m;
		}

		s = 1.f / r3[3];		/* now back substitute row 3 */
		r3[4] *= s;
		r3[5] *= s;
		r3[6] *= s;
		r3[7] *= s;

		m2 = r2[3];			/* now back substitute row 2 */
		s = 1.f / r2[2];
		r2[4] = s * (r2[4] - r3[4] * m2);
		r2[5] = s * (r2[5] - r3[5] * m2);
		r2[6] = s * (r2[6] - r3[6] * m2);
		r2[7] = s * (r2[7] - r3[7] * m2);
		m1 = r1[3];
		r1[4] -= r3[4] * m1;
		r1[5] -= r3[5] * m1;
		r1[6] -= r3[6] * m1;
		r1[7] -= r3[7] * m1;
		m0 = r0[3];
		r0[4] -= r3[4] * m0;
		r0[5] -= r3[5] * m0;
		r0[6] -= r3[6] * m0;
		r0[7] -= r3[7] * m0;

		m1 = r1[2];			/* now back substitute row 1 */
		s = 1.f / r1[1];
		r1[4] = s * (r1[4] - r2[4] * m1);
		r1[5] = s * (r1[5] - r2[5] * m1);
		r1[6] = s * (r1[6] - r2[6] * m1);
		r1[7] = s * (r1[7] - r2[7] * m1);
		m0 = r0[2];
		r0[4] -= r2[4] * m0;
		r0[5] -= r2[5] * m0;
		r0[6] -= r2[6] * m0;
		r0[7] -= r2[7] * m0;

		m0 = r0[1];			/* now back substitute row 0 */
		s = 1.f / r0[0];
		r0[4] = s * (r0[4] - r1[4] * m0);
		r0[5] = s * (r0[5] - r1[5] * m0);
		r0[6] = s * (r0[6] - r1[6] * m0);
		r0[7] = s * (r0[7] - r1[7] * m0);

		out.data[0] = r0[4];
		out.data[1] = r0[5];
		out.data[2] = r0[6];
		out.data[3] = r0[7];
		out.data[4] = r1[4];
		out.data[5] = r1[5];
		out.data[6] = r1[6];
		out.data[7] = r1[7];
		out.data[8] = r2[4];
		out.data[9] = r2[5];
		out.data[10] = r2[6];
		out.data[11] = r2[7];
		out.data[12] = r3[4];
		out.data[13] = r3[5];
		out.data[14] = r3[6];
		out.data[15] = r3[7];

		return out;
	}


	inline Matrix Matrix::Translation(const Vec3 &t)
	{
		Matrix m(false);

		m[0]  = 1; m[4]  = 0; m[8]  = 0; m[12]  = t.x;
		m[1]  = 0; m[5]  = 1; m[9]  = 0; m[13]  = t.y;
		m[2]  = 0; m[6]  = 0; m[10] = 1; m[14] = t.z;
		m[3]  = 0;  m[7]  = 0; m[11] = 0; m[15] = 1;

		return m;
	}

	inline Matrix Matrix::Scale(const Vec3 &s)
	{
		Matrix m(false);

		m[0] = s.x; m[4] = 0;   m[8]  = 0;   m[12]  = 0;
		m[1] = 0;   m[5] = s.y; m[9]  = 0;   m[13]  = 0;
		m[2] = 0;   m[6] = 0;   m[10] = s.z; m[14] = 0;
		m[3] = 0;   m[7] = 0;   m[11] = 0;   m[15] = 1;

		return m;
	}

	inline Matrix Matrix::Rotation(const Vec3& axis,const Float angle)
	{
		Float a = DEG2RAD(angle);
		const Vec3 &v = axis;

		Matrix m(false);

		Float qqq = (1-cos(a));

		m[0]  = (v.x * v.x * qqq) + cos(a);
		m[4]  = (v.x * v.y * qqq) - (v.z * sin(a));
		m[8]  = (v.x * v.z * qqq) + (v.y * sin(a));
		m[12] = 0;

		m[1]  = (v.y * v.x * qqq) + (v.z * sin(a));
		m[5]  = (v.y * v.y * qqq) + cos(a);
		m[9]  = (v.y * v.z * qqq) - (v.x * sin(a));
		m[13] = 0;

		m[2]  = (v.z * v.x * qqq) - (v.y * sin(a));
		m[6]  = (v.z * v.y * qqq) + (v.x * sin(a));
		m[10] = (v.z * v.z * qqq) + cos(a);
		m[14] = 0;

		m[3]  = 0;
		m[7]  = 0;
		m[11] = 0;
		m[15] = 1;

		return m;
	}

	inline Matrix Matrix::Frustum(Float l, Float r, Float b, Float t, Float n, Float f)
	{
		Matrix m(false);

		m[0] = (2.f*n)/(r-l);	m[1] = 0;		m[2]  = 0;		  m[3] = 0;
		m[4] = 0;		m[5] = (2.f*n)/(t-b);	m[6]  = 0;		  m[7] = 0;
		m[8] = (r+l)/(r-l);	m[9] = (t+b)/(t-b);	m[10] = f/(n-f);	  m[11] = -1;
		m[12] = 0;		m[13] = 0;		m[14] = (n*f)/(n-f);	  m[15] = 0;

		return m;
	}

	inline Matrix Matrix::Ortho(Float l, Float r, Float b, Float t, Float n, Float f)
	{
		Matrix m(false);

		m[0] = 2.f/(r-l);	m[1] = 0;		m[2]  = 0;		  m[3] = 0;
		m[4] = 0;		m[5] = 2.f/(t-b);	m[6]  = 0;		  m[7] = 0;
		m[8] = 0;		m[9] = 0;		m[10] = 1/(n-f);	  m[11] = 0;
		m[12] = (l+r)/(l-r);	m[13] = (t+b)/(b-t);	m[14] = n/(n-f);	  m[15] = 1;

		return m;
	}

	inline Matrix Matrix::Lookat(const Vec3& eye, const Vec3& forward, const Vec3& up)
	{
		Vec3 side = cross(forward,up).normalized();
		Vec3 up2 = cross(side,forward);

		Matrix m(true);

		m[0] = side.x;
		m[4] = side.y;
		m[8] = side.z;

		m[1] = up2.x;
		m[5] = up2.y;
		m[9] = up2.z;

		m[2] = -forward.x;
		m[6] = -forward.y;
		m[10] = -forward.z;

		m = Translation(eye*-1) * m;

		return m;
	}

	inline Matrix Matrix::Shadow(const Plane& groundplane, const Vec3& light)
	{
		const int X = 0;
		const int Y = 1;
		const int Z = 2;
		const int W = 3;

		Float lightpos[4];
		lightpos[0] = light.x;
		lightpos[1] = light.y;
		lightpos[2] = light.z;
		lightpos[3] = 1;

		Float shadowMat[4][4];
		Float dot = groundplane[X] * lightpos[X] + groundplane[Y] * lightpos[Y] + groundplane[Z] * lightpos[Z] + groundplane[W] * lightpos[W];

		shadowMat[0][0] = dot - lightpos[X] * groundplane[X];
		shadowMat[1][0] = 0.f - lightpos[X] * groundplane[Y];
		shadowMat[2][0] = 0.f - lightpos[X] * groundplane[Z];
		shadowMat[3][0] = 0.f - lightpos[X] * groundplane[W];
		shadowMat[0][1] = 0.f - lightpos[Y] * groundplane[X];
		shadowMat[1][1] = dot - lightpos[Y] * groundplane[Y];
		shadowMat[2][1] = 0.f - lightpos[Y] * groundplane[Z];
		shadowMat[3][1] = 0.f - lightpos[Y] * groundplane[W];
		shadowMat[0][2] = 0.f - lightpos[Z] * groundplane[X];
		shadowMat[1][2] = 0.f - lightpos[Z] * groundplane[Y];
		shadowMat[2][2] = dot - lightpos[Z] * groundplane[Z];
		shadowMat[3][2] = 0.f - lightpos[Z] * groundplane[W];
		shadowMat[0][3] = 0.f - lightpos[W] * groundplane[X];
		shadowMat[1][3] = 0.f - lightpos[W] * groundplane[Y];
		shadowMat[2][3] = 0.f - lightpos[W] * groundplane[Z];
		shadowMat[3][3] = dot - lightpos[W] * groundplane[W];

		return reinterpret_cast<Matrix&>(shadowMat);
	}

	inline bool Ray::getIntersectionWithTriangle(const Vec3& vert0, const Vec3& vert1, const Vec3& vert2, Float &t) const
	{
		//Algorithmus aus http://www.graphics.cornell.edu/pubs/1997/MT97.pdf

		Float u;
		Float v;
		Vec3 edge1 = vert1 - vert0;
		Vec3 edge2 = vert2 - vert0;

		Vec3 tvec, pvec, qvec;
		Float det, inv_det;

		pvec = cross(m_direction, edge2);

		det = dot(edge1, pvec);

		if (det > -0.0000001f)
			return false;

		inv_det = 1.0f / det;

		tvec = m_origin - vert0;

		u = dot(tvec, pvec) * inv_det;
		if (u < -0.001f || u > 1.001f)
			return false;

		qvec = cross(tvec, edge1);

		v = dot(m_direction, qvec) * inv_det;
		if (v < -0.001f || u + v > 1.001f)
			return false;

		t = dot(edge2, qvec) * inv_det;

		if (t <= 0)
			return false;

		return true;
	}

	inline bool Ray::getIntersectionWithAABBox(const AABBox& box, Float& t, Float t0/* = 0*/, Float t1/* = 1*/) const
	{
		//Algorithmus aus http://cag.csail.mit.edu/~amy/papers/box-jgt.pdf

		int sign[3];
		Vec3 inv_direction = Vec3(1/m_direction.x, 1/m_direction.y, 1/m_direction.z);
		sign[0] = (inv_direction.x < 0);
		sign[1] = (inv_direction.y < 0);
		sign[2] = (inv_direction.z < 0);

		const Vec3 bounds[] = {box.getMin(), box.getMax()};

		Float tmax, tymin, tymax, tzmin, tzmax;
		Float& tmin = t;

		tmin = (bounds[sign[0]].x - m_origin.x) * inv_direction.x;
		tmax = (bounds[1-sign[0]].x - m_origin.x) * inv_direction.x;
		tymin = (bounds[sign[1]].y - m_origin.y) * inv_direction.y;
		tymax = (bounds[1-sign[1]].y - m_origin.y) * inv_direction.y;

		if ( (tmin > tymax) || (tymin > tmax) )
			return false;
		if (tymin > tmin)
			tmin = tymin;
		if (tymax < tmax)
			tmax = tymax;

		tzmin = (bounds[sign[2]].z - m_origin.z) * inv_direction.z;
		tzmax = (bounds[1-sign[2]].z - m_origin.z) * inv_direction.z;

		if ( (tmin > tzmax) || (tzmin > tmax) )
			return false;
		if (tzmin > tmin)
			tmin = tzmin;
		if (tzmax < tmax)
			tmax = tzmax;

		return ( (tmin < t1) && (tmax > t0) );
	}


}//end namespace

namespace std
{

inline ostream& operator<<(ostream& o, const math3D::Vec3& v)
{
	o << "[" << v.x << " " << v.y << " " << v.z << "]";
	return o;
}

inline ostream& operator<<(ostream& o, const math3D::Matrix& m)
{
	for(int i = 0; i < 4; i++)
	{
		o << "[";
		for(int j = 0; j < 4; j++)
			o << m[i*4+j] << " ";
		o << "]" << std::endl;
	}

	return o;
}

}//end std namespace

