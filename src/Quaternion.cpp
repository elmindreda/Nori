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
#include <wendy/Matrix.h>
#include <wendy/Quaternion.h>

#include <sstream>
#include <cmath>
#include <cstdio>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Quat::Quat(void)
{
}

Quat::Quat(float sw, float sx, float sy, float sz):
  w(sw),
  x(sx),
  y(sy),
  z(sz)
{
}

Quat::Quat(const String& string)
{
  std::istringstream stream(string, std::ios::in);

  stream >> x >> y >> z >> w;
}

void Quat::invert(void)
{
  x = -x;
  y = -y;
  z = -z;
}

void Quat::normalize(void)
{
  const float square = w * w + x * x + y * y + z * z;

  if (square * square != square)
  {
    const float scale = 1.f / sqrtf(square);

    w *= scale;
    x *= scale;
    y *= scale;
    z *= scale;
  }
}

void Quat::rotateVector(Vec3& vector) const
{
  // TODO: Replace with sane code.

  Mat3 matrix;

  matrix.setQuatRotation(*this);
  matrix.rotateVector(vector);
}

float Quat::dot(const Quat& other) const
{
  return w * other.w + x * other.x + y * other.y + z * other.z;
}

String Quat::asString(void) const
{
  std::ostringstream stream;

  stream << x << ' ' << y << ' ' << z << ' ' << w;
  return stream.str();
}

Quat Quat::interpolateTo(float t, const Quat& other) const
{
  const float cosine = dot(other);
  const float angle = acosf(cosine);

  if (fabsf(angle) < 0.01f)
    return *this;

  float scale0 = sinf((1.f - t) * angle) / sinf(angle);
  const float scale1 = sinf(t * angle) / sinf(angle);

  if (cosine < 0.f)
  {
    scale0 = -scale0;
    Quat result(*this * scale0 + other * scale1);
    result.normalize();
    return result;
  }

  return *this * scale0 + other * scale1;
}

Quat::operator float* (void)
{
  return &(w);
}

Quat::operator const float* (void) const
{
  return &(w);
}

Quat Quat::operator * (float value) const
{
  return Quat(w * value, x * value, y * value, z * value);
}

Quat Quat::operator / (float value) const
{
  return Quat(w / value, x / value, y / value, z / value);
}

Quat Quat::operator + (const Quat& quat) const
{
  return Quat(w + quat.w, x + quat.x, y + quat.y, z + quat.z);
}

Quat Quat::operator * (const Quat& quat) const
{
  Quat result;

  result.w = w * quat.w - x * quat.x - y * quat.y - z * quat.z;
  result.x = w * quat.x + x * quat.w + y * quat.z - z * quat.y;
  result.y = w * quat.y + y * quat.w + z * quat.x - x * quat.z;
  result.z = w * quat.z + z * quat.w + x * quat.y - y * quat.x;

  return result;
}

Quat& Quat::operator += (const Quat& quat)
{
  w += quat.w;
  x += quat.x;
  y += quat.y;
  z += quat.z;

  return *this;
}

Quat& Quat::operator *= (const Quat& quat)
{
  Quat temp;

  temp.w = w * quat.w - x * quat.x - y * quat.y - z * quat.z;
  temp.x = w * quat.x + x * quat.w + y * quat.z - z * quat.y;
  temp.y = w * quat.y + y * quat.w + z * quat.x - x * quat.z;
  temp.z = w * quat.z + z * quat.w + x * quat.y - y * quat.x;

  return operator = (temp);
}

bool Quat::operator == (const Quat& quat) const
{
  return w == quat.w && x == quat.x && y == quat.y && z == quat.z;
}

bool Quat::operator != (const Quat& quat) const
{
  return w != quat.w || x != quat.x || y != quat.y || z != quat.z;
}

void Quat::set(float sw, float sx, float sy, float sz)
{
  w = sw;
  x = sx;
  y = sy;
  z = sz;
}

void Quat::setIdentity(void)
{
  w = 1.f;
  x = 0.f;
  y = 0.f;
  z = 0.f;
}

void Quat::setEulerRotation(const Vec3& angles)
{
  // TODO: implement!
  /*
  const float cosX = cosf(angles.x * 0.5f);
  const float cosY = cosf(angles.y * 0.5f);
  const float cosZ = cosf(angles.z * 0.5f);

  const float sinX = sinf(angles.x * 0.5f);
  const float sinY = sinf(angles.y * 0.5f);
  const float sinZ = sinf(angles.z * 0.5f);

  w = cosX * cosY * cosZ + sinX * sinY * sinZ;
  x = sinX * cosY * cosZ - cosX * sinY * sinZ;
  y = cosX * sinY * cosZ + sinX * cosY * sinZ;
  z = cosX * cosY * sinZ - sinX * sinY * cosZ;
  */
}

void Quat::setMatrixRotation(const Mat3& matrix)
{
  const float trace = matrix.x.x + matrix.y.y + matrix.z.z;

  if (trace > 0.f)
  {
    float root = sqrtf(trace + 1.f);

    w = root * 0.5f;

    root = 0.5f / root;

    x = (matrix.y.z - matrix.z.y) * root;
    y = (matrix.z.x - matrix.x.z) * root;
    z = (matrix.x.y - matrix.y.x) * root;
  }
  else
  {
    float q[4];
    int i, j, k;

    int next[3] = { 1, 2, 0 };

    i = 0;

    if (matrix.y.y > matrix.x.x)
      i = 1;

    if (matrix.z.z > matrix(i, i))
      i = 2;

    j = next[i];
    k = next[j];

    float root = sqrtf((matrix(i, i) - (matrix(j, j) + matrix(k, k))) + 1.f);

    q[i] = root * 0.5f;

    if (root != 0.f)
      root = 0.5f / root;

    q[3] = (matrix(j, k) - matrix(k, j)) * root;
    q[j] = (matrix(i, j) + matrix(j, i)) * root;
    q[k] = (matrix(i, k) + matrix(k, i)) * root;

    w = q[3];
    x = q[0];
    y = q[1];
    z = q[2];
  }
}

void Quat::setVectorRotation(const Vec3& vector)
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

void Quat::getAxisRotation(Vec3& axis, float& angle) const
{
  const float square = w * w + x * x + y * y + z * z;

  if (square)
  {
    axis.x = x * square;
    axis.y = y * square;
    axis.z = z * square;

    angle = 2.f * acosf(w);
  }
  else
  {
    axis.x = 1.f;
    axis.y = 0.f;
    axis.z = 0.f;

    angle = 0.f;
  }
}

void Quat::setAxisRotation(const Vec3& axis, float angle)
{
  const float halfAngle = angle * 0.5f;

  w = cosf(halfAngle);
  x = axis.x * sinf(halfAngle);
  y = axis.y * sinf(halfAngle);
  z = axis.z * sinf(halfAngle);
  normalize();
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
