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
#include <wendy/Quaternion.h>
#include <wendy/AABB.h>
#include <wendy/Matrix.h>

#include <sstream>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Mat2::Mat2(void)
{
  setIdentity();
}

Mat2::Mat2(const Vec2& sx, const Vec2& sy):
  x(sx),
  y(sy)
{
}

Mat2::Mat2(const String& string)
{
  std::istringstream stream(string);

  stream >> x.x >> x.y;
  stream >> y.x >> y.y;
}

bool Mat2::invert(void)
{
  const float det = determinant();
  if (det == 0.f)
    return false;

  const float invDet = 1.f / det;

  Mat2 temp;
  temp.x.x = y.y * invDet;
  temp.x.y = -x.y * invDet;
  temp.y.x = -y.x * invDet;
  temp.y.y = x.x * invDet;

  operator = (temp);
  return true;
}

void Mat2::transpose(void)
{
  float temp;

  temp = x.y; x.y = y.x; y.x = temp;
}

void Mat2::transformVector(Vec2& vector) const
{
  Vec2 temp;

  temp.x = vector.x * x.x + vector.y * y.x;
  temp.y = vector.x * x.y + vector.y * y.y;

  vector = temp;
}

float Mat2::determinant(void) const
{
  return x.x * y.y - x.y * y.x;
}

Mat2::operator float* (void)
{
  return &(x.x);
}

Mat2::operator const float* (void) const
{
  return &(x.x);
}

float Mat2::operator () (unsigned int row, unsigned int column) const
{
  return (&(x.x))[column * 2 + row];
}

float& Mat2::operator () (unsigned int row, unsigned int column)
{
  return (&(x.x))[column * 2 + row];
}

Mat2 Mat2::operator + (const Mat2& matrix) const
{
  return Mat2(x + matrix.x, y + matrix.y);
}

Mat2 Mat2::operator - (const Mat2& matrix) const
{
  return Mat2(x - matrix.x, y - matrix.y);
}

Mat2 Mat2::operator * (const Mat2& matrix) const
{
  Mat2 result;

  result.x.x = x.x * matrix.x.x + y.x * matrix.x.y;
  result.y.x = x.x * matrix.y.x + y.x * matrix.y.y;

  result.x.y = x.y * matrix.x.x + y.y * matrix.x.y;
  result.y.y = x.y * matrix.y.x + y.y * matrix.y.y;

  return result;
}

Vec2 Mat2::operator * (const Vec2& vector) const
{
  Vec2 result = vector;
  transformVector(result);
  return result;
}

Mat2& Mat2::operator += (const Mat2& matrix)
{
  x += matrix.x;
  y += matrix.y;

  return *this;
}

Mat2& Mat2::operator -= (const Mat2& matrix)
{
  x -= matrix.x;
  y -= matrix.y;

  return *this;
}

Mat2& Mat2::operator *= (const Mat2& matrix)
{
  Mat2 result;

  result.x.x = x.x * matrix.x.x + y.x * matrix.x.y;
  result.y.x = x.x * matrix.y.x + y.x * matrix.y.y;

  result.x.y = x.y * matrix.x.x + y.y * matrix.x.y;
  result.y.y = x.y * matrix.y.x + y.y * matrix.y.y;

  return operator = (result);
}

String Mat2::asString(void) const
{
  std::ostringstream stream;

  stream << x.x << ' ' << x.y << ' '
         << y.x << ' ' << y.y;

  return stream.str();
}

void Mat2::setIdentity(void)
{
  x.set(1.f, 0.f);
  y.set(0.f, 1.f);
}

void Mat2::set(const Vec2& sx, const Vec2& sy)
{
  x = sx;
  y = sy;
}

void Mat2::setEulerRotation(const float angle)
{
  const float sina = sinf(angle);
  const float cosa = cosf(angle);

  x.x = cosa;
  x.y = sina;
  y.x = -sina;
  y.y = cosa;
}

///////////////////////////////////////////////////////////////////////

Mat3::Mat3(void)
{
  setIdentity();
}

Mat3::Mat3(const Vec3& sx, const Vec3& sy, const Vec3& sz):
  x(sx),
  y(sy),
  z(sz)
{
}

Mat3::Mat3(const String& string)
{
  std::istringstream stream(string);

  stream >> x.x >> x.y >> x.z;
  stream >> y.x >> y.y >> y.z;
  stream >> z.x >> z.y >> z.z;
}

