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
  key.depth = ((1 << 24) - 1) * clamp(depth, 0.f, 1.f);

  return key;
}

SortKey SortKey::makeBlendedKey(uint8 layer, float depth)
{
  SortKey key;
  key.value = 0;
  key.layer = layer;
  key.depth = ((1 << 24) - 1) * (1.f - clamp(depth, 0.f, 1.f));

  return key;
}

///////////////////////////////////////////////////////////////////////

Operation::Operation(void):
  state(NULL)
{
}

///////////////////////////////////////////////////////////////////////

Queue::Queue(void):
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

void Queue::removeOperations(void)
{
  operations.clear();
  keys.clear();
  sorted = true;
}

const OperationList& Queue::getOperations(void) const
{
  return operations;
}

const SortKeyList& Queue::getSortKeys(void) const
{
  if (!sorted)
  {
    std::sort(keys.begin(), keys.end(), SortKeyComparator());
    sorted = true;
  }

  return keys;
}

///////////////////////////////////////////////////////////////////////

Scene::Scene(GeometryPool& initPool, Technique::Type initType):
  pool(initPool),
  type(initType)
{
}

void Scene::addOperation(const Operation& operation, float depth, uint16 layer)
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

void Scene::createOperations(const Transform3& transform,
                             const GL::PrimitiveRange& range,
                             const Material& material,
                             float depth)
{
  const Technique* technique = material.findBestTechnique(type);
  if (!technique)
    return;

  Operation operation;
  operation.range = range;
  operation.transform = transform;

  const PassList& passes = technique->getPasses();
  uint16 layer = 0;

  for (PassList::const_iterator p = passes.begin();  p != passes.end();  p++)
  {
    operation.state = &(*p);
    addOperation(operation, depth, layer++);
  }
}

void Scene::removeOperations(void)
{
  opaqueQueue.removeOperations();
  blendedQueue.removeOperations();
}

GeometryPool& Scene::getGeometryPool(void) const
{
  return pool;
}

Queue& Scene::getOpaqueQueue(void)
{
  return opaqueQueue;
}

const Queue& Scene::getOpaqueQueue(void) const
{
  return opaqueQueue;
}

Queue& Scene::getBlendedQueue(void)
{
  return blendedQueue;
}

const Queue& Scene::getBlendedQueue(void) const
{
  return blendedQueue;
}

Technique::Type Scene::getTechniqueType(void) const
{
  return type;
}

void Scene::setTechniqueType(Technique::Type newType)
{
  type = newType;
}

///////////////////////////////////////////////////////////////////////

Renderable::~Renderable(void)
{
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
