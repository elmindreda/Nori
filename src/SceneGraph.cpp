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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderQueue.h>
#include <wendy/RenderSprite.h>
#include <wendy/RenderParticle.h>
#include <wendy/RenderMesh.h>

#include <wendy/SceneGraph.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace scene
  {

///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

NodeType::NodeType(const String& name):
  Managed<NodeType>(name)
{
}

NodeType::~NodeType(void)
{
}

///////////////////////////////////////////////////////////////////////

Node::Node(void):
  parent(NULL),
  visible(true),
  dirtyWorld(false),
  dirtyBounds(false)
{
}

Node::~Node(void)
{
  destroyChildren();
  removeFromParent();
}

bool Node::addChild(Node& child)
{
  if (isChildOf(child))
    return false;

  child.removeFromParent();

  children.push_back(&child);
  child.parent = this;
  child.addedToParent(*this);

  invalidateBounds();
  return true;
}

void Node::removeFromParent(void)
{
  if (parent)
  {
    List& siblings = parent->children;
    siblings.erase(std::find(siblings.begin(), siblings.end(), this));
    parent = NULL;
    removedFromParent();
  }
}

void Node::destroyChildren(void)
{
  while (!children.empty())
    delete children.back();
}

bool Node::isChildOf(const Node& node) const
{
  if (parent != NULL)
  {
    if (parent == &node)
      return true;

    return parent->isChildOf(node);
  }

  return false;
}

bool Node::isVisible(void) const
{
  return visible;
}

bool Node::hasChildren(void) const
{
  return !children.empty();
}

Node* Node::getParent(void) const
{
  return parent;
}

const Node::List& Node::getChildren(void) const
{
  return children;
}

void Node::setVisible(bool enabled)
{
  visible = enabled;
}

Transform3& Node::getLocalTransform(void)
{
  dirtyWorld = true;

  if (parent)
    parent->invalidateBounds();

  return local;
}

const Transform3& Node::getLocalTransform(void) const
{
  return local;
}

const Transform3& Node::getWorldTransform(void) const
{
  updateWorldTransform();
  return world;
}

const Sphere& Node::getLocalBounds(void) const
{
  return localBounds;
}

void Node::setLocalBounds(const Sphere& newBounds)
{
  localBounds = newBounds;

  invalidateBounds();
}

const Sphere& Node::getTotalBounds(void) const
{
  if (dirtyBounds)
  {
    totalBounds = localBounds;

    const List& children = getChildren();

    for (List::const_iterator i = children.begin();  i != children.end();  i++)
    {
      Sphere childBounds = (*i)->getTotalBounds();
      childBounds.transformBy((*i)->getLocalTransform());
      totalBounds.envelop(childBounds);
    }

    dirtyBounds = false;
  }

  return totalBounds;
}

void Node::addedToParent(Node& parent)
{
  dirtyWorld = true;
}

void Node::removedFromParent(void)
{
  dirtyWorld = true;
}

void Node::update(Time deltaTime)
{
  const List& children = getChildren();

  for (List::const_iterator i = children.begin();  i != children.end();  i++)
    (*i)->update(deltaTime);
}

void Node::restart(void)
{
}

void Node::enqueue(render::Queue& queue, QueuePhase phase) const
{
  const List& children = getChildren();

  for (List::const_iterator i = children.begin();  i != children.end();  i++)
  {
    Node& node = **i;

    if (!node.isVisible())
      continue;

    // TODO: Make less gluäöusch.

    Sphere worldBounds = node.getTotalBounds();
    worldBounds.transformBy(node.getWorldTransform());

    const Frustum& frustum = queue.getCamera().getFrustum();

    if (frustum.intersects(worldBounds))
      node.enqueue(queue, phase);
  }
}

void Node::invalidateBounds(void)
{
  for (Node* parent = this;  parent;  parent = parent->getParent())
    parent->dirtyBounds = true;
}

bool Node::updateWorldTransform(void) const
{
  world = local;
  if (const Node* parent = getParent())
  {
    parent->updateWorldTransform();
    world *= parent->world;
  }

  /* TODO: Fix this.

  const Node* parent = getParent();
  if (parent)
  {
    if (parent->updateWorldTransform())
      dirtyWorld = true;
  }

  if (dirtyWorld)
  {
    world = local;
    if (parent)
      world.concatenate(parent->world);

    dirtyWorld = false;
    return true;
  }
  */

  return false;
}

///////////////////////////////////////////////////////////////////////

Graph::Graph(const String& name):
  Resource<Graph>(name),
  currentTime(0.0)
{
}

Graph::~Graph(void)
{
  while (!roots.empty())
  {
    delete roots.back();
    roots.pop_back();
  }
}

void Graph::enqueue(render::Queue& queue) const
{
  Node::List nodes;
  query(queue.getCamera().getFrustum(), nodes);

  for (Node::List::const_iterator i = nodes.begin();  i != nodes.end();  i++)
    (*i)->enqueue(queue, COLLECT_LIGHTS);

  for (Node::List::const_iterator i = nodes.begin();  i != nodes.end();  i++)
    (*i)->enqueue(queue, COLLECT_GEOMETRY);
}

void Graph::query(const Sphere& bounds, Node::List& nodes) const
{
  for (Node::List::const_iterator i = roots.begin();  i != roots.end();  i++)
  {
    if ((*i)->isVisible())
    {
      Sphere total = (*i)->getTotalBounds();
      total.transformBy((*i)->getWorldTransform());

      if (bounds.intersects(total))
        nodes.push_back(*i);
    }
  }
}

void Graph::query(const Frustum& frustum, Node::List& nodes) const
{
  for (Node::List::const_iterator i = roots.begin();  i != roots.end();  i++)
  {
    Node& node = **i;

    if (node.isVisible())
    {
      Sphere total = node.getTotalBounds();
      total.transformBy(node.getWorldTransform());

      if (frustum.intersects(total))
        nodes.push_back(&node);
    }
  }
}

void Graph::addNode(Node& node)
{
  if (std::find(roots.begin(), roots.end(), &node) != roots.end())
    return;

  roots.push_back(&node);
}

void Graph::removeNode(Node& node)
{
  Node::List::iterator i = std::find(roots.begin(), roots.end(), &node);
  if (i != roots.end())
    roots.erase(i);
}

void Graph::removeNodes(void)
{
  roots.clear();
}

const Node::List& Graph::getNodes(void) const
{
  return roots;
}

Time Graph::getTimeElapsed(void) const
{
  return currentTime;
}

void Graph::setTimeElapsed(Time newTime)
{
  Time deltaTime = newTime - currentTime;

  if (deltaTime < 0.0)
  {
    for (Node::List::const_iterator i = roots.begin();  i != roots.end();  i++)
      (*i)->restart();

    deltaTime = newTime;
  }

  for (Node::List::const_iterator i = roots.begin();  i != roots.end();  i++)
    (*i)->update(deltaTime);

  currentTime = newTime;
}

///////////////////////////////////////////////////////////////////////

render::Light* LightNode::getLight(void) const
{
  return light;
}

void LightNode::setLight(render::Light* newLight)
{
  light = newLight;
}

void LightNode::update(Time deltaTime)
{
  Node::update(deltaTime);

  //setLocalBounds(light->getBounds());
}

void LightNode::enqueue(render::Queue& queue, QueuePhase phase) const
{
  Node::enqueue(queue, phase);

  if (phase != COLLECT_LIGHTS)
    return;

  /*
  const Transform3& world = getWorldTransform();

  switch (light->getType())
  {
    case Light::DIRECTIONAL:
    {
      Vec3 direction(0.f, 0.f, 1.f);
      world.rotateVector(direction);
      light->setDirection(direction);
      break;
    }

    case Light::POINT:
    {
      Vec3 position(0.f, 0.f, 0.f);
      world.transformVector(position);
      light->setPosition(position);
      break;
    }
  }
  */

  queue.attachLight(*light);
}

///////////////////////////////////////////////////////////////////////

render::Mesh* MeshNode::getMesh(void) const
{
  return mesh;
}

void MeshNode::setMesh(render::Mesh* newMesh)
{
  mesh = newMesh;
  setLocalBounds(mesh->getBounds());
}

void MeshNode::enqueue(render::Queue& queue, QueuePhase phase) const
{
  Node::enqueue(queue, phase);

  if (phase == COLLECT_GEOMETRY)
  {
    if (mesh)
      mesh->enqueue(queue, getWorldTransform());
  }
}

///////////////////////////////////////////////////////////////////////

render::Camera* CameraNode::getCamera(void) const
{
  return camera;
}

void CameraNode::setCamera(render::Camera* newCamera)
{
  camera = newCamera;
}

void CameraNode::update(Time deltaTime)
{
  Node::update(deltaTime);

  if (!camera)
    return;

  camera->setTransform(getWorldTransform());
}

///////////////////////////////////////////////////////////////////////

SpriteNode::SpriteNode(void)
{
  setSpriteSize(Vec2(1.f, 1.f));
}

render::Material* SpriteNode::getMaterial(void) const
{
  return material;
}

void SpriteNode::setMaterial(render::Material* newMaterial)
{
  material = newMaterial;
}

const Vec2& SpriteNode::getSpriteSize(void) const
{
  return spriteSize;
}

void SpriteNode::setSpriteSize(const Vec2& newSize)
{
  spriteSize = newSize;

  setLocalBounds(Sphere(Vec3::ZERO, (newSize / 2.f).length()));
}

void SpriteNode::enqueue(render::Queue& queue, QueuePhase phase) const
{
  Node::enqueue(queue, phase);

  if (phase == COLLECT_GEOMETRY)
  {
    render::Sprite3 sprite;
    sprite.size = spriteSize;
    sprite.material = material;
    sprite.enqueue(queue, getWorldTransform());
  }
}

///////////////////////////////////////////////////////////////////////

ParticleSystemNode::ParticleSystemNode(void):
  elapsed(0.0)
{
}

const String& ParticleSystemNode::getSystemName(void) const
{
  return systemName;
}

void ParticleSystemNode::setSystemName(const String& newSystemName)
{
  systemName = newSystemName;
}

void ParticleSystemNode::update(Time deltaTime)
{
  Node::update(deltaTime);

  elapsed += deltaTime;

  render::ParticleSystem* system = render::ParticleSystem::findInstance(systemName);
  if (!system)
  {
    Log::writeError("Cannot find particle system %s for updating", systemName.c_str());
    return;
  }

  system->setTransform(getWorldTransform());
  system->setTimeElapsed(elapsed);
  setLocalBounds(system->getBounds());
}

void ParticleSystemNode::restart(void)
{
  elapsed = 0.0;
}

void ParticleSystemNode::enqueue(render::Queue& queue, QueuePhase phase) const
{
  Node::enqueue(queue, phase);

  if (phase == COLLECT_GEOMETRY)
  {
    render::ParticleSystem* system = render::ParticleSystem::findInstance(systemName);
    if (!system)
    {
      Log::writeError("Cannot find particle system %s for enqueueing", systemName.c_str());
      return;
    }

    system->enqueue(queue, Transform3());
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace scene*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
