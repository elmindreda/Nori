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
  Transform2();
  Transform2(const vec2& position, float angle, float scale);
  void invert();
  void scaleVector(vec2& vector) const;
  void rotateVector(vec2& vector) const;
  void translateVector(vec2& vector) const;
  void transformVector(vec2& vector) const;
  operator mat3 () const;
  Transform2 operator * (const Transform2& other) const;
  Transform2& operator *= (const Transform2& other);
  void setIdentity();
  void set(const vec2& newPosition, float newAngle, float newScale = 1.f);
  vec2 position;
  float angle;
  float scale;
  static Transform2 IDENTITY;
};

///////////////////////////////////////////////////////////////////////

class Transform3
{
public:
  Transform3();
  Transform3(const vec3& position, const quat& rotation, float scale = 1.f);
  void invert();
  void scaleVector(vec3& vector) const;
  void rotateVector(vec3& vector) const;
  void translateVector(vec3& vector) const;
  void transformVector(vec3& vector) const;
  operator mat4 () const;
  vec3 operator * (const vec3& point) const;
  Transform3 operator * (const Transform3& other) const;
  Transform3& operator *= (const Transform3& other);
  void setIdentity();
  void set(const vec3& newPosition, const quat& newRotation, float newScale = 1.f);
  vec3 position;
  quat rotation;
  float scale;
  static Transform3 IDENTITY;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_TRANSFORM_H*/
///////////////////////////////////////////////////////////////////////
