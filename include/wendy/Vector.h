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
#ifndef WENDY_VECTOR_H
#define WENDY_VECTOR_H
///////////////////////////////////////////////////////////////////////

#include <cmath>
#include <vector>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class Vec2
{
public:
  Vec2(void);
  Vec2(float sx, float sy);
  explicit Vec2(const String& string);
  float length(void) const;
  float lengthSquared(void) const;
  float dot(const Vec2& other) const;
  String asString(void) const;
  Vec2 interpolateTo(float t, const Vec2& other) const;
  Vec2 absolute(void) const;
  Vec2& scaleBy(float factor);
  Vec2& scaleTo(float length);
  Vec2& negate(void);
  Vec2& normalize(void);
  Vec2 normalized(void) const;
  operator float* (void);
  operator const float* (void) const;
  Vec2 operator - (void) const;
  Vec2 operator + (float value) const;
  Vec2 operator - (float value) const;
  Vec2 operator * (float value) const;
  Vec2 operator / (float value) const;
  Vec2 operator += (float value);
  Vec2& operator -= (float value);
  Vec2& operator *= (float value);
  Vec2& operator /= (float value);
  Vec2 operator + (const Vec2& vector) const;
  Vec2 operator - (const Vec2& vector) const;
  Vec2 operator * (const Vec2& vector) const;
  Vec2 operator / (const Vec2& vector) const;
  Vec2 operator += (const Vec2& vector);
  Vec2& operator -= (const Vec2& vector);
  Vec2& operator *= (const Vec2& vector);
  Vec2& operator /= (const Vec2& vector);
  bool operator == (const Vec2& vector) const;
  bool operator != (const Vec2& vector) const;
  void setDefaults(void);
  void set(float sx, float sy);
  float x;
  float y;
  static const Vec2 ZERO;
  static const Vec2 ONE;
  static const Vec2 X;
  static const Vec2 Y;
};

///////////////////////////////////////////////////////////////////////

class Vec2i
{
public:
  Vec2i(void);
  Vec2i(int x, int y);
  Vec2i operator - (void) const;
  Vec2i operator + (int value) const;
  Vec2i operator - (int value) const;
  Vec2i operator * (int value) const;
  Vec2i operator / (int value) const;
  Vec2i operator += (int value);
  Vec2i& operator -= (int value);
  Vec2i& operator *= (int value);
  Vec2i& operator /= (int value);
  Vec2i operator + (const Vec2i& other) const;
  Vec2i operator - (const Vec2i& other) const;
  Vec2i operator * (const Vec2i& other) const;
  Vec2i operator / (const Vec2i& other) const;
  Vec2i& operator += (const Vec2i& other);
  Vec2i& operator -= (const Vec2i& other);
  Vec2i& operator *= (const Vec2i& other);
  Vec2i& operator /= (const Vec2i& other);
  bool operator == (const Vec2i& other) const;
  bool operator != (const Vec2i& other) const;
  void setDefaults(void);
  void set(int newX, int newY);
  int x;
  int y;
};

///////////////////////////////////////////////////////////////////////

class Vec3
{
public:
  Vec3(void);
  Vec3(float sx, float sy, float sz);
  explicit Vec3(const String& string);
  float length(void) const;
  float lengthSquared(void) const;
  float dot(const Vec3& vector) const;
  String asString(void) const;
  Vec3 cross(const Vec3& vector) const;
  Vec3 interpolateTo(float t, const Vec3& other) const;
  Vec3 absolute(void) const;
  Vec3& scaleBy(float factor);
  Vec3& scaleTo(float length);
  Vec3& mirrorBy(const Vec3& vector);
  Vec3& negate(void);
  Vec3& normalize(void);
  Vec3 normalized(void) const;
  operator float* (void);
  operator const float* (void) const;
  Vec3 operator - (void) const;
  Vec3 operator + (float value) const;
  Vec3 operator - (float value) const;
  Vec3 operator * (float value) const;
  Vec3 operator / (float value) const;
  Vec3 operator += (float value);
  Vec3& operator -= (float value);
  Vec3& operator *= (float value);
  Vec3& operator /= (float value);
  Vec3 operator + (const Vec3& vector) const;
  Vec3 operator - (const Vec3& vector) const;
  Vec3 operator * (const Vec3& vector) const;
  Vec3 operator / (const Vec3& vector) const;
  Vec3 operator += (const Vec3& vector);
  Vec3& operator -= (const Vec3& vector);
  Vec3& operator *= (const Vec3& vector);
  Vec3& operator /= (const Vec3& vector);
  bool operator == (const Vec3& vector) const;
  bool operator != (const Vec3& vector) const;
  void setDefaults(void);
  void set(float sx, float sy, float sz);
  float x;
  float y;
  float z;
  static const Vec3 ZERO;
  static const Vec3 ONE;
  static const Vec3 X;
  static const Vec3 Y;
  static const Vec3 Z;
};

///////////////////////////////////////////////////////////////////////

class Vec4
{
public:
  Vec4(void);
  Vec4(float sx, float sy, float sz, float sw);
  Vec4(const Vec3& xyz, float sw);
  explicit Vec4(const String& string);
  float length(void) const;
  float lengthSquared(void) const;
  float dot(const Vec4& vector) const;
  String asString(void) const;
  Vec4 cross(const Vec4& vector) const;
  Vec4 interpolateTo(float t, const Vec4& other) const;
  Vec4 absolute(void) const;
  Vec4& scaleBy(float factor);
  Vec4& scaleTo(float length);
  Vec4& normalize(void);
  Vec4 normalized(void) const;
  operator float* (void);
  operator const float* (void) const;
  Vec4 operator - (void) const;
  Vec4 operator + (float value) const;
  Vec4 operator - (float value) const;
  Vec4 operator * (float value) const;
  Vec4 operator / (float value) const;
  Vec4 operator += (float value);
  Vec4& operator -= (float value);
  Vec4& operator *= (float value);
  Vec4& operator /= (float value);
  Vec4 operator + (const Vec4& vector) const;
  Vec4 operator - (const Vec4& vector) const;
  Vec4 operator * (const Vec4& vector) const;
  Vec4 operator / (const Vec4& vector) const;
  Vec4 operator += (const Vec4& vector);
  Vec4& operator -= (const Vec4& vector);
  Vec4& operator *= (const Vec4& vector);
  Vec4& operator /= (const Vec4& vector);
  bool operator == (const Vec4& vector) const;
  bool operator != (const Vec4& vector) const;
  void setDefaults(void);
  void set(float sx, float sy, float sz, float sw);
  void set(const Vec3& xyz, float sw);
  float x;
  float y;
  float z;
  float w;
  static const Vec4 ZERO;
  static const Vec4 ONE;
  static const Vec4 X;
  static const Vec4 Y;
  static const Vec4 Z;
  static const Vec4 W;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_VECTOR_H*/
///////////////////////////////////////////////////////////////////////
