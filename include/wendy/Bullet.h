///////////////////////////////////////////////////////////////////////
// Wendy library
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_BULLET_H
#define WENDY_BULLET_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Transform.h>
#include <wendy/Path.h>
#include <wendy/Resource.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace bullet
  {

///////////////////////////////////////////////////////////////////////

Transform3 convert(const btTransform& transform);
btTransform convert(const Transform3& transform);

vec3 convert(const btVector3& vector);
btVector3 convert(const vec3& vector);

///////////////////////////////////////////////////////////////////////

class BvhMeshShapeReader : public ResourceReader
{
public:
  BvhMeshShapeReader(ResourceIndex& index);
  btBvhTriangleMeshShape* read(const Path& path);
};

///////////////////////////////////////////////////////////////////////

class BvhMeshShapeWriter
{
public:
  bool write(const Path& path, const btBvhTriangleMeshShape& shape);
};

///////////////////////////////////////////////////////////////////////

  } /*namespace bullet*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_BULLET_H*/
///////////////////////////////////////////////////////////////////////
