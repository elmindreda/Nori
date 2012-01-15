///////////////////////////////////////////////////////////////////////
// Wendy default renderer
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Transform.h>

#include <wendy/RenderPool.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderScene.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

namespace
{

struct SortKeyComparator
{
  inline bool operator () (SortKey first, SortKey second)
  {
    return first.value < second.value;
  }
};

} /*namespace*/

///////////////////////////////////////////////////////////////////////

SortKey SortKey::makeOpaqueKey(uint8 layer, uint16 state, float depth)
{
  SortKey key;
  key.value = 0;
  key.layer = layer;
  key.state = state;
  key.depth = (unsigned) (((1 << 24) - 1) * clamp(depth, 0.f, 1.f));

  return key;
}

SortKey SortKey::makeBlendedKey(uint8 layer, float depth)
{
  SortKey key;
  key.value = 0;
  key.layer = layer;
  key.depth = (unsigned) (((1 << 24) - 1) * (1.f - clamp(depth, 0.f, 1.f)));

  return key;
}

///////////////////////////////////////////////////////////////////////

Operation::Operation():
  state(NULL)
{
}

///////////////////////////////////////////////////////////////////////

Queue::Queue():
  sorted(false)
{
}

void Queue::addOperation(const Operation& operation, SortKey key)
{
  key.index = (uint16) operations.size();
  keys.push_back(key);

  operations.push_back(operation);

  sorted = false;
}

void Queue::removeOperations()
{
  operations.clear();
  keys.clear();
  sorted = true;
}

const OperationList& Queue::getOperations() const
{
  return operations;
}

const SortKeyList& Queue::getSortKeys() const
{
  if (!sorted)
  {
    std::sort(keys.begin(), keys.end(), SortKeyComparator());
    sorted = true;
  }

  return keys;
}

///////////////////////////////////////////////////////////////////////

Scene::Scene(GeometryPool& initPool, Phase initPhase):
  pool(&initPool),
  phase(initPhase)
{
}

void Scene::addOperation(const Operation& operation, float depth, uint8 layer)
{
  if (operation.state->isBlending())
  {
    SortKey key = SortKey::makeBlendedKey(layer, depth);
    blendedQueue.addOperation(operation, key);
  }
  else
  {
    SortKey key = SortKey::makeOpaqueKey(layer, operation.state->getID(), depth);
    opaqueQueue.addOperation(operation, key);
  }
}

void Scene::createOperations(const mat4& transform,
                             const GL::PrimitiveRange& range,
                             const Material& material,
                             float depth)
{
  Operation operation;
  operation.range = range;
  operation.transform = transform;

  const PassList& passes = material.getTechnique(phase).passes;
  uint8 layer = 0;

  for (PassList::const_iterator p = passes.begin();  p != passes.end();  p++)
  {
    operation.state = &(*p);
    addOperation(operation, depth, layer++);
  }
}

void Scene::removeOperations()
{
  opaqueQueue.removeOperations();
  blendedQueue.removeOperations();
}

void Scene::attachLight(Light& light)
{
  if (std::find(lights.begin(), lights.end(), &light) != lights.end())
    return;

  lights.push_back(&light);
}

void Scene::detachLights()
{
  lights.clear();
}

const LightList& Scene::getLights() const
{
  return lights;
}

const vec3& Scene::getAmbientIntensity() const
{
  return ambient;
}

void Scene::setAmbientIntensity(const vec3& newIntensity)
{
  ambient = newIntensity;
}

GeometryPool& Scene::getGeometryPool() const
{
  return *pool;
}

Queue& Scene::getOpaqueQueue()
{
  return opaqueQueue;
}

const Queue& Scene::getOpaqueQueue() const
{
  return opaqueQueue;
}

Queue& Scene::getBlendedQueue()
{
  return blendedQueue;
}

const Queue& Scene::getBlendedQueue() const
{
  return blendedQueue;
}

Phase Scene::getPhase() const
{
  return phase;
}

void Scene::setPhase(Phase newPhase)
{
  phase = newPhase;
}

///////////////////////////////////////////////////////////////////////

Renderable::~Renderable()
{
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
