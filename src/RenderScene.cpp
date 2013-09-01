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
#include <wendy/Timer.h>
#include <wendy/Profile.h>
#include <wendy/Transform.h>
#include <wendy/AABB.h>
#include <wendy/Plane.h>
#include <wendy/Frustum.h>
#include <wendy/Camera.h>

#include <wendy/RenderPool.h>
#include <wendy/RenderState.h>
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

SortKey SortKey::makeOpaqueKey(uint8 layer, uint16 state, float depth)
{
  SortKey key;
  key.layer = layer;
  key.state = state;
  key.depth = (unsigned) (((1 << 24) - 1) * clamp(depth, 0.f, 1.f));

  return key;
}

SortKey SortKey::makeBlendedKey(uint8 layer, float depth)
{
  SortKey key;
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
  m_sorted(true)
{
}

void Queue::addOperation(const Operation& operation, SortKey key)
{
  key.index = (uint16) m_operations.size();
  m_keys.push_back(key);
  m_operations.push_back(operation);
  m_sorted = false;
}

void Queue::removeOperations()
{
  m_operations.clear();
  m_keys.clear();
  m_sorted = true;
}

const SortKeyList& Queue::keys() const
{
  if (!m_sorted)
  {
    std::sort(m_keys.begin(), m_keys.end());
    m_sorted = true;
  }

  return m_keys;
}

///////////////////////////////////////////////////////////////////////

Scene::Scene(VertexPool& pool, Phase phase):
  m_pool(&pool),
  m_phase(phase)
{
}

void Scene::addOperation(const Operation& operation, float depth, uint8 layer)
{
  if (operation.state->isBlending())
  {
    SortKey key = SortKey::makeBlendedKey(layer, depth);
    m_blendedQueue.addOperation(operation, key);
  }
  else
  {
    SortKey key = SortKey::makeOpaqueKey(layer, operation.state->ID(), depth);
    m_opaqueQueue.addOperation(operation, key);
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

  uint8 layer = 0;

  for (auto& p : material.technique(m_phase).passes)
  {
    operation.state = &p;
    addOperation(operation, depth, layer++);
  }
}

void Scene::removeOperations()
{
  m_opaqueQueue.removeOperations();
  m_blendedQueue.removeOperations();
}

void Scene::attachLight(Light& light)
{
  if (std::find(m_lights.begin(), m_lights.end(), &light) != m_lights.end())
    return;

  m_lights.push_back(&light);
}

void Scene::detachLights()
{
  m_lights.clear();
}

void Scene::setAmbientIntensity(const vec3& newIntensity)
{
  m_ambient = newIntensity;
}

void Scene::setPhase(Phase newPhase)
{
  m_phase = newPhase;
}

///////////////////////////////////////////////////////////////////////

Renderable::~Renderable()
{
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
