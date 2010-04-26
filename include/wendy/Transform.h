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
#ifndef WENDY_TRANSFORM_H
#define WENDY_TRANSFORM_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class Mat3;
class Mat4;

///////////////////////////////////////////////////////////////////////

class Transform2
{
public:
  Transform2(void);
  Transform2(const Vec2& position, float angle);
  void invert(void);
  void rotateVector(Vec2& vector) const;
  void translateVector(Vec2& vector) const;
  void transformVector(Vec2& vector) const;
  operator Mat3 (void) const;
  Transform2 operator * (const Transform2& parent) const;
  Transform2& operator *= (const Transform2& parent);
  void setIdentity(void);
  void set(const Vec2& newPosition, float newAngle);
  Vec2 position;
  float angle;
};

///////////////////////////////////////////////////////////////////////

class Transform3
{
public:
  Transform3(void);
  Transform3(const Vec3& position, const Quat& rotation);
  void invert(void);
  void rotateVector(Vec3& vector) const;
  void translateVector(Vec3& vector) const;
  void transformVector(Vec3& vector) const;
  operator Mat4 (void) const;
  Transform3 operator * (const Transform3& parent) const;
  Transform3& operator *= (const Transform3& parent);
  void setIdentity(void);
  void set(const Vec3& newPosition, const Quat& newRotation);
  Vec3 position;
  Quat rotation;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_TRANSFORM_H*/
///////////////////////////////////////////////////////////////////////
