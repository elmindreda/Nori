///////////////////////////////////////////////////////////////////////
// Wendy OpenDE library
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

#include <moira/Moira.h>

#include <wendy/Config.h>
#include <wendy/OpenDE.h>
#include <wendy/DEMass.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace DE
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Mass::Mass(void)
{
  setDefaults();
}

void Mass::transformBy(const Transform3& transform)
{
}

Mass Mass::operator + (const Mass& other) const
{
  Mass result(*this);
  result += other;
  return result;
}

Mass& Mass::operator += (const Mass& other)
{
  dMassAdd(&value, &other.value);
}

void Mass::setSphere(float density, float radius)
{
  dMassSetSphere(&value, density, radius);
}

void Mass::setAAB(float density, const Vector3& size)
{
  dMassSetBox(&value, density, size.x, size.y, size.z);
}

void Mass::setCapsule(float density, Axis axis, float length, float radius)
{
  dMassSetCapsule(&value, density, axis, radius, length);
}

void Mass::setCylinder(float density, Axis axis, float length, float radius)
{
  dMassSetCylinder(&value, density, axis, radius, length);
}

void Mass::setDefaults(void)
{
  dMassSetZero(&value);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace DE*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
