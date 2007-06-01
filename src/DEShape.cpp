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
#include <wendy/DEWorld.h>
#include <wendy/DEMass.h>
#include <wendy/DEBody.h>
#include <wendy/DEShape.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace DE
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Shape::~Shape(void)
{
  dGeomDestroy(geomID);
}

bool Shape::isEnabled(void) const
{
  return dGeomIsEnabled(geomID);
}

void Shape::setEnabled(bool newState)
{
  if (newState)
    dGeomEnable(geomID);
  else
    dGeomDisable(geomID);
}

unsigned long Shape::getCategoryBits(void) const
{
  return dGeomGetCategoryBits(geomID);
}

void Shape::setCategoryBits(unsigned long newBits)
{
  dGeomSetCategoryBits(geomID, newBits);
}

unsigned long Shape::getCollideBits(void) const
{
  return dGeomGetCollideBits(geomID);
}

void Shape::setCollideBits(unsigned long newBits)
{
  dGeomSetCollideBits(geomID, newBits);
}

Body* Shape::getBody(void) const
{
  dBodyID bodyID = dGeomGetBody(geomID);
  return reinterpret_cast<Body*>(dBodyGetData(bodyID));
}

void Shape::setBody(Body* newBody)
{
  if (newBody)
    dGeomSetBody(geomID, newBody->bodyID);
  else
    dGeomSetBody(geomID, 0);
}

Transform3 Shape::getTransform(void) const
{
}

void Shape::setTransform(const Transform3& newTransform)
{
}

Shape::Shape(dGeomID initGeomID):
  geomID(initGeomID)
{
  dGeomSetData(geomID, this);
}

///////////////////////////////////////////////////////////////////////

SphereShape::SphereShape(float radius):
  Shape(dCreateSphere(0, radius))
{
}

float SphereShape::getRadius(void) const
{
  return dGeomSphereGetRadius(geomID);
}

void SphereShape::setRadius(float newRadius)
{
  dGeomSphereSetRadius(geomID, newRadius);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace DE*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
