// Copyright (c) 2007,2010, Eduard Heidt

#pragma once

#include "config.h"
#include "RefCounted.h"
#include "IVisitor.h"
#include <vector>
#include <list>

namespace eh
{
	class API_3D SceneNode : public RefCounted
	{
	public:
		virtual ~SceneNode() {};

		typedef int FLAGS;

		static const FLAGS FLAG_UNVISIBLE = 0x00000001;
		static const FLAGS FLAG_SELECTED = 0x00000002;
		static const FLAGS FLAG_HIGHLIGHTED = 0x00000004;

		FLAGS getFlags() const
		{
			return m_flags;
		}
		FLAGS& Flags()
		{
			return m_flags;
		}

		virtual void accept(IVisitor &visitor) = 0;
		virtual const AABBox& getBounding() const
		{
			return m_BoundingBox;
		};

	protected:
		AABBox m_BoundingBox;
		FLAGS m_flags;

		SceneNode() :m_flags(0) {}
	};

	class SceneNodeVector : public std::vector< Ptr<SceneNode> >
	{
	public:
		SceneNodeVector()
		{
		}
		template<class U>
		SceneNodeVector(Ptr<U> const & pNode)
		{
			if (pNode)
				push_back(pNode);
		}
		SceneNodeVector(const SceneNodeVector& v) : std::vector< Ptr<SceneNode> >(v.begin(), v.end())
		{
		}
	};

	typedef std::list< Ptr<SceneNode> > SceneNodeList;


}// end namespace
