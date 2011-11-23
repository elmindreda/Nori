///////////////////////////////////////////////////////////////////////
// Wendy Bullet helpers
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

/*! @defgroup bullet Bullet helper API
 *
 *  This module provides functions and classes that make it easier to use Wendy
 *  and Bullet together in an application.  It makes no attempt to wrap Bullet,
 *  as it already provides a good C++ API.
 */

///////////////////////////////////////////////////////////////////////

/*! @ingroup bullet
 */
Transform3 convert(const btTransform& transform);

/*! @ingroup bullet
 */
btTransform convert(const Transform3& transform);

/*! @ingroup bullet
 */
vec3 convert(const btVector3& vector);

/*! @ingroup bullet
 */
btVector3 convert(const vec3& vector);

///////////////////////////////////////////////////////////////////////

/*! @ingroup bullet
 */
class BvhMeshShapeReader : public ResourceReader
{
public:
  BvhMeshShapeReader(ResourceCache& cache);
  btBvhTriangleMeshShape* read(const Path& path);
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup bullet
 */
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
