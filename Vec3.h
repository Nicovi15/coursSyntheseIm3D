#pragma once
#ifndef _VEC_
#define _VEC_
#include <iostream>


class Vec3
{

public:
	float x;
	float y;
	float z;

	Vec3 operator*(const float f) const
	{
		return Vec3{ x * f, y * f, z * f };
	}

	Vec3 operator/(const float f) const
	{
		return Vec3{ x / f, y/f, z / f };
	}

	Vec3 operator*(const Vec3 v) const
	{
		return Vec3{ x * v.x, y * v.y, z * v.z };
	}

	Vec3 operator+(const Vec3 v) const
	{
		return Vec3{ x + v.x, y + v.y, z + v.z };
	}

	Vec3 operator-(const Vec3 v) const
	{
		return Vec3{ x - v.x, y - v.y, z - v.z };
	}

	float normSquared() const
	{
		return x * x + y * y + z * z;
	}

	Vec3 unitVector() const
	{
		const float norm = std::sqrt(normSquared());
		return Vec3{ x / norm, y / norm, z / norm };
	}

	Vec3 norm() {
		return *this = *this * (1 / sqrt(x * x + y * y + z * z));
	}

	float dot(const Vec3& b) const 
	{ 
		return x * b.x + y * b.y + z * b.z; 
	}

	Vec3 cross(const Vec3& b) const 
	{
		Vec3 res;
		res.x = y * b.z - z * b.y;
		res.y = z * b.x - x * b.z;
		res.z = x * b.y - y * b.x;
		return res;
	}

};

inline Vec3 operator*(const float f, const Vec3 v)
{
	return Vec3{ f * v.x, f * v.y, f * v.z };
}

#endif;


