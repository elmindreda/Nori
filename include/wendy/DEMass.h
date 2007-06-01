///////////////////////////////////////////////////////////////////////
// Wendy ODE library
// Copyright (c) 2007 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_DEMASS_H
#define WENDY_DEMASS_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace DE
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

enum Axis
{
  AXIS_X = 1,
  AXIS_Y = 2,
  AXIS_Z = 3,
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opende
 */
class Mass
{
public:
  Mass(void);
  void transformBy(const Transform3& transform);
  Mass operator + (const Mass& other) const;
  Mass& operator += (const Mass& other);
  void setSphere(float density, float radius);
  void setAAB(float density, const Vector3& size);
  void setCapsule(float density, Axis axis, float length, float radius);
  void setCylinder(float density, Axis axis, float length, float radius);
  void setDefaults(void);
private:
  dMass value;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace DE*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEMASS_H*/
///////////////////////////////////////////////////////////////////////
