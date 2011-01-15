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
#ifndef WENDY_QUATERNION_H
#define WENDY_QUATERNION_H
///////////////////////////////////////////////////////////////////////

#include <vector>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class Mat3;

///////////////////////////////////////////////////////////////////////

class Quat
{
public:
  Quat(void);
  Quat(float sw, float sx, float sy, float sz);
  explicit Quat(const String& string);
  void invert(void);
  void normalize(void);
  void rotateVector(Vec3& vector) const;
  float dot(const Quat& other) const;
  String asString(void) const;
  Quat interpolateTo(float t, const Quat& other) const;
  operator float* (void);
  operator const float* (void) const;
  Quat operator - (void) const;
  Quat operator * (float value) const;
  Quat operator / (float value) const;
  Quat operator + (const Quat& quat) const;
  Quat operator * (const Quat& quat) const;
  Quat& operator += (const Quat& quat);
  Quat& operator *= (const Quat& quat);
  bool operator == (const Quat& quat) const;
  bool operator != (const Quat& quat) const;
  void set(float sw, float sx, float sy, float sz);
  void setIdentity(void);
  void setEulerRotation(const Vec3& angles);
  void setMatrixRotation(const Mat3& matrix);
  void setVectorRotation(const Vec3& vector);
  void getAxisRotation(Vec3& axis, float& angle) const;
  void setAxisRotation(const Vec3& axis, float angle);
  float w;
  float x;
  float y;
  float z;
  static const Quat IDENTITY;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_QUATERNION_H*/
///////////////////////////////////////////////////////////////////////