void Mat3::transpose(void)
{
  float temp;

  temp = x.y; x.y = y.x; y.x = temp;
  temp = x.z; x.z = z.x; z.x = temp;
  temp = y.z; y.z = z.y; z.y = temp;
}

void Mat3::transformVector(Vec3& vector) const
{
  Vec3 temp;

  temp.x = vector.x * x.x + vector.y * y.x + vector.z * z.x;
  temp.y = vector.x * x.y + vector.y * y.y + vector.z * z.y;
  temp.z = vector.x * x.z + vector.y * y.z + vector.z * z.z;

  vector = temp;
}

float Mat3::determinant(void) const
{
  return x.x * (y.y * z.z - z.y * y.z) -
         y.x * (x.y * z.z - z.y * x.z) +
         z.x * (x.y * y.z - y.y * x.z);
}

Mat3::operator float* (void)
{
  return &(x.x);
}

Mat3::operator const float* (void) const
{
  return &(x.x);
}

float Mat3::operator () (unsigned int row, unsigned int column) const
{
  return (&(x.x))[column * 3 + row];
}

float& Mat3::operator () (unsigned int row, unsigned int column)
{
  return (&(x.x))[column * 3 + row];
}

Mat3 Mat3::operator + (const Mat3& matrix) const
{
  return Mat3(x + matrix.x, y + matrix.y, z + matrix.z);
}

Mat3 Mat3::operator - (const Mat3& matrix) const
{
  return Mat3(x - matrix.x, y - matrix.y, z - matrix.z);
}

Mat3 Mat3::operator * (const Mat3& matrix) const
{
  Mat3 result;

  result.x.x = x.x * matrix.x.x + y.x * matrix.x.y + z.x * matrix.x.z;
  result.y.x = x.x * matrix.y.x + y.x * matrix.y.y + z.x * matrix.y.z;
  result.z.x = x.x * matrix.z.x + y.x * matrix.z.y + z.x * matrix.z.z;

  result.x.y = x.y * matrix.x.x + y.y * matrix.x.y + z.y * matrix.x.z;
  result.y.y = x.y * matrix.y.x + y.y * matrix.y.y + z.y * matrix.y.z;
  result.z.y = x.y * matrix.z.x + y.y * matrix.z.y + z.y * matrix.z.z;

  result.x.z = x.z * matrix.x.x + y.z * matrix.x.y + z.z * matrix.x.z;
  result.y.z = x.z * matrix.y.x + y.z * matrix.y.y + z.z * matrix.y.z;
  result.z.z = x.z * matrix.z.x + y.z * matrix.z.y + z.z * matrix.z.z;

  return result;
}

Vec3 Mat3::operator * (const Vec3& vector) const
{
  Vec3 result = vector;
  transformVector(result);
  return result;
}

Mat3& Mat3::operator += (const Mat3& matrix)
{
  x += matrix.x;
  y += matrix.y;
  z += matrix.z;

  return *this;
}

Mat3& Mat3::operator -= (const Mat3& matrix)
{
  x -= matrix.x;
  y -= matrix.y;
  z -= matrix.z;

  return *this;
}

Mat3& Mat3::operator *= (const Mat3& matrix)
{
  Mat3 result;

  result.x.x = x.x * matrix.x.x + y.x * matrix.x.y + z.x * matrix.x.z;
  result.y.x = x.x * matrix.y.x + y.x * matrix.y.y + z.x * matrix.y.z;
  result.z.x = x.x * matrix.z.x + y.x * matrix.z.y + z.x * matrix.z.z;

  result.x.y = x.y * matrix.x.x + y.y * matrix.x.y + z.y * matrix.x.z;
  result.y.y = x.y * matrix.y.x + y.y * matrix.y.y + z.y * matrix.y.z;
  result.z.y = x.y * matrix.z.x + y.y * matrix.z.y + z.y * matrix.z.z;

  result.x.z = x.z * matrix.x.x + y.z * matrix.x.y + z.z * matrix.x.z;
  result.y.z = x.z * matrix.y.x + y.z * matrix.y.y + z.z * matrix.y.z;
  result.z.z = x.z * matrix.z.x + y.z * matrix.z.y + z.z * matrix.z.z;

  return operator = (result);
}

String Mat3::asString(void) const
{
  std::ostringstream stream;

  stream << x.x << ' ' << x.y << ' ' << x.z << ' '
         << y.x << ' ' << y.y << ' ' << y.z << ' '
         << z.x << ' ' << z.y << ' ' << z.z;

  return stream.str();
}

