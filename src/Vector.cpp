///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
///////////////////////////////////////////////////////////////////////

#include <wendy/Config.h>
#include <wendy/Core.h>
#include <wendy/Vector.h>

#include <sstream>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Vec2::Vec2(void)
{
}

Vec2::Vec2(float sx, float sy):
  x(sx),
  y(sy)
{
}

Vec2::Vec2(const String& string)
{
  std::istringstream stream(string, std::ios::in);

  stream >> x >> y;
}

float Vec2::length(void) const
{
  return (float) sqrtf(dot(*this));
}

float Vec2::lengthSquared(void) const
{
  return x * x + y * y;
}

float Vec2::dot(const Vec2& other) const
{
  return x * other.x + y * other.y;
}

String Vec2::asString(void) const
{
  std::ostringstream stream;

  stream << x << ' ' << y;
  return stream.str();
}

Vec2 Vec2::interpolateTo(float t, const Vec2& other) const
{
  return Vec2(x * (1.f - t) + other.x * t,
              y * (1.f - t) + other.y * t);
}

Vec2 Vec2::absolute(void) const
{
  return Vec2(fabsf(x), fabsf(y));
}

Vec2& Vec2::scaleTo(float len)
{
  const float scale = len / length();

  x *= scale;
  y *= scale;
  return *this;
}

Vec2& Vec2::negate(void)
{
  x = -x;
  y = -y;
  return *this;
}

Vec2& Vec2::normalize(void)
{
  const float scale = 1.f / length();

  x *= scale;
  y *= scale;
  return *this;
}

Vec2 Vec2::normalized(void) const
{
  const float scale = 1.f / length();

  return Vec2(x * scale, y * scale);
}

Vec2::operator float* (void)
{
  return &(x);
}

Vec2::operator const float* (void) const
{
  return &(x);
}

Vec2 Vec2::operator - (void) const
{
  return Vec2(-x, -y);
}

Vec2 Vec2::operator + (float value) const
{
  return Vec2(x + value, y + value);
}

Vec2 Vec2::operator - (float value) const
{
  return Vec2(x - value, y - value);
}

Vec2 Vec2::operator * (float value) const
{
  return Vec2(x * value, y * value);
}

Vec2 Vec2::operator / (float value) const
{
  return Vec2(x / value, y / value);
}

Vec2 Vec2::operator += (float value)
{
  x += value;
  y += value;
  return *this;
}

Vec2& Vec2::operator -= (float value)
{
  x -= value;
  y -= value;
  return *this;
}

Vec2& Vec2::operator *= (float value)
{
  x *= value;
  y *= value;
  return *this;
}

Vec2& Vec2::operator /= (float value)
{
  x /= value;
  y /= value;
  return *this;
}

Vec2 Vec2::operator + (const Vec2& vector) const
{
  return Vec2(x + vector.x, y + vector.y);
}

Vec2 Vec2::operator - (const Vec2& vector) const
{
  return Vec2(x - vector.x, y - vector.y);
}

Vec2 Vec2::operator * (const Vec2& vector) const
{
  return Vec2(x * vector.x, y * vector.y);
}

Vec2 Vec2::operator / (const Vec2& vector) const
{
  return Vec2(x / vector.x, y / vector.y);
}

Vec2 Vec2::operator += (const Vec2& vector)
{
  x += vector.x;
  y += vector.y;
  return *this;
}

Vec2& Vec2::operator -= (const Vec2& vector)
{
  x -= vector.x;
  y -= vector.y;
  return *this;
}

Vec2& Vec2::operator *= (const Vec2& vector)
{
  x *= vector.x;
  y *= vector.y;
  return *this;
}

Vec2& Vec2::operator /= (const Vec2& vector)
{
  x /= vector.x;
  y /= vector.y;
  return *this;
}

bool Vec2::operator == (const Vec2& vector) const
{
  return x == vector.x && y == vector.y;
}

bool Vec2::operator != (const Vec2& vector) const
{
  return x != vector.x || y != vector.y;
}

void Vec2::setDefaults(void)
{
  x = 0.f;
  y = 0.f;
}

void Vec2::set(float sx, float sy)
{
  x = sx;
  y = sy;
}

const Vec2 Vec2::ZERO(0.f, 0.f);
const Vec2 Vec2::ONE(1.f, 1.f);
const Vec2 Vec2::X(1.f, 0.f);
const Vec2 Vec2::Y(0.f, 1.f);

///////////////////////////////////////////////////////////////////////

Vec2i::Vec2i(void)
{
}

Vec2i::Vec2i(int initX, int initY):
  x(initX),
  y(initY)
{
}

