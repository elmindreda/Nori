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

Transform2 Transform2::inverse() const
{
  Transform2 inverse;
  inverse.angle = -angle;
  inverse.position = inverse.rotate(-position);
  inverse.scale = 1.f / scale;
  return inverse;
}

vec2 Transform2::rotate(vec2 vector) const
{
  const float sina = sin(angle);
  const float cosa = cos(angle);

  return vec2(vector.x * cosa - vector.y * sina,
              vector.x * sina + vector.y * cosa);
}

vec2 Transform2::operator * (vec2 vector) const
{
  return rotate(vector * scale) + position;
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
  return Transform2(position + rotate(other.position),
                    angle + other.angle,
                    scale * other.scale);
}

Transform2& Transform2::operator *= (const Transform2& other)
{
  return operator = (operator * (other));
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

Transform3 Transform3::inverse() const
{
  const quat ir = glm::inverse(rotation);
  return Transform3(ir * -position, ir, 1.f / scale);
}

Transform3::operator mat4 () const
{
  mat4 result = glm::mat4_cast(rotation);

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

vec3 Transform3::operator * (vec3 vector) const
{
  return rotation * scale * vector + position;
}

Transform3 Transform3::operator * (const Transform3& other) const
{
  return Transform3(position + rotation * other.position,
                    rotation * other.rotation,
                    scale * other.scale);
}

Transform3& Transform3::operator *= (const Transform3& other)
{
  return operator = (operator * (other));
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
