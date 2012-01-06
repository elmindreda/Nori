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

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace bullet
  {

///////////////////////////////////////////////////////////////////////

/*! @ingroup bullet
 */
Transform3 convert(const btTransform& transform, float scale = 1.f);

/*! @ingroup bullet
 */
btTransform convert(const Transform3& transform);

/*! @ingroup bullet
 */
vec3 convert(const btVector3& vector);

/*! @ingroup bullet
 */
btVector3 convert(const vec3& vector);

/*! @ingroup bullet
 */
btTriangleMesh* convert(const Mesh& mesh);

///////////////////////////////////////////////////////////////////////

class BvhMeshShape : public Resource
{
public:
  BvhMeshShape(const ResourceInfo& info);
  Ptr<btBvhTriangleMeshShape> shape;
  Ptr<btTriangleIndexVertexArray> mesh;
  Ptr<btOptimizedBvh> bvh;
  Ptr<btTriangleInfoMap> info;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup bullet
 */
class BvhMeshShapeReader : public ResourceReader<BvhMeshShape>
{
public:
  BvhMeshShapeReader(ResourceCache& cache);
  using ResourceReader<BvhMeshShape>::read;
  Ref<BvhMeshShape> read(const String& name, const Path& path);
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

/*! @ingroup bullet
 */
class AvatarSweepCallback : public btCollisionWorld::ConvexResultCallback
{
public:
  AvatarSweepCallback(const btCollisionObject* self);
  bool needsCollision(btBroadphaseProxy* proxy) const;
  btScalar addSingleResult(btCollisionWorld::LocalConvexResult& result,
                           bool normalInWorldSpace);
  btVector3 m_hitNormalWorld;
  btCollisionObject* m_hitCollisionObject;
private:
  const btCollisionObject* self;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace bullet*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_BULLET_H*/
///////////////////////////////////////////////////////////////////////
