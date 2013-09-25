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

#include <wendy/Config.hpp>
#include <wendy/Core.hpp>
#include <wendy/Transform.hpp>

#include <glm/gtx/transform.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Transform2::Transform2():
  angle(0.f),
  scale(0.f)
{
}

Transform2::Transform2(const vec2& initPosition, float initAngle, float initScale):
  position(initPosition),
  angle(initAngle),
  scale(initScale)
{
}

void Transform2::invert()
{
  angle = -angle;
  scale = 1.f / scale;
  position = -position;
  rotateVector(position);
}

void Transform2::scaleVector(vec2& vector) const
{
  vector *= scale;
}

void Transform2::rotateVector(vec2& vector) const
{
  const float sina = sin(angle);
  const float cosa = cos(angle);

  vec2 result;

  result.x = vector.x * cosa - vector.y * sina;
  result.y = vector.x * sina + vector.y * cosa;
  vector = result;
}

void Transform2::translateVector(vec2& vector) const
{
  vector += position;
}

void Transform2::transformVector(vec2& vector) const
{
  vector *= scale;
  rotateVector(vector);
  vector += position;
}

Transform2::operator mat3 () const
{
  const float sina = sin(angle) * scale;
  const float cosa = cos(angle) * scale;

  mat3 result;
  result[0][0] = cosa;
  result[0][1] = sina;
  result[1][0] = -sina;
  result[1][1] = cosa;
  result[2][0] = position.x;
  result[2][1] = position.y;

  return result;
}

Transform2 Transform2::operator * (const Transform2& other) const
{
  Transform2 result(*this);
  result *= other;
  return result;
}

Transform2& Transform2::operator *= (const Transform2& other)
{
  vec2 local = other.position;
  rotateVector(local);
  position += local;
  angle += other.angle;
  scale *= other.scale;
  return *this;
}

void Transform2::setIdentity()
{
  position = vec2(0.f);
  angle = 0.f;
  scale = 1.f;
}

void Transform2::set(const vec2& newPosition, float newAngle, float newScale)
{
  position = newPosition;
  angle = newAngle;
  scale = newScale;
}

Transform2 Transform2::IDENTITY;

///////////////////////////////////////////////////////////////////////

Transform3::Transform3():
  scale(1.f)
{
}

Transform3::Transform3(const vec3& initPosition,
                       const quat& initRotation,
                       float initScale):
  position(initPosition),
  rotation(initRotation),
  scale(initScale)
{
}

void Transform3::invert()
{
  rotation = inverse(rotation);
  position = rotation * -position;
  scale = 1.f / scale;
}

void Transform3::scaleVector(vec3& vector) const
{
  vector *= scale;
}

void Transform3::rotateVector(vec3& vector) const
{
  vector = rotation * vector;
}

void Transform3::translateVector(vec3& vector) const
{
  vector += position;
}

void Transform3::transformVector(vec3& vector) const
{
  vector = rotation * scale * vector + position;
}

Transform3::operator mat4 () const
{
  mat4 result = mat4_cast(rotation);

  for (size_t x = 0;  x < 3;  x++)
  {
    for (size_t y = 0;  y < 3;  y++)
      result[x][y] *= scale;
  }

  result[3][0] = position.x;
  result[3][1] = position.y;
  result[3][2] = position.z;

  return result;
}

vec3 Transform3::operator * (const vec3& point) const
{
  return rotation * scale * point + position;
}

Transform3 Transform3::operator * (const Transform3& other) const
{
  Transform3 result(*this);
  result *= other;
  return result;
}

Transform3& Transform3::operator *= (const Transform3& other)
{
  position += rotation * other.position;
  rotation = rotation * other.rotation;
  scale *= other.scale;
  return *this;
}

void Transform3::setIdentity()
{
  rotation = quat();
  position = vec3();
  scale = 1.f;
}

void Transform3::set(const vec3& newPosition,
                     const quat& newRotation,
                     float newScale)
{
  position = newPosition;
  rotation = newRotation;
  scale = newScale;
}

Transform3 Transform3::IDENTITY;

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
