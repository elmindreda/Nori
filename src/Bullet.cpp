///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#include <wendy/Config.hpp>
#include <wendy/Core.hpp>
#include <wendy/Transform.hpp>
#include <wendy/Path.hpp>
#include <wendy/Resource.hpp>
#include <wendy/Vertex.hpp>
#include <wendy/Mesh.hpp>

#include <wendy/Bullet.hpp>

namespace wendy
{
  namespace bullet
  {

Transform3 convert(const btTransform& transform, float scale)
{
  Transform3 result;
  result.position = vec3(transform.getOrigin().x(),
                         transform.getOrigin().y(),
                         transform.getOrigin().z());

  btQuaternion rotation = transform.getRotation();
  result.rotation = quat(rotation.w(), rotation.x(), rotation.y(), rotation.z());

  result.scale = scale;
  return result;
}

btTransform convert(const Transform3& transform)
{
  btTransform result;
  result.setOrigin(btVector3(transform.position.x,
                             transform.position.y,
                             transform.position.z));

  result.setRotation(btQuaternion(transform.rotation.x,
                                  transform.rotation.y,
                                  transform.rotation.z,
                                  transform.rotation.w));

  return result;
}

vec3 convert(const btVector3& vector)
{
  return vec3(vector.x(), vector.y(), vector.z());
}

btVector3 convert(const vec3& vector)
{
  return btVector3(vector.x, vector.y, vector.z);
}

btTriangleMesh* convert(const Mesh& data)
{
  btTriangleMesh* mesh;

  if (data.vertices.size() > 65536)
    mesh = new btTriangleMesh(true);
  else
    mesh = new btTriangleMesh(false);

  for (const MeshSection& s : data.sections)
  {
    for (const MeshTriangle& t : s.triangles)
    {
      mesh->addTriangle(convert(data.vertices[t.indices[0]].position),
                        convert(data.vertices[t.indices[1]].position),
                        convert(data.vertices[t.indices[2]].position),
                        true);
    }
  }

  return mesh;
}

Ref<BvhTriangleMeshShape> BvhTriangleMeshShape::create(const ResourceInfo& info,
                                                       const Mesh& data)
{
  Ref<BvhTriangleMeshShape> shape(new BvhTriangleMeshShape(info));
  if (!shape->init(data))
    return nullptr;

  return shape;
}

Ref<BvhTriangleMeshShape> BvhTriangleMeshShape::read(ResourceCache& cache,
                                                     const std::string& meshName)
{
  std::string name;

  name += "source:";
  name += meshName;

  if (Ref<BvhTriangleMeshShape> shape = cache.find<BvhTriangleMeshShape>(name))
    return shape;

  Ref<Mesh> data = Mesh::read(cache, meshName);
  if (!data)
  {
    logError("Failed to read mesh for mesh shape %s", name.c_str());
    return nullptr;
  }

  return create(ResourceInfo(cache, name), *data);
}

BvhTriangleMeshShape::BvhTriangleMeshShape(const ResourceInfo& info):
  Resource(info)
{
}

bool BvhTriangleMeshShape::init(const Mesh& data)
{
  m_mesh.reset(convert(data));
  m_shape.reset(new btBvhTriangleMeshShape(m_mesh.get(), true, true));
  return true;
}

AvatarSweepCallback::AvatarSweepCallback(const btCollisionObject* self):
  self(self)
{
}

bool AvatarSweepCallback::needsCollision(btBroadphaseProxy* proxy) const
{
  if (!ConvexResultCallback::needsCollision(proxy))
    return false;

  if (proxy->m_clientObject == self)
    return false;

  return true;
}

btScalar AvatarSweepCallback::addSingleResult(btCollisionWorld::LocalConvexResult& result,
                                              bool normalInWorldSpace)
{
  m_hitCollisionObject = result.m_hitCollisionObject;

  if (normalInWorldSpace)
    m_hitNormalWorld = result.m_hitNormalLocal;
  else
  {
    const btTransform& transform = m_hitCollisionObject->getWorldTransform();
    m_hitNormalWorld = transform.getBasis() * result.m_hitNormalLocal;
  }

  return m_closestHitFraction = result.m_hitFraction;
}

  } /*namespace bullet*/
} /*namespace wendy*/

