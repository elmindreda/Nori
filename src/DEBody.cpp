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

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace DE
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Body::Body(World& initWorld):
  world(initWorld)
{
  bodyID = dBodyCreate(world.worldID);
  dBodySetData(bodyID, this);
}

Body::~Body(void)
{
  dBodyDestroy(bodyID);
}

void Body::addForce(const Vector3& force)
{
}

void Body::addForce(const Vector3& force, const Vector3& position)
{
}

void Body::addTorque(const Vector3& torque)
{
}

bool Body::isEnabled(void) const
{
  return dBodyIsEnabled(bodyID);
}

void Body::setEnabled(bool newState)
{
  if (newState)
    dBodyEnable(bodyID);
  else
    dBodyDisable(bodyID);
}

Mass Body::getMass(void) const
{
  // TODO: The code.
  return Mass();
}

void Body::setMass(const Mass& newMass)
{
  // TODO: The code.
}

Vector3 Body::getForce(void) const
{
  const dReal* force = dBodyGetForce(bodyID);
  return Vector3(force[0], force[1], force[2]);
}

void Body::setForce(const Vector3& newForce)
{
  dBodySetForce(bodyID, newForce.x, newForce.y, newForce.z);
}

Vector3 Body::getTorque(void) const
{
  const dReal* torque = dBodyGetTorque(bodyID);
  return Vector3(torque[0], torque[1], torque[2]);
}

void Body::setTorque(const Vector3& newTorque)
{
  dBodySetTorque(bodyID, newTorque.x, newTorque.y, newTorque.z);
}

Transform3 Body::getTransform(void) const
{
  Transform3 transform;

  const dReal* position = dBodyGetPosition(bodyID);
  transform.position.set(position[0], position[1], position[2]);

  const dReal* rotation = dBodyGetQuaternion(bodyID);
  transform.rotation.set(rotation[0], rotation[1], rotation[2], rotation[3]);

  return transform;
}

void Body::setTransform(const Transform3& newTransform)
{
  const Vector3& position = newTransform.position;
  dBodySetPosition(bodyID, position.x, position.y, position.z);
  dBodySetQuaternion(bodyID, newTransform.rotation);
}

void Body::setLinearVelocity(const Vector3& newVelocity)
{
  dBodySetLinearVel(bodyID, newVelocity.x, newVelocity.y, newVelocity.z);
}

void Body::setAngularVelocity(const Vector3& newVelocity)
{
  dBodySetAngularVel(bodyID, newVelocity.x, newVelocity.y, newVelocity.z);
}

World& Body::getWorld(void) const
{
  return world;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace DE*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