void Mat3::setIdentity(void)
{
  x.set(1.f, 0.f, 0.f);
  y.set(0.f, 1.f, 0.f);
  z.set(0.f, 0.f, 1.f);
}

void Mat3::set(const Vec3& sx, const Vec3& sy, const Vec3& sz)
{
  x = sx;
  y = sy;
  z = sz;
}

void Mat3::setQuatRotation(const Quat& quat)
{
  x.x = 1.f - 2.f * quat.y * quat.y - 2.f * quat.z * quat.z;
  x.y = 2.f * quat.x * quat.y + 2.f * quat.w * quat.z;
  x.z = 2.f * quat.x * quat.z - 2.f * quat.w * quat.y;

  y.x = 2.f * quat.x * quat.y - 2.f * quat.w * quat.z;
  y.y = 1.f - 2.f * quat.x * quat.x - 2.f * quat.z * quat.z;
  y.z = 2.f * quat.y * quat.z + 2.f * quat.w * quat.x;

  z.x = 2.f * quat.x * quat.z + 2.f * quat.w * quat.y;
  z.y = 2.f * quat.y * quat.z - 2.f * quat.w * quat.x;
  z.z = 1.f - 2.f * quat.x * quat.x - 2.f * quat.y * quat.y;
}

void Mat3::setVectorRotation(const Vec3& vector)
{
  // NOTE: This code sucks.

  Vec3 origin(0.f, 0.f, 1.f);

  const float dot = origin.dot(vector);

  if (fabsf(dot) < 0.9999f)
  {
    Vec3 axis = origin.cross(vector).normalized();
    setAxisRotation(axis, acosf(vector.dot(origin)));
  }
  else if (dot < 0.f)
  {
    const Vec3 axis(1.f, 0.f, 0.f);
    setAxisRotation(axis, (float) M_PI);
  }
  else
    setIdentity();
}

void Mat3::setAxisRotation(const Vec3& axis, float angle)
{
  const float cosAngle = cosf(angle);
  const float sinAngle = sinf(angle);

  const float oneMinusCos = 1.f - cosAngle;

  const float x2 = axis.x * axis.x;
  const float y2 = axis.y * axis.y;
  const float z2 = axis.z * axis.z;
  const float xy = axis.x * axis.y * oneMinusCos;
  const float xz = axis.x * axis.z * oneMinusCos;
  const float yz = axis.y * axis.z * oneMinusCos;
  const float xs = axis.x * sinAngle;
  const float ys = axis.y * sinAngle;
  const float zs = axis.z * sinAngle;

  x.x = x2 * oneMinusCos + cosAngle;
  y.x = xy - zs;
  z.x = xz + ys;
  x.y = xy + zs;
  y.y = y2 * oneMinusCos + cosAngle;
  z.y = yz - xs;
  x.z = xz - ys;
  y.z = yz + xs;
  z.z = z2 * oneMinusCos + cosAngle;
}

///////////////////////////////////////////////////////////////////////

Mat4::Mat4(void)
{
  setIdentity();
}

Mat4::Mat4(const Vec4& sx, const Vec4& sy, const Vec4& sz, const Vec4& sw):
  x(sx),
  y(sy),
  z(sz),
  w(sw)
{
}

Mat4::Mat4(const String& string)
{
  std::istringstream stream(string);

  stream >> x.x >> x.y >> x.z >> x.w;
  stream >> y.x >> y.y >> y.z >> y.w;
  stream >> z.x >> z.y >> z.z >> z.w;
  stream >> w.x >> w.y >> w.z >> w.w;
}

void Mat4::transpose(void)
{
  float temp;

  temp = x.y; x.y = y.x; y.x = temp;
  temp = x.z; x.z = z.x; z.x = temp;
  temp = x.w; x.w = w.x; w.x = temp;
  temp = y.z; y.z = z.y; z.y = temp;
  temp = y.w; y.w = w.y; w.y = temp;
  temp = z.w; z.w = w.z; w.z = temp;
}

void Mat4::transformVector(Vec3& vector) const
{
  Vec3 temp;

  temp.x = vector.x * x.x + vector.y * y.x + vector.z * z.x + w.x;
  temp.y = vector.x * x.y + vector.y * y.y + vector.z * z.y + w.y;
  temp.z = vector.x * x.z + vector.y * y.z + vector.z * z.z + w.z;

  vector = temp;
}

