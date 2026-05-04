#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Vector3
{
public:
	float x;
	float y;
	float z;
	Vector3() { x = 0; y = 0; z = 0; };
	Vector3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; };
	Vector3(XMFLOAT3 vector) { x = vector.x; y = vector.y; z = vector.z; }
	Vector3 operator+(const Vector3& other) const
	{
		return Vector3(x + other.x, y + other.y, z + other.z);
	}
	Vector3 operator-(const Vector3& other) const
	{
		return Vector3(x - other.x, y - other.y, z - other.z);
	}
	Vector3 operator*(float scalar) const
	{
		return Vector3(x * scalar, y * scalar, z * scalar);
	}
	Vector3 operator/(float scalar) const
	{
		return Vector3(x / scalar, y / scalar, z / scalar);
	}
	Vector3& operator+=(const Vector3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
	Vector3& operator-=(const Vector3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
	Vector3& operator*=(float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}
	Vector3& operator/=(float scalar)
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}
};

