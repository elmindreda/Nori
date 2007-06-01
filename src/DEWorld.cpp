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

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace DE
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

World::World(void):
  gravity(Vector3::ZERO)
{
  worldID = dWorldCreate();
}

World::~World(void)
{
  dWorldDestroy(worldID);
}

void World::update(Time deltaTime)
{
  dWorldStep(worldID, deltaTime);
}

const Vector3& World::getGravity(void) const
{
  return gravity;
}

void World::setGravity(const Vector3& newGravity)
{
  gravity = newGravity;
  dWorldSetGravity(worldID, gravity.x, gravity.y, gravity.z);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace DE*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