Vec2i Vec2i::operator - (void) const
{
  return Vec2i(-x, -y);
}

Vec2i Vec2i::operator + (int value) const
{
  return Vec2i(x + value, y + value);
}

Vec2i Vec2i::operator - (int value) const
{
  return Vec2i(x - value, y - value);
}

Vec2i Vec2i::operator * (int value) const
{
  return Vec2i(x * value, y * value);
}

Vec2i Vec2i::operator / (int value) const
{
  return Vec2i(x / value, y / value);
}

Vec2i Vec2i::operator += (int value)
{
  x += value;
  y += value;
  return *this;
}

Vec2i& Vec2i::operator -= (int value)
{
  x -= value;
  y -= value;
  return *this;
}

Vec2i& Vec2i::operator *= (int value)
{
  x *= value;
  y *= value;
  return *this;
}

Vec2i& Vec2i::operator /= (int value)
{
  x /= value;
  y /= value;
  return *this;
}

Vec2i Vec2i::operator + (const Vec2i& other) const
{
  return Vec2i(x + other.x, y + other.y);
}

Vec2i Vec2i::operator - (const Vec2i& other) const
{
  return Vec2i(x - other.x, y - other.y);
}

Vec2i Vec2i::operator * (const Vec2i& other) const
{
  return Vec2i(x * other.x, y * other.y);
}

Vec2i Vec2i::operator / (const Vec2i& other) const
{
  return Vec2i(x / other.x, y / other.y);
}

Vec2i& Vec2i::operator += (const Vec2i& other)
{
  x += other.x;
  y += other.y;
  return *this;
}

Vec2i& Vec2i::operator -= (const Vec2i& other)
{
  x -= other.x;
  y -= other.y;
  return *this;
}

Vec2i& Vec2i::operator *= (const Vec2i& other)
{
  x *= other.x;
  y *= other.y;
  return *this;
}

Vec2i& Vec2i::operator /= (const Vec2i& other)
{
  x /= other.x;
  y /= other.y;
  return *this;
}

bool Vec2i::operator == (const Vec2i& other) const
{
  return x == other.x && y == other.y;
}

bool Vec2i::operator != (const Vec2i& other) const
{
  return x != other.x || y != other.y;
}

void Vec2i::setDefaults(void)
{
  x = y = 0;
}

void Vec2i::set(int newX, int newY)
{
  x = newX;
  y = newY;
}

///////////////////////////////////////////////////////////////////////

Vec3::Vec3(void)
{
}

Vec3::Vec3(float sx, float sy, float sz):
  x(sx),
  y(sy),
  z(sz)
{
}

Vec3::Vec3(const String& string)
{
  std::istringstream stream(string, std::ios::in);

  stream >> x >> y >> z;
}

float Vec3::length(void) const
{
  return sqrtf(x * x + y * y + z * z);
}

float Vec3::lengthSquared(void) const
{
  return x * x + y * y + z * z;
}

float Vec3::dot(const Vec3& vector) const
{
  return x * vector.x + y * vector.y + z * vector.z;
}

String Vec3::asString(void) const
{
  std::ostringstream stream;

  stream << x << ' ' << y << ' ' << z;
  return stream.str();
}

Vec3 Vec3::cross(const Vec3& vector) const
{
  return Vec3(vector.z * y - vector.y * z,
              vector.x * z - vector.z * x,
	      vector.y * x - vector.x * y);
}

Vec3 Vec3::interpolateTo(float t, const Vec3& other) const
{
  return Vec3(x * (1.f - t) + other.x * t,
              y * (1.f - t) + other.y * t,
              z * (1.f - t) + other.z * t);
}

Vec3 Vec3::absolute(void) const
{
  return Vec3(fabsf(x), fabsf(y), fabsf(z));
}

Vec3& Vec3::scaleTo(float len)
{
  const float scale = len / length();

  x *= scale;
  y *= scale;
  z *= scale;
  return *this;
}

Vec3& Vec3::mirrorBy(const Vec3& vector)
{
  const float scale = 2.f * (x * vector.x + y * vector.y + z * vector.z);

  x = vector.x * scale - x;
  y = vector.y * scale - y;
  z = vector.z * scale - z;
  return *this;
}

Vec3& Vec3::negate(void)
{
  x = -x;
  y = -y;
  z = -z;
  return *this;
}

Vec3& Vec3::normalize(void)
{
  const float scale = 1.f / length();

  x *= scale;
  y *= scale;
  z *= scale;
  return *this;
}

Vec3 Vec3::normalized(void) const
{
  const float scale = 1.f / length();

  return Vec3(x * scale, y * scale, z * scale);
}

