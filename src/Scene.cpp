///////////////////////////////////////////////////////////////////////
// Wendy scene graph
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/Time.hpp>
#include <wendy/Profile.hpp>
#include <wendy/Transform.hpp>
#include <wendy/Primitive.hpp>
#include <wendy/Frustum.hpp>
#include <wendy/Camera.hpp>

#include <wendy/Texture.hpp>
#include <wendy/RenderBuffer.hpp>
#include <wendy/Program.hpp>
#include <wendy/RenderContext.hpp>
#include <wendy/Pass.hpp>
#include <wendy/Material.hpp>
#include <wendy/RenderQueue.hpp>
#include <wendy/Model.hpp>
#include <wendy/Scene.hpp>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

SceneNode::SceneNode():
  m_parent(nullptr),
  m_graph(nullptr),
  m_dirtyWorld(false),
  m_dirtyBounds(false)
{
}

SceneNode::~SceneNode()
{
  destroyChildren();
  removeFromParent();
}

bool SceneNode::addChild(SceneNode& child)
{
  if (isChildOf(child))
    return false;

  child.removeFromParent();

  m_children.push_back(&child);
  child.m_parent = this;
  child.setGraph(m_graph);
  child.invalidateWorldTransform();

  invalidateBounds();
  return true;
}

void SceneNode::removeFromParent()
{
  if (m_parent || m_graph)
  {
    if (m_parent)
    {
      auto& siblings = m_parent->m_children;
      siblings.erase(std::find(siblings.begin(), siblings.end(), this));

      m_parent->invalidateBounds();
      m_parent = nullptr;

      invalidateWorldTransform();
    }
    else
    {
      auto& roots = m_graph->m_roots;
      roots.erase(std::find(roots.begin(), roots.end(), this));
    }

    setGraph(nullptr);
  }
}

void SceneNode::destroyChildren()
{
  while (!m_children.empty())
    delete m_children.back();
}

bool SceneNode::isChildOf(const SceneNode& node) const
{
  if (m_parent)
  {
    if (m_parent == &node)
      return true;

    return m_parent->isChildOf(node);
  }

  return false;
}

void SceneNode::setLocalTransform(const Transform3& newTransform)
{
  m_local = newTransform;

  if (m_parent)
    m_parent->invalidateBounds();

  invalidateWorldTransform();
}

void SceneNode::setLocalPosition(const vec3& newPosition)
{
  m_local.position = newPosition;

  if (m_parent)
    m_parent->invalidateBounds();

  invalidateWorldTransform();
}

void SceneNode::setLocalRotation(const quat& newRotation)
{
  m_local.rotation = newRotation;

  if (m_parent)
    m_parent->invalidateBounds();

  invalidateWorldTransform();
}

void SceneNode::setLocalScale(float newScale)
{
  m_local.scale = newScale;

  if (m_parent)
    m_parent->invalidateBounds();

  invalidateWorldTransform();
}

const Transform3& SceneNode::worldTransform() const
{
  if (m_dirtyWorld)
  {
    if (m_parent)
      m_world = m_parent->worldTransform() * m_local;
    else
      m_world = m_local;

    m_dirtyWorld = false;
  }

  return m_world;
}

const Sphere& SceneNode::localBounds() const
{
  return m_localBounds;
}

void SceneNode::setLocalBounds(const Sphere& newBounds)
{
  m_localBounds = newBounds;
  invalidateBounds();
}

const Sphere& SceneNode::totalBounds() const
{
  if (m_dirtyBounds)
  {
    m_totalBounds = m_localBounds;

    for (auto c : m_children)
    {
      Sphere childBounds = c->totalBounds();
      childBounds.transformBy(c->localTransform());
      m_totalBounds.envelop(childBounds);
    }

    m_dirtyBounds = false;
  }

  return m_totalBounds;
}

void SceneNode::setRenderable(Renderable* newRenderable)
{
  m_renderable = newRenderable;

  if (m_renderable)
    setLocalBounds(m_renderable->bounds());
  else
    setLocalBounds(Sphere());
}

void SceneNode::setCamera(Camera* newCamera)
{
  if (m_graph)
  {
    if (m_camera)
    {
      auto& updated = m_graph->m_updated;
      updated.erase(std::find(updated.begin(), updated.end(), this));
    }
    else if (newCamera)
      m_graph->m_updated.push_back(this);
  }

  m_camera = newCamera;
}

void SceneNode::update()
{
  if (m_camera)
    m_camera->setTransform(worldTransform());
}

void SceneNode::enqueue(RenderQueue& queue, const Camera& camera) const
{
  if (m_renderable)
    m_renderable->enqueue(queue, camera, worldTransform());

  for (auto c : m_children)
    c->enqueue(queue, camera);
}

void SceneNode::invalidateBounds()
{
  for (SceneNode* node = this;  node;  node = node->parent())
    node->m_dirtyBounds = true;
}

void SceneNode::invalidateWorldTransform()
{
  m_dirtyWorld = true;

  for (auto c : m_children)
    c->invalidateWorldTransform();
}

void SceneNode::setGraph(SceneGraph* newGraph)
{
  if (m_graph && m_camera)
  {
    auto& updated = m_graph->m_updated;
    updated.erase(std::find(updated.begin(), updated.end(), this));
  }

  m_graph = newGraph;

  if (m_graph && m_camera)
    m_graph->m_updated.push_back(this);

  for (auto c : m_children)
    c->setGraph(m_graph);
}

///////////////////////////////////////////////////////////////////////

SceneGraph::~SceneGraph()
{
  destroyRootNodes();
}

void SceneGraph::update()
{
  for (auto n : m_updated)
    n->update();
}

void SceneGraph::enqueue(RenderQueue& queue, const Camera& camera) const
{
  ProfileNodeCall call("SceneGraph::enqueue");

  const Frustum& frustum = camera.frustum();

  for (auto r : m_roots)
  {
    Sphere worldBounds = r->totalBounds();
    worldBounds.transformBy(r->worldTransform());

    if (frustum.intersects(worldBounds))
      r->enqueue(queue, camera);
  }
}

void SceneGraph::query(const Sphere& sphere, std::vector<SceneNode*>& nodes) const
{
  for (auto r : m_roots)
  {
    Sphere worldBounds = r->totalBounds();
    worldBounds.transformBy(r->worldTransform());

    if (sphere.intersects(worldBounds))
      nodes.push_back(r);
  }
}

void SceneGraph::query(const Frustum& frustum, std::vector<SceneNode*>& nodes) const
{
  for (auto r : m_roots)
  {
    Sphere worldBounds = r->totalBounds();
    worldBounds.transformBy(r->worldTransform());

    if (frustum.intersects(worldBounds))
      nodes.push_back(r);
  }
}

void SceneGraph::addRootNode(SceneNode& node)
{
  node.removeFromParent();
  m_roots.push_back(&node);
  node.setGraph(this);
}

void SceneGraph::destroyRootNodes()
{
  while (!m_roots.empty())
    delete m_roots.back();
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
