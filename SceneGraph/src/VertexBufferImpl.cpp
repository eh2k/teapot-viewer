#include "VertexBuffer.h"
#include <unordered_map>
#include <boost/tuple/tuple.hpp>

#include <stdio.h>

namespace eh{

template< class T >
class VertexBufferImpl: public IVertexBuffer
{
	struct Vertex : public T
	{
		Vertex(const T& t):T(t)
		{}
		inline bool operator == (const Vertex& other) const
		{
			for(unsigned i = 0; i < sizeof(Vertex)/sizeof(Float); i++)
			{
				Float fa = *(((Float*)this)+i);
				Float fb = *(((Float*)&other)+i);
				if(fequal(fa,fb) == false)
					return false;
			}
			return true;
		}
		operator size_t() const
		{
			size_t seed = 0;
			//boost::hash_combine( seed, this );
			for(unsigned i = 0; i < sizeof(Vertex)/sizeof(Float); i++)
			{
				Float f = *(((Float*)this)+i);
				boost::hash_combine(seed, (int)(f*100) );
			}
			return seed;
		}
	};

	typedef std::unordered_map< Vertex, Uint > map;
	typedef std::vector< Vertex > vec;
	map m_idx;
	vec m_Buff;
public:
	VertexBufferImpl(const void* pBuffer = NULL, Uint nCount = 0):
		m_idx(64*1024/sizeof(Vertex))
	{
		m_Buff.insert(m_Buff.end(), ((Vertex*)pBuffer), &((Vertex*)pBuffer)[nCount]);
	}
	virtual ~VertexBufferImpl()
	{
	}

	virtual Uint addVertex(const Vec3& v, const Vec3& n = Vec3(), const Vec3& t = Vec3())
	{
		m_resource = NULL;

		//printf("%f %f %f - %f %f %f - %f % f\n", v.x, v.y, v.z, n.x, n.y, n.z, t.x, t.y);

		Vec3 vnt[] = { v, n, t };
		const Vertex& vertex = reinterpret_cast<Vertex&>(vnt[0]);
#if defined(_MSC_VER)
		map::const_iterator it = m_idx.find( vertex );
		if(it != m_idx.end())
			return it->second;
#else
		if(m_idx.find( vertex ) != m_idx.end())
			return m_idx[vertex];
#endif

		m_Buff.push_back(vertex);
		m_idx[ m_Buff.back() ] = (Uint) m_Buff.size()-1;
		return (Uint) m_Buff.size()-1;
	}

	virtual Uint pushVertex(const Vec3& v, const Vec3& n = Vec3(), const Vec3& t = Vec3())
	{
		m_resource = NULL;

		Vec3 vnt[] = { v, n, t };
		const Vertex& vertex = reinterpret_cast<Vertex&>(vnt[0]);

		m_Buff.push_back(vertex);
		m_idx[ m_Buff.back() ] = (unsigned int) m_Buff.size()-1;
		return (unsigned int) m_Buff.size()-1;
	}

	virtual const Vec3& getCoord(Uint i) const
	{
		if(i < m_Buff.size())
			return reinterpret_cast<const Vec3&>(m_Buff[i]);
		return
			Vec3::Null();
	}
	virtual const Vec3& getNormal(Uint i) const
	{
		if(i < (Uint)m_Buff.size() && sizeof(Vertex) > sizeof(Vec3) )
		{
			const Vec3* p = reinterpret_cast<const Vec3*>(&m_Buff[i]);
			++p;
			return *p;
		}
		return
			Vec3::Null();
	}
	virtual const Vec3& getTexCoord(Uint i) const
	{
		if(i < (Uint)m_Buff.size() && sizeof(Vertex) > sizeof(Vec3)*2)
		{
			const Vec3* p = reinterpret_cast<const Vec3*>(&m_Buff[i]);
			++p;
			++p;
			return *p;
		}
		return
			Vec3::Null();
	}
	virtual Uint getVertexCount() const
	{
		return (Uint)m_Buff.size();
	}

	virtual const void* getBuffer(Uint offset) const
	{
		return &m_Buff[0];
	}
	virtual Uint getStride() const
	{
		return sizeof(Vertex);
	}
	virtual Uint getBufferSize() const
	{
		return (Uint) m_Buff.size() * sizeof(Vertex);
	}
};

Ptr<IVertexBuffer> CreateVertexBuffer(Uint nStride, const void* pBuffer/* = NULL*/, Uint nCount/* = 0*/)
{
	if(nStride == sizeof(Vec3))
		return new VertexBufferImpl<Vec3>(pBuffer, nCount);
	else if(nStride == (sizeof(Vec3) + sizeof(Vec3)))
		return new VertexBufferImpl< boost::tuple<Vec3,Vec3> >(pBuffer, nCount);
	else if(nStride == (sizeof(Vec3) + sizeof(Vec3) + sizeof(Float)*2))
		return new VertexBufferImpl< boost::tuple<Vec3,Vec3,Float, Float> >(pBuffer, nCount);
	else
		return NULL;

}

} //end eh namespace
