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

#include <wendy/Config.h>

#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>
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

NodeType::NodeType(const String& name):
  Managed<NodeType>(name)
{
}

NodeType::~NodeType(void)
{
}

///////////////////////////////////////////////////////////////////////

Node::Node(const String& initName):
  name(initName),
  parent(NULL),
  graph(NULL),
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
  child.setGraph(graph);
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
    setGraph(NULL);

    removedFromParent();
  }
  else if (graph)
  {
    List& roots = graph->roots;
    roots.erase(std::find(roots.begin(), roots.end(), this));

    setGraph(NULL);

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

const String& Node::getName(void) const
{
  return name;
}

void Node::setName(const String& newName)
{
  name = newName;
}

Graph* Node::getGraph(void) const
{
  return graph;
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

void Node::update(void)
{
  const List& children = getChildren();

  for (List::const_iterator i = children.begin();  i != children.end();  i++)
    (*i)->update();
}

void Node::enqueue(render::Queue& queue) const
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
      node.enqueue(queue);
  }
}

void Node::invalidateBounds(void)
{
  for (Node* parent = this;  parent;  parent = parent->getParent())
    parent->dirtyBounds = true;
}

bool Node::updateWorldTransform(void) const
{
  if (const Node* parent = getParent())
  {
    parent->updateWorldTransform();
    world = parent->world * local;
  }
  else
    world = local;

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

void Node::setGraph(Graph* newGraph)
{
  graph = newGraph;

  for (List::const_iterator c = children.begin();  c != children.end();  c++)
    (*c)->setGraph(graph);
}

///////////////////////////////////////////////////////////////////////

Graph::Graph(const String& name):
  Resource<Graph>(name)
{
}

Graph::~Graph(void)
{
  destroyRootNodes();
}

void Graph::update(void)
{
  for (Node::List::const_iterator i = roots.begin();  i != roots.end();  i++)
    (*i)->update();
}

void Graph::enqueue(render::Queue& queue) const
{
  Node::List nodes;
  query(queue.getCamera().getFrustum(), nodes);

  queue.setPhase(render::Queue::COLLECT_LIGHTS);

  for (Node::List::const_iterator i = nodes.begin();  i != nodes.end();  i++)
    (*i)->enqueue(queue);

  queue.setPhase(render::Queue::COLLECT_GEOMETRY);

  for (Node::List::const_iterator i = nodes.begin();  i != nodes.end();  i++)
    (*i)->enqueue(queue);
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

void Graph::addRootNode(Node& node)
{
  node.removeFromParent();
  roots.push_back(&node);
  node.setGraph(this);
}

void Graph::destroyRootNodes(void)
{
  while (!roots.empty())
    delete roots.back();
}

const Node::List& Graph::getNodes(void) const
{
  return roots;
}

///////////////////////////////////////////////////////////////////////

LightNode::LightNode(const String& name):
  Node(name)
{
}

render::Light* LightNode::getLight(void) const
{
  return light;
}

void LightNode::setLight(render::Light* newLight)
{
  light = newLight;
}

void LightNode::update(void)
{
  Node::update();

  if (light)
  {
    const Transform3& world = getWorldTransform();

    if (light->getType() == render::Light::DIRECTIONAL ||
        light->getType() == render::Light::SPOTLIGHT)
    {
      Vec3 direction(0.f, 0.f, -1.f);
      world.rotateVector(direction);
      light->setDirection(direction);
    }

    if (light->getType() == render::Light::POINT ||
        light->getType() == render::Light::SPOTLIGHT)
    {
      light->setPosition(world.position);
    }

    setLocalBounds(Sphere(Vec3::ZERO, light->getRadius()));
  }
  else
    setLocalBounds(Sphere(Vec3::ZERO, 0.f));
}

void LightNode::enqueue(render::Queue& queue) const
{
  Node::enqueue(queue);

  if (light)
  {
    if (queue.getPhase() == render::Queue::COLLECT_LIGHTS)
      queue.attachLight(*light);
  }
}

///////////////////////////////////////////////////////////////////////

MeshNode::MeshNode(const String& name):
  Node(name),
  shadowCaster(true)
{
}

bool MeshNode::isShadowCaster(void) const
{
  return shadowCaster;
}

void MeshNode::setCastsShadows(bool enabled)
{
  shadowCaster = enabled;
}

render::Mesh* MeshNode::getMesh(void) const
{
  return mesh;
}

void MeshNode::setMesh(render::Mesh* newMesh)
{
  mesh = newMesh;
  setLocalBounds(mesh->getBounds());
}

void MeshNode::enqueue(render::Queue& queue) const
{
  Node::enqueue(queue);

  if (mesh)
  {
    if ((queue.getPhase() == render::Queue::COLLECT_GEOMETRY) ||
        (queue.getPhase() == render::Queue::COLLECT_SHADOW_CASTERS && shadowCaster))
    {
      mesh->enqueue(queue, getWorldTransform());
    }
  }
}

///////////////////////////////////////////////////////////////////////

CameraNode::CameraNode(const String& name):
  Node(name)
{
}

render::Camera* CameraNode::getCamera(void) const
{
  return camera;
}

void CameraNode::setCamera(render::Camera* newCamera)
{
  camera = newCamera;
}

void CameraNode::update(void)
{
  Node::update();

  if (!camera)
    return;

  camera->setTransform(getWorldTransform());
}

///////////////////////////////////////////////////////////////////////

SpriteNode::SpriteNode(const String& name):
  Node(name)
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

void SpriteNode::enqueue(render::Queue& queue) const
{
  Node::enqueue(queue);

  if (queue.getPhase() == render::Queue::COLLECT_GEOMETRY)
  {
    render::Sprite3 sprite;
    sprite.size = spriteSize;
    sprite.material = material;
    sprite.enqueue(queue, getWorldTransform());
  }
}

///////////////////////////////////////////////////////////////////////

ParticleSystemNode::ParticleSystemNode(const String& name):
  Node(name)
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

void ParticleSystemNode::update(void)
{
  Node::update();

  render::ParticleSystem* system = render::ParticleSystem::findInstance(systemName);
  if (!system)
  {
    Log::writeError("Cannot find particle system \'%s\' for updating", systemName.c_str());
    return;
  }

  system->setTransform(getWorldTransform());
  setLocalBounds(system->getBounds());
}

void ParticleSystemNode::enqueue(render::Queue& queue) const
{
  Node::enqueue(queue);

  if (queue.getPhase() == render::Queue::COLLECT_GEOMETRY)
  {
    render::ParticleSystem* system = render::ParticleSystem::findInstance(systemName);
    if (!system)
    {
      Log::writeError("Cannot find particle system \'%s\' for enqueueing", systemName.c_str());
      return;
    }

    system->enqueue(queue, Transform3());
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace scene*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
