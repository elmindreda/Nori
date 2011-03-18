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
  Transform2(const vec2& position, float angle);
  void invert(void);
  void rotateVector(vec2& vector) const;
  void translateVector(vec2& vector) const;
  void transformVector(vec2& vector) const;
  operator mat3 (void) const;
  Transform2 operator * (const Transform2& other) const;
  Transform2& operator *= (const Transform2& other);
  void setIdentity(void);
  void set(const vec2& newPosition, float newAngle);
  vec2 position;
  float angle;
  static Transform2 IDENTITY;
};

///////////////////////////////////////////////////////////////////////

class Transform3
{
public:
  Transform3(void);
  Transform3(const vec3& position, const quat& rotation);
  void invert(void);
  void rotateVector(vec3& vector) const;
  void translateVector(vec3& vector) const;
  void transformVector(vec3& vector) const;
  operator mat4 (void) const;
  Transform3 operator * (const Transform3& other) const;
  Transform3& operator *= (const Transform3& other);
  void setIdentity(void);
  void set(const vec3& newPosition, const quat& newRotation);
  vec3 position;
  quat rotation;
  static Transform3 IDENTITY;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_TRANSFORM_H*/
///////////////////////////////////////////////////////////////////////
