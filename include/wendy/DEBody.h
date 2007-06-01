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
#ifndef WENDY_DEBODY_H
#define WENDY_DEBODY_H
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
class Body
{
  friend class Shape;
public:
  Body(World& world);
  ~Body(void);
  void addForce(const Vector3& force);
  void addForce(const Vector3& force, const Vector3& position);
  void addTorque(const Vector3& torque);
  bool isEnabled(void) const;
  void setEnabled(bool newState);
  Mass getMass(void) const;
  void setMass(const Mass& newMass);
  Vector3 getForce(void) const;
  void setForce(const Vector3& newForce);
  Vector3 getTorque(void) const;
  void setTorque(const Vector3& newTorque);
  Transform3 getTransform(void) const;
  void setTransform(const Transform3& newTransform);
  void setLinearVelocity(const Vector3& newVelocity);
  void setAngularVelocity(const Vector3& newVelocity);
  World& getWorld(void) const;
private:
  dBodyID bodyID;
  World& world;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace DE*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEBODY_H*/
///////////////////////////////////////////////////////////////////////
