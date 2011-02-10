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
#include <wendy/Transform.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Transform2::Transform2(void)
{
  setIdentity();
}

Transform2::Transform2(const Vec2& initPosition, float initAngle):
  position(initPosition),
  angle(initAngle)
{
}

void Transform2::invert(void)
{
  angle = -angle;
  position.negate();
  rotateVector(position);
}

void Transform2::rotateVector(Vec2& vector) const
{
  const float sina = sinf(angle);
  const float cosa = cosf(angle);

  Vec2 result;

  result.x = vector.x * cosa - vector.y * sina;
  result.y = vector.x * sina + vector.y * cosa;
  vector = result;
}

void Transform2::translateVector(Vec2& vector) const
{
  vector += position;
}

void Transform2::transformVector(Vec2& vector) const
{
  rotateVector(vector);
  vector += position;
}

Transform2::operator Mat3 (void) const
{
  const float sina = sinf(angle);
  const float cosa = cosf(angle);

  Mat3 result;
  result.x.x = cosa;
  result.x.y = sina;
  result.y.x = -sina;
  result.y.y = cosa;
  result.z.x = position.x;
  result.z.y = position.y;

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
  Vec2 local = other.position;
  rotateVector(local);
  position += local;
  angle += other.angle;
  return *this;
}

void Transform2::setIdentity(void)
{
  position = Vec2::ZERO;
  angle = 0.f;
}

void Transform2::set(const Vec2& newPosition, float newAngle)
{
  position = newPosition;
  angle = newAngle;
}

Transform2 Transform2::IDENTITY;

///////////////////////////////////////////////////////////////////////

Transform3::Transform3(void)
{
  setIdentity();
}

Transform3::Transform3(const Vec3& initPosition, const Quat& initRotation):
  position(initPosition),
  rotation(initRotation)
{
}

void Transform3::invert(void)
{
  rotation.invert();
  position.negate();
  rotation.rotateVector(position);
}

void Transform3::rotateVector(Vec3& vector) const
{
  rotation.rotateVector(vector);
}

void Transform3::translateVector(Vec3& vector) const
{
  vector += position;
}

void Transform3::transformVector(Vec3& vector) const
{
  rotation.rotateVector(vector);
  vector += position;
}

Transform3::operator Mat4 (void) const
{
  Mat4 result;
  result.setIdentity();
  result.setQuatRotation(rotation);
  result.setTranslation(position);
  return result;
}

Transform3 Transform3::operator * (const Transform3& other) const
{
  Transform3 result(*this);
  result *= other;
  return result;
}

Transform3& Transform3::operator *= (const Transform3& other)
{
  Vec3 local = other.position;
  rotateVector(local);
  position += local;
  rotation *= other.rotation;
  return *this;
}

void Transform3::setIdentity(void)
{
  rotation.setIdentity();
  position = Vec3::ZERO;
}

void Transform3::set(const Vec3& newPosition, const Quat& newRotation)
{
  position = newPosition;
  rotation = newRotation;
}

Transform3 Transform3::IDENTITY;

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
