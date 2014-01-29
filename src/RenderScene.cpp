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

#include <wendy/Config.hpp>

#include <wendy/Core.hpp>
#include <wendy/Timer.hpp>
#include <wendy/Profile.hpp>
#include <wendy/Transform.hpp>
#include <wendy/Primitive.hpp>
#include <wendy/Frustum.hpp>
#include <wendy/Camera.hpp>

#include <wendy/Pass.hpp>
#include <wendy/Material.hpp>
#include <wendy/RenderScene.hpp>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Light::Light():
  m_type(DIRECTIONAL),
  m_radius(1.f),
  m_color(1.f)
{
}

void Light::enqueue(Scene& scene,
                    const Camera& camera,
                    const Transform3& transform) const
{
  LightData data;

  data.type = m_type;
  data.radius = m_radius;
  data.color = m_color;
  data.direction = transform.rotation * vec3(0.f, 0.f, -1.f);
  data.position = transform.position;

  scene.addLight(data);
}

Sphere Light::bounds() const
{
  return Sphere(vec3(0.f), m_radius);
}

void Light::setType(LightType newType)
{
  m_type = newType;
}

void Light::setRadius(float newRadius)
{
  m_radius = newRadius;
}

void Light::setColor(const vec3& newColor)
{
  m_color = newColor;
}

///////////////////////////////////////////////////////////////////////

RenderOpKey RenderOpKey::makeOpaqueKey(uint8 layer, uint16 state, float depth)
{
  RenderOpKey key;
  key.layer = layer;
  key.state = state;
  key.depth = (unsigned) (((1 << 24) - 1) * clamp(depth, 0.f, 1.f));

  return key;
}

RenderOpKey RenderOpKey::makeBlendedKey(uint8 layer, float depth)
{
  RenderOpKey key;
  key.layer = layer;
  key.depth = (unsigned) (((1 << 24) - 1) * (1.f - clamp(depth, 0.f, 1.f)));

  return key;
}

///////////////////////////////////////////////////////////////////////

RenderOp::RenderOp():
  state(nullptr)
{
}

///////////////////////////////////////////////////////////////////////

RenderQueue::RenderQueue():
  m_sorted(true)
{
}

void RenderQueue::addOperation(const RenderOp& operation, RenderOpKey key)
{
  key.index = (uint16) m_operations.size();
  m_keys.push_back(key);
  m_operations.push_back(operation);
  m_sorted = false;
}

void RenderQueue::removeOperations()
{
  m_operations.clear();
  m_keys.clear();
  m_sorted = true;
}

const std::vector<uint64>& RenderQueue::keys() const
{
  if (!m_sorted)
  {
    std::sort(m_keys.begin(), m_keys.end());
    m_sorted = true;
  }

  return m_keys;
}

///////////////////////////////////////////////////////////////////////

Scene::Scene(RenderContext& context, Phase phase):
  m_context(context),
  m_phase(phase)
{
}

void Scene::addOperation(const RenderOp& operation, float depth, uint8 layer)
{
  if (operation.state->isBlending())
  {
    RenderOpKey key = RenderOpKey::makeBlendedKey(layer, depth);
    m_blendedQueue.addOperation(operation, key);
  }
  else
  {
    RenderOpKey key = RenderOpKey::makeOpaqueKey(layer, operation.state->ID(), depth);
    m_opaqueQueue.addOperation(operation, key);
  }
}

void Scene::createOperations(const mat4& transform,
                             const PrimitiveRange& range,
                             const Material& material,
                             float depth)
{
  RenderOp operation;
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

void Scene::addLight(const LightData& light)
{
  m_lights.push_back(light);
}

void Scene::removeLights()
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

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
