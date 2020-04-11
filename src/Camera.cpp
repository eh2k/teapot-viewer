// Copyright (c) 2007,2010, Eduard Heidt

#include "Camera.h"

#include <iostream>

namespace eh
{

	Ptr<Camera> Camera::create(const std::string& name, Float width, Float height, Float near, Float _far, const Vec3& pos, const Vec3& dir, const Vec3& up)
	{
		return new Camera(name, width, height, near, _far, pos, dir, up);
	}

	Ptr<Camera> Camera::create(const std::string& name, Float width, Float height, Float near, Float _far, const Matrix& m)
	{
		Vec3 side(m[0],m[4],m[8]);
		Vec3 up(m[1],m[5],m[9]);
		Vec3 dir(-m[2],-m[6],-m[10]);

		Vec3 t(-m[12], -m[13], -m[14]);

		Vec3 pos = transform(t, (m * Matrix::Translation(t)).getInverted());

		return new Camera(name, width, height, near, _far, pos, dir, up);
	}

	Matrix Camera::getLookAtMatrix() const
	{
		return Matrix::Lookat(m_pos, m_dir, m_up);
	}

	Matrix Camera::getRotation() const
	{
		return Matrix::Lookat(Vec3::Null(), m_dir, m_up);
	}
}