Vec3::operator float* (void)
{
  return &(x);
}

Vec3::operator const float* (void) const
{
  return &(x);
}

Vec3 Vec3::operator - (void) const
{
  return Vec3(-x, -y, -z);
}

Vec3 Vec3::operator + (float value) const
{
  return Vec3(x + value, y + value, z + value);
}

Vec3 Vec3::operator - (float value) const
{
  return Vec3(x - value, y - value, z - value);
}

Vec3 Vec3::operator * (float value) const
{
  return Vec3(x * value, y * value, z * value);
}

Vec3 Vec3::operator / (float value) const
{
  return Vec3(x / value, y / value, z / value);
}

Vec3 Vec3::operator += (float value)
{
  x += value;
  y += value;
  z += value;
  return *this;
}

Vec3& Vec3::operator -= (float value)
{
  x -= value;
  y -= value;
  z -= value;
  return *this;
}

Vec3& Vec3::operator *= (float value)
{
  x *= value;
  y *= value;
  z *= value;
  return *this;
}

Vec3& Vec3::operator /= (float value)
{
  x /= value;
  y /= value;
  z /= value;
  return *this;
}

Vec3 Vec3::operator + (const Vec3& vector) const
{
  return Vec3(x + vector.x, y + vector.y, z + vector.z);
}

Vec3 Vec3::operator - (const Vec3& vector) const
{
  return Vec3(x - vector.x, y - vector.y, z - vector.z);
}

Vec3 Vec3::operator * (const Vec3& vector) const
{
  return Vec3(x * vector.x, y * vector.y, z * vector.z);
}

Vec3 Vec3::operator / (const Vec3& vector) const
{
  return Vec3(x / vector.x, y / vector.y, z / vector.z);
}

Vec3 Vec3::operator += (const Vec3& vector)
{
  x += vector.x;
  y += vector.y;
  z += vector.z;
  return *this;
}

Vec3& Vec3::operator -= (const Vec3& vector)
{
  x -= vector.x;
  y -= vector.y;
  z -= vector.z;
  return *this;
}

Vec3& Vec3::operator *= (const Vec3& vector)
{
  x *= vector.x;
  y *= vector.y;
  z *= vector.z;
  return *this;
}

Vec3& Vec3::operator /= (const Vec3& vector)
{
  x /= vector.x;
  y /= vector.y;
  z /= vector.z;
  return *this;
}

bool Vec3::operator == (const Vec3& vector) const
{
  return x == vector.x && y == vector.y && z == vector.z;
}

bool Vec3::operator != (const Vec3& vector) const
{
  return x != vector.x || y != vector.y || z != vector.z;
}

void Vec3::setDefaults(void)
{
  x = 0.f;
  y = 0.f;
  z = 0.f;
}

void Vec3::set(float sx, float sy, float sz)
{
  x = sx;
  y = sy;
  z = sz;
}

const Vec3 Vec3::ZERO(0.f, 0.f, 0.f);
const Vec3 Vec3::ONE(1.f, 1.f, 1.f);
const Vec3 Vec3::X(1.f, 0.f, 0.f);
const Vec3 Vec3::Y(0.f, 1.f, 0.f);
const Vec3 Vec3::Z(0.f, 0.f, 1.f);

///////////////////////////////////////////////////////////////////////

Vec4::Vec4(void)
{
}

Vec4::Vec4(float sx, float sy, float sz, float sw):
  x(sx),
  y(sy),
  z(sz),
  w(sw)
{
}

Vec4::Vec4(const Vec3& xyz, float sw):
  x(xyz.x),
  y(xyz.y),
  z(xyz.z),
  w(sw)
{
}

Vec4::Vec4(const String& string)
{
  std::istringstream stream(string, std::ios::in);

  stream >> x >> y >> z >> w;
}

float Vec4::length(void) const
{
  return sqrtf(x * x + y * y + z * z + w * w);
}

float Vec4::lengthSquared(void) const
{
  return x * x + y * y + z * z + w * w;
}

float Vec4::dot(const Vec4& vector) const
{
  return x * vector.x + y * vector.y + z * vector.z + w * vector.w;
}

String Vec4::asString(void) const
{
  std::ostringstream stream;

  stream << x << ' ' << y << ' ' << z << ' ' << w;
  return stream.str();
}

Vec4 Vec4::interpolateTo(float t, const Vec4& other) const
{
  return Vec4(x * (1.f - t) + other.x * t,
              y * (1.f - t) + other.y * t,
              z * (1.f - t) + other.z * t,
              w * (1.f - t) + other.w * t);
}