void Mat4::transformVector(Vec4& vector) const
{
  Vec4 temp;

  temp.x = vector.x * x.x + vector.y * y.x + vector.z * z.x + vector.w * w.x;
  temp.y = vector.x * x.y + vector.y * y.y + vector.z * z.y + vector.w * w.y;
  temp.z = vector.x * x.z + vector.y * y.z + vector.z * z.z + vector.w * w.z;
  temp.w = vector.x * x.w + vector.y * y.w + vector.z * z.w + vector.w * w.w;

  vector = temp;
}

void Mat4::translateVector(Vec3& vector) const
{
  vector.x += w.x;
  vector.y += w.y;
  vector.z += w.z;
}

void Mat4::rotateVector(Vec3& vector) const
{
  Vec3 temp;

  temp.x = vector.x * x.x + vector.y * y.x + vector.z * z.x;
  temp.y = vector.x * x.y + vector.y * y.y + vector.z * z.y;
  temp.z = vector.x * x.z + vector.y * y.z + vector.z * z.z;

  vector = temp;
}

Mat4::operator float* (void)
{
  return &(x.x);
}

Mat4::operator const float* (void) const
{
  return &(x.x);
}

float Mat4::operator () (unsigned int row, unsigned int column) const
{
  return (&(x.x))[column * 4 + row];
}

float& Mat4::operator () (unsigned int row, unsigned int column)
{
  return (&(x.x))[column * 4 + row];
}

Mat4 Mat4::operator + (const Mat4& matrix) const
{
  return Mat4(x + matrix.x, y + matrix.y, z + matrix.z, w + matrix.w);
}

Mat4 Mat4::operator - (const Mat4& matrix) const
{
  return Mat4(x - matrix.x, y - matrix.y, z - matrix.z, w - matrix.w);
}

Mat4 Mat4::operator * (const Mat4& matrix) const
{
  Mat4 result;

  result.x.x = x.x * matrix.x.x + y.x * matrix.x.y + z.x * matrix.x.z + w.x * matrix.x.w;
  result.y.x = x.x * matrix.y.x + y.x * matrix.y.y + z.x * matrix.y.z + w.x * matrix.y.w;
  result.z.x = x.x * matrix.z.x + y.x * matrix.z.y + z.x * matrix.z.z + w.x * matrix.z.w;
  result.w.x = x.x * matrix.w.x + y.x * matrix.w.y + z.x * matrix.w.z + w.x * matrix.w.w;

  result.x.y = x.y * matrix.x.x + y.y * matrix.x.y + z.y * matrix.x.z + w.y * matrix.x.w;
  result.y.y = x.y * matrix.y.x + y.y * matrix.y.y + z.y * matrix.y.z + w.y * matrix.y.w;
  result.z.y = x.y * matrix.z.x + y.y * matrix.z.y + z.y * matrix.z.z + w.y * matrix.z.w;
  result.w.y = x.y * matrix.w.x + y.y * matrix.w.y + z.y * matrix.w.z + w.y * matrix.w.w;

  result.x.z = x.z * matrix.x.x + y.z * matrix.x.y + z.z * matrix.x.z + w.z * matrix.x.w;
  result.y.z = x.z * matrix.y.x + y.z * matrix.y.y + z.z * matrix.y.z + w.z * matrix.y.w;
  result.z.z = x.z * matrix.z.x + y.z * matrix.z.y + z.z * matrix.z.z + w.z * matrix.z.w;
  result.w.z = x.z * matrix.w.x + y.z * matrix.w.y + z.z * matrix.w.z + w.z * matrix.w.w;

  result.x.w = x.w * matrix.x.x + y.w * matrix.x.y + z.w * matrix.x.z + w.w * matrix.x.w;
  result.y.w = x.w * matrix.y.x + y.w * matrix.y.y + z.w * matrix.y.z + w.w * matrix.y.w;
  result.z.w = x.w * matrix.z.x + y.w * matrix.z.y + z.w * matrix.z.z + w.w * matrix.z.w;
  result.w.w = x.w * matrix.w.x + y.w * matrix.w.y + z.w * matrix.w.z + w.w * matrix.w.w;

  return result;
}

Vec4 Mat4::operator * (const Vec4& vector) const
{
  Vec4 result = vector;
  transformVector(result);
  return result;
}

