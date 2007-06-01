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
#ifndef WENDY_DESHAPE_H
#define WENDY_DESHAPE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace DE
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @ingroup opende
 */
class Shape
{
public:
  virtual ~Shape(void);
  bool isEnabled(void) const;
  void setEnabled(bool newState);
  unsigned long getCategoryBits(void) const;
  void setCategoryBits(unsigned long newBits);
  unsigned long getCollideBits(void) const;
  void setCollideBits(unsigned long newBits);
  Body* getBody(void) const;
  void setBody(Body* newBody);
  Transform3 getTransform(void) const;
  void setTransform(const Transform3& newTransform);
protected:
  Shape(dGeomID geomID);
  dGeomID geomID;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opende
 */
class SphereShape : public Shape
{
public:
  SphereShape(float radius);
  float getRadius(void) const;
  void setRadius(float newRadius);
};

///////////////////////////////////////////////////////////////////////

  } /*namespace DE*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DESHAPE_H*/
///////////////////////////////////////////////////////////////////////