Vec4 Vec4::absolute(void) const
{
  return Vec4(fabsf(x), fabsf(y), fabsf(z), fabsf(w));
}

Vec4& Vec4::scaleTo(float len)
{
  const float scale = len / length();

  x *= scale;
  y *= scale;
  z *= scale;
  w *= scale;
  return *this;
}

Vec4& Vec4::normalize(void)
{
  const float scale = 1.f / length();

  x *= scale;
  y *= scale;
  z *= scale;
  w *= scale;
  return *this;
}

Vec4 Vec4::normalized(void) const
{
  const float scale = 1.f / length();

  return Vec4(x * scale, y * scale, z * scale, w * scale);
}

Vec4::operator float* (void)
{
  return &(x);
}

Vec4::operator const float* (void) const
{
  return &(x);
}

Vec4 Vec4::operator - (void) const
{
  return Vec4(-x, -y, -z, -w);
}

Vec4 Vec4::operator + (float value) const
{
  return Vec4(x + value, y + value, z + value, w + value);
}

Vec4 Vec4::operator - (float value) const
{
  return Vec4(x - value, y - value, z - value, w - value);
}

Vec4 Vec4::operator * (float value) const
{
  return Vec4(x * value, y * value, z * value, w * value);
}

Vec4 Vec4::operator / (float value) const
{
  value = 1.f / value;

  return Vec4(x * value, y * value, z * value, w * value);
}

Vec4 Vec4::operator += (float value)
{
  x += value;
  y += value;
  z += value;
  w += value;
  return *this;
}

Vec4& Vec4::operator -= (float value)
{
  x -= value;
  y -= value;
  z -= value;
  w -= value;
  return *this;
}

Vec4& Vec4::operator *= (float value)
{
  x *= value;
  y *= value;
  z *= value;
  w *= value;
  return *this;
}

Vec4& Vec4::operator /= (float value)
{
  const float scale = 1.f / value;

  x *= scale;
  y *= scale;
  z *= scale;
  w *= scale;
  return *this;
}

Vec4 Vec4::operator + (const Vec4& vector) const
{
  return Vec4(x + vector.x, y + vector.y, z + vector.z, w + vector.w);
}

Vec4 Vec4::operator - (const Vec4& vector) const
{
  return Vec4(x - vector.x, y - vector.y, z - vector.z, w - vector.w);
}

Vec4 Vec4::operator * (const Vec4& vector) const
{
  return Vec4(x * vector.x, y * vector.y, z * vector.z, w * vector.w);
}

Vec4 Vec4::operator / (const Vec4& vector) const
{
  return Vec4(x / vector.x, y / vector.y, z / vector.z, w / vector.w);
}

Vec4 Vec4::operator += (const Vec4& vector)
{
  x += vector.x;
  y += vector.y;
  z += vector.z;
  w += vector.w;
  return *this;
}

Vec4& Vec4::operator -= (const Vec4& vector)
{
  x -= vector.x;
  y -= vector.y;
  z -= vector.z;
  w -= vector.w;
  return *this;
}

Vec4& Vec4::operator *= (const Vec4& vector)
{
  x *= vector.x;
  y *= vector.y;
  z *= vector.z;
  w *= vector.w;
  return *this;
}

Vec4& Vec4::operator /= (const Vec4& vector)
{
  x /= vector.x;
  y /= vector.y;
  z /= vector.z;
  w /= vector.w;
  return *this;
}

bool Vec4::operator == (const Vec4& vector) const
{
  return x == vector.x && y == vector.y && z == vector.z && w == vector.w;
}

bool Vec4::operator != (const Vec4& vector) const
{
  return x != vector.x || y != vector.y || z != vector.z || w != vector.w;
}

void Vec4::setDefaults(void)
{
  x = 0.f;
  y = 0.f;
  z = 0.f;
  w = 0.f;
}

void Vec4::set(float sx, float sy, float sz, float sw)
{
  x = sx;
  y = sy;
  z = sz;
  w = sw;
}

void Vec4::set(const Vec3& xyz, float sw)
{
  x = xyz.x;
  y = xyz.y;
  z = xyz.z;
  w = sw;
}

const Vec4 Vec4::ZERO(0.f, 0.f, 0.f, 0.f);
const Vec4 Vec4::ONE(1.f, 1.f, 1.f, 1.f);
const Vec4 Vec4::X(1.f, 0.f, 0.f, 0.f);
const Vec4 Vec4::Y(0.f, 1.f, 0.f, 0.f);
const Vec4 Vec4::Z(0.f, 0.f, 1.f, 0.f);
const Vec4 Vec4::W(0.f, 0.f, 0.f, 1.f);

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