Mat4& Mat4::operator += (const Mat4& matrix)
{
  x += matrix.x;
  y += matrix.y;
  z += matrix.z;
  w += matrix.w;

  return *this;
}

Mat4& Mat4::operator -= (const Mat4& matrix)
{
  x -= matrix.x;
  y -= matrix.y;
  z -= matrix.z;
  w -= matrix.w;

  return *this;
}

Mat4& Mat4::operator *= (const Mat4& matrix)
{
  Mat4 result;

  result.x.x = x.x * matrix.x.x + y.x * matrix.x.y + z.x * matrix.x.z + w.x * matrix.x.w;
  result.y.x = x.x * matrix.y.x + y.x * matrix.y.y + z.x * matrix.y.z + w.x * matrix.y.w;
  result.z.x = x.x * matrix.z.x + y.x * matrix.z.y + z.x * matrix.z.z + w.x * matrix.z.w;
  result.w.x = x.x * matrix.w.x + y.x * matrix.w.y + z.x * matrix.w.z + w.x * matrix.w.w;

  result.x.y = x.y * matrix.x.x + y.y * matrix.x.y + z.y * matrix.x.z + w.y * matrix.x.w;
  result.y.y = x.y * matrix.y.x + y.y * matrix.y.y + z.y * matrix.y.z + w.y * matrix.y.w;
  result.z.y = x.y * matrix.z.x + y.y * matrix.z.y + z.y * matrix.z.z + w.y * matrix.z.w;
  result.w.y = x.y * matrix.w.x + y.y * matrix.w.y + z.y * matrix.w.z + w.y * matrix.w.w;

  result.x.z = x.z * matrix.x.x + y.z * matrix.x.y + z.z * matrix.x.z + w.z * matrix.x.w;
  result.y.z = x.z * matrix.y.x + y.z * matrix.y.y + z.z * matrix.y.z + w.z * matrix.y.w;
  result.z.z = x.z * matrix.z.x + y.z * matrix.z.y + z.z * matrix.z.z + w.z * matrix.z.w;
  result.w.z = x.z * matrix.w.x + y.z * matrix.w.y + z.z * matrix.w.z + w.z * matrix.w.w;

  result.x.w = x.w * matrix.x.x + y.w * matrix.x.y + z.w * matrix.x.z + w.w * matrix.x.w;
  result.y.w = x.w * matrix.y.x + y.w * matrix.y.y + z.w * matrix.y.z + w.w * matrix.y.w;
  result.z.w = x.w * matrix.z.x + y.w * matrix.z.y + z.w * matrix.z.z + w.w * matrix.z.w;
  result.w.w = x.w * matrix.w.x + y.w * matrix.w.y + z.w * matrix.w.z + w.w * matrix.w.w;

  return operator = (result);
}

String Mat4::asString(void) const
{
  std::ostringstream stream;

  stream << x.x << ' ' << x.y << ' ' << x.z << ' ' << x.w << ' '
         << y.x << ' ' << y.y << ' ' << y.z << ' ' << y.w << ' '
         << z.x << ' ' << z.y << ' ' << z.z << ' ' << z.w << ' '
         << w.x << ' ' << w.y << ' ' << w.z << ' ' << w.w;

  return stream.str();
}

void Mat4::setIdentity(void)
{
  x.set(1.f, 0.f, 0.f, 0.f);
  y.set(0.f, 1.f, 0.f, 0.f);
  z.set(0.f, 0.f, 1.f, 0.f);
  w.set(0.f, 0.f, 0.f, 1.f);
}

void Mat4::set(const Vec4& sx, const Vec4& sy, const Vec4& sz, const Vec4& sw)
{
  x = sx;
  y = sy;
  z = sz;
  w = sw;
}

void Mat4::setQuatRotation(const Quat& quat)
{
  x.x = 1.f - 2.f * quat.y * quat.y - 2.f * quat.z * quat.z;
  x.y = 2.f * quat.x * quat.y + 2.f * quat.w * quat.z;
  x.z = 2.f * quat.x * quat.z - 2.f * quat.w * quat.y;

  y.x = 2.f * quat.x * quat.y - 2.f * quat.w * quat.z;
  y.y = 1.f - 2.f * quat.x * quat.x - 2.f * quat.z * quat.z;
  y.z = 2.f * quat.y * quat.z + 2.f * quat.w * quat.x;

  z.x = 2.f * quat.x * quat.z + 2.f * quat.w * quat.y;
  z.y = 2.f * quat.y * quat.z - 2.f * quat.w * quat.x;
  z.z = 1.f - 2.f * quat.x * quat.x - 2.f * quat.y * quat.y;
}

