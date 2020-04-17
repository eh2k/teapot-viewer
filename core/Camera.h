// Copyright (c) 2007,2010, Eduard Heidt

#pragma once

#include "config.h"
#include "RefCounted.h"
#include <string>

namespace eh{

class API_3D Camera : public RefCounted
{
public:
	static Ptr<Camera> create(const std::string& name, Float width, Float height, Float near, Float _far, const Vec3& pos, const Vec3& dir, const Vec3& up);
	static Ptr<Camera> create(const std::string& name, Float width, Float height, Float near, Float _far, const Matrix& transform);

	const std::string& getName() const
	{
		return m_name;
	}

	Matrix getLookAtMatrix() const;

	Matrix getRotation() const;
	const Vec3& getPosition() const { return m_pos; }

	const Vec3& getDirection() const { return m_dir; }
	const Vec3& getUpVector() const { return m_up; }
private:
	friend class Viewport;
	friend class Controller;

	std::string m_name;

	Float m_width;
	Float m_height;

	Float m_far;
	Float m_near;
	Vec3  m_pos;
	Vec3  m_dir;
	Vec3  m_up;

	Camera(const std::string& name, Float width, Float height, Float near, Float _far, const Vec3& pos, const Vec3& dir, const Vec3& up):
		m_name(name),
		m_width(width),
		m_height(height),
		m_far(_far),
		m_near(near),
		m_pos(pos),
		m_dir(dir),
		m_up(up)
	{}
};


} //end namespace
