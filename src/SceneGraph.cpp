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
#include <wendy/RenderScene.h>
#include <wendy/RenderSprite.h>
#include <wendy/RenderModel.h>

#include <wendy/SceneGraph.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace scene
  {

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

const Transform3& Node::getLocalTransform(void) const
{
  return local;
}

void Node::setLocalTransform(const Transform3& newTransform)
{
  local = newTransform;

  if (parent)
    parent->invalidateBounds();

  dirtyWorld = true;
}

void Node::setLocalPosition(const vec3& newPosition)
{
  local.position = newPosition;

  if (parent)
    parent->invalidateBounds();

  dirtyWorld = true;
}

void Node::setLocalRotation(const quat& newRotation)
{
  local.rotation = newRotation;

  if (parent)
    parent->invalidateBounds();

  dirtyWorld = true;
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

  std::for_each(children.begin(), children.end(), std::mem_fun(&Node::update));
}

void Node::enqueue(render::Scene& scene, const render::Camera& camera) const
{
  const Frustum& frustum = camera.getFrustum();

  const List& children = getChildren();

  for (List::const_iterator i = children.begin();  i != children.end();  i++)
  {
    Node& node = **i;

    if (!node.isVisible())
      continue;

    // TODO: Make less gluäöusch.

    Sphere worldBounds = node.getTotalBounds();
    worldBounds.transformBy(node.getWorldTransform());

    if (frustum.intersects(worldBounds))
      node.enqueue(scene, camera);
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

Graph::~Graph(void)
{
  destroyRootNodes();
}

void Graph::update(void)
{
  std::for_each(roots.begin(), roots.end(), std::mem_fun(&Node::update));
}

void Graph::enqueue(render::Scene& scene, const render::Camera& camera) const
{
  Node::List nodes;
  query(camera.getFrustum(), nodes);

  for (Node::List::const_iterator i = nodes.begin();  i != nodes.end();  i++)
    (*i)->enqueue(scene, camera);
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
      vec3 direction(0.f, 0.f, -1.f);
      world.rotateVector(direction);
      light->setDirection(direction);
    }

    if (light->getType() == render::Light::POINT ||
        light->getType() == render::Light::SPOTLIGHT)
    {
      light->setPosition(world.position);
    }

    setLocalBounds(Sphere(vec3(0.f), light->getRadius()));
  }
  else
    setLocalBounds(Sphere(vec3(0.f), 0.f));
}

void LightNode::enqueue(render::Scene& scene, const render::Camera& camera) const
{
  Node::enqueue(scene, camera);

  if (light)
    scene.attachLight(*light);
}

///////////////////////////////////////////////////////////////////////

ModelNode::ModelNode(const String& name):
  Node(name),
  shadowCaster(true)
{
}

bool ModelNode::isShadowCaster(void) const
{
  return shadowCaster;
}

void ModelNode::setCastsShadows(bool enabled)
{
  shadowCaster = enabled;
}

render::Model* ModelNode::getModel(void) const
{
  return model;
}

void ModelNode::setModel(render::Model* newModel)
{
  model = newModel;
  setLocalBounds(model->getBounds());
}

void ModelNode::enqueue(render::Scene& scene, const render::Camera& camera) const
{
  Node::enqueue(scene, camera);

  if (model)
    model->enqueue(scene, camera, getWorldTransform());
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
  setSpriteSize(vec2(1.f));
}

render::Material* SpriteNode::getMaterial(void) const
{
  return material;
}

void SpriteNode::setMaterial(render::Material* newMaterial)
{
  material = newMaterial;
}

const vec2& SpriteNode::getSpriteSize(void) const
{
  return spriteSize;
}

void SpriteNode::setSpriteSize(const vec2& newSize)
{
  spriteSize = newSize;

  setLocalBounds(Sphere(vec3(0.f), length(newSize / 2.f)));
}

void SpriteNode::enqueue(render::Scene& scene, const render::Camera& camera) const
{
  Node::enqueue(scene, camera);

  render::Sprite3 sprite;
  sprite.size = spriteSize;
  sprite.material = material;
  sprite.enqueue(scene, camera, getWorldTransform());
}

///////////////////////////////////////////////////////////////////////

  } /*namespace scene*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