void Mat4::setVectorRotation(const Vec3& vector)
{
  // NOTE: This code sucks.

  Vec3 origin(0.f, 0.f, 1.f);

  const float dot = origin.dot(vector);

  if (fabsf(dot) < 0.9999f)
  {
    Vec3 axis = origin.cross(vector).normalized();
    setAxisRotation(axis, acosf(vector.dot(origin)));
  }
  else if (dot < 0.f)
  {
    const Vec3 axis(1.f, 0.f, 0.f);
    setAxisRotation(axis, (float) M_PI);
  }
  else
    setIdentity();
}

void Mat4::setAxisRotation(const Vec3& axis, float angle)
{
  const float cosAngle = cosf(angle);
  const float sinAngle = sinf(angle);

  const float oneMinusCos = 1.f - cosAngle;

  const float x2 = axis.x * axis.x;
  const float y2 = axis.y * axis.y;
  const float z2 = axis.z * axis.z;
  const float xy = axis.x * axis.y * oneMinusCos;
  const float xz = axis.x * axis.z * oneMinusCos;
  const float yz = axis.y * axis.z * oneMinusCos;
  const float xs = axis.x * sinAngle;
  const float ys = axis.y * sinAngle;
  const float zs = axis.z * sinAngle;

  x.x = x2 * oneMinusCos + cosAngle;
  y.x = xy - zs;
  z.x = xz + ys;
  x.y = xy + zs;
  y.y = y2 * oneMinusCos + cosAngle;
  z.y = yz - xs;
  x.z = xz - ys;
  y.z = yz + xs;
  z.z = z2 * oneMinusCos + cosAngle;
}

void Mat4::getMatrixRotation(Mat3& matrix) const
{
  matrix.x.x = x.x;
  matrix.x.y = x.y;
  matrix.x.z = x.z;

  matrix.y.x = y.x;
  matrix.y.y = y.y;
  matrix.y.z = y.z;

  matrix.z.x = z.x;
  matrix.z.y = z.y;
  matrix.z.z = z.z;
}

void Mat4::setMatrixRotation(const Mat3& matrix)
{
  x.x = matrix.x.x;
  x.y = matrix.x.y;
  x.z = matrix.x.z;

  y.x = matrix.y.x;
  y.y = matrix.y.y;
  y.z = matrix.y.z;

  z.x = matrix.z.x;
  z.y = matrix.z.y;
  z.z = matrix.z.z;
}

void Mat4::getTranslation(Vec3& vector) const
{
  vector.x = w.x;
  vector.y = w.y;
  vector.z = w.z;
}

void Mat4::setTranslation(const Vec3& vector)
{
  w.x = vector.x;
  w.y = vector.y;
  w.z = vector.z;
}

void Mat4::setOrthoProjection(float width, float height)
{
  x.set(2.f / width, 0.f, 0.f, 0.f);
  y.set(0.f, 2.f / height, 0.f, 0.f);
  z.set(0.f, 0.f, -1.f, 0.f);
  w.set(-1.f, -1.f, 0.f, 1.f);
}

void Mat4::setOrthoProjection(const AABB& volume)
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  volume.getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  Vec3 t;
  t.x = -(maxX + minX) / (maxX - minX);
  t.y = -(maxY + minY) / (maxY - minY);
  t.z = -(maxZ + minZ) / (maxZ - minZ);

  x.set(2.f / (maxX - minX), 0.f, 0.f, 0.f);
  y.set(0.f, 2.f / (maxY - minY), 0.f, 0.f);
  z.set(0.f, 0.f, -2.f / (maxZ - minZ), 0.f);
  w.set(t.x, t.y, t.z, 1.f);
}

void Mat4::setPerspectiveProjection(float FOV, float aspect, float nearZ, float farZ)
{
  const float f = 1.f / tanf((FOV * (float) M_PI / 180.f) / 2.f);

  x.set(f / aspect, 0.f, 0.f, 0.f);
  y.set(0.f, f, 0.f, 0.f);
  z.set(0.f, 0.f, (farZ + nearZ) / (nearZ - farZ), -1.f);
  w.set(0.f, 0.f, (2.f * farZ * nearZ) / (nearZ - farZ), 0.f);
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
