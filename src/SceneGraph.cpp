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

#include <wendy/Core.h>
#include <wendy/Timer.h>
#include <wendy/Profile.h>

#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#include <wendy/RenderPool.h>
#include <wendy/RenderState.h>
#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderScene.h>
#include <wendy/RenderModel.h>

#include <wendy/SceneGraph.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace scene
  {

///////////////////////////////////////////////////////////////////////

Node::Node(bool initNeedsUpdate):
  needsUpdate(initNeedsUpdate),
  parent(NULL),
  graph(NULL),
  dirtyWorld(false),
  dirtyBounds(false)
{
}

Node::~Node()
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
  child.invalidateWorldTransform();

  invalidateBounds();
  return true;
}

void Node::removeFromParent()
{
  if (parent || graph)
  {
    if (parent)
    {
      List& siblings = parent->children;
      siblings.erase(std::find(siblings.begin(), siblings.end(), this));

      parent->invalidateBounds();
      parent = NULL;

      invalidateWorldTransform();
    }
    else
    {
      List& roots = graph->roots;
      roots.erase(std::find(roots.begin(), roots.end(), this));
    }

    setGraph(NULL);
  }
}

void Node::destroyChildren()
{
  while (!children.empty())
    delete children.back();
}

bool Node::isChildOf(const Node& node) const
{
  if (parent)
  {
    if (parent == &node)
      return true;

    return parent->isChildOf(node);
  }

  return false;
}

bool Node::hasChildren() const
{
  return !children.empty();
}

Graph* Node::getGraph() const
{
  return graph;
}

Node* Node::getParent() const
{
  return parent;
}

const Node::List& Node::getChildren() const
{
  return children;
}

const Transform3& Node::getLocalTransform() const
{
  return local;
}

void Node::setLocalTransform(const Transform3& newTransform)
{
  local = newTransform;

  if (parent)
    parent->invalidateBounds();

  invalidateWorldTransform();
}

void Node::setLocalPosition(const vec3& newPosition)
{
  local.position = newPosition;

  if (parent)
    parent->invalidateBounds();

  invalidateWorldTransform();
}

void Node::setLocalRotation(const quat& newRotation)
{
  local.rotation = newRotation;

  if (parent)
    parent->invalidateBounds();

  invalidateWorldTransform();
}

void Node::setLocalScale(float newScale)
{
  local.scale = newScale;

  if (parent)
    parent->invalidateBounds();

  invalidateWorldTransform();
}

const Transform3& Node::getWorldTransform() const
{
  if (dirtyWorld)
  {
    if (parent)
      world = parent->getWorldTransform() * local;
    else
      world = local;

    dirtyWorld = false;
  }

  return world;
}

const Sphere& Node::getLocalBounds() const
{
  return localBounds;
}

void Node::setLocalBounds(const Sphere& newBounds)
{
  localBounds = newBounds;
  invalidateBounds();
}

const Sphere& Node::getTotalBounds() const
{
  if (dirtyBounds)
  {
    totalBounds = localBounds;

    const List& children = getChildren();

    for (auto c = children.begin();  c != children.end();  c++)
    {
      Sphere childBounds = (*c)->getTotalBounds();
      childBounds.transformBy((*c)->getLocalTransform());
      totalBounds.envelop(childBounds);
    }

    dirtyBounds = false;
  }

  return totalBounds;
}

void Node::update()
{
  const List& children = getChildren();

  std::for_each(children.begin(), children.end(), std::mem_fun(&Node::update));
}

void Node::enqueue(render::Scene& scene, const render::Camera& camera) const
{
  const Frustum& frustum = camera.getFrustum();

  const List& children = getChildren();

  for (auto c = children.begin();  c != children.end();  c++)
  {
    Sphere worldBounds = (*c)->getTotalBounds();
    worldBounds.transformBy((*c)->getWorldTransform());

    if (frustum.intersects(worldBounds))
      (*c)->enqueue(scene, camera);
  }
}

Node::Node(const Node& source)
{
  panic("Scene graph nodes may not be copied");
}

Node& Node::operator = (const Node& source)
{
  panic("Scene graph nodes may not be assigned");
}

void Node::invalidateBounds()
{
  for (Node* node = this;  node;  node = node->getParent())
    node->dirtyBounds = true;
}

void Node::invalidateWorldTransform()
{
  dirtyWorld = true;

  std::for_each(children.begin(), children.end(), std::mem_fun(&Node::invalidateWorldTransform));
}

void Node::setGraph(Graph* newGraph)
{
  if (graph && needsUpdate)
  {
    List& updated = graph->updated;
    updated.erase(std::find(updated.begin(), updated.end(), this));
  }

  graph = newGraph;

  if (graph && needsUpdate)
  {
    List& updated = graph->updated;
    updated.push_back(this);
  }

  for (auto c = children.begin();  c != children.end();  c++)
    (*c)->setGraph(graph);
}

///////////////////////////////////////////////////////////////////////

Graph::~Graph()
{
  destroyRootNodes();
}

void Graph::update()
{
  std::for_each(updated.begin(), updated.end(), std::mem_fun(&Node::update));
}

void Graph::enqueue(render::Scene& scene, const render::Camera& camera) const
{
  ProfileNodeCall call("scene::Graph::enqueue");

  const Frustum& frustum = camera.getFrustum();

  for (auto r = roots.begin();  r != roots.end();  r++)
  {
    Sphere worldBounds = (*r)->getTotalBounds();
    worldBounds.transformBy((*r)->getWorldTransform());

    if (frustum.intersects(worldBounds))
      (*r)->enqueue(scene, camera);
  }
}

void Graph::query(const Sphere& sphere, Node::List& nodes) const
{
  for (auto r = roots.begin();  r != roots.end();  r++)
  {
    Sphere worldBounds = (*r)->getTotalBounds();
    worldBounds.transformBy((*r)->getWorldTransform());

    if (sphere.intersects(worldBounds))
      nodes.push_back(*r);
  }
}

void Graph::query(const Frustum& frustum, Node::List& nodes) const
{
  for (auto r = roots.begin();  r != roots.end();  r++)
  {
    Sphere worldBounds = (*r)->getTotalBounds();
    worldBounds.transformBy((*r)->getWorldTransform());

    if (frustum.intersects(worldBounds))
      nodes.push_back(*r);
  }
}

void Graph::addRootNode(Node& node)
{
  node.removeFromParent();
  roots.push_back(&node);
  node.setGraph(this);
}

void Graph::destroyRootNodes()
{
  while (!roots.empty())
    delete roots.back();
}

const Node::List& Graph::getNodes() const
{
  return roots;
}

///////////////////////////////////////////////////////////////////////

LightNode::LightNode():
  Node(true)
{
}

render::Light* LightNode::getLight() const
{
  return light;
}

void LightNode::setLight(render::Light* newLight)
{
  light = newLight;
}

void LightNode::update()
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

    setLocalBounds(Sphere(vec3(), light->getRadius()));
  }
  else
    setLocalBounds(Sphere());
}

void LightNode::enqueue(render::Scene& scene, const render::Camera& camera) const
{
  Node::enqueue(scene, camera);

  if (light)
    scene.attachLight(*light);
}

///////////////////////////////////////////////////////////////////////

ModelNode::ModelNode():
  shadowCaster(false)
{
}

bool ModelNode::isShadowCaster() const
{
  return shadowCaster;
}

void ModelNode::setCastsShadows(bool enabled)
{
  shadowCaster = enabled;
}

render::Model* ModelNode::getModel() const
{
  return model;
}

void ModelNode::setModel(render::Model* newModel)
{
  model = newModel;
  setLocalBounds(model->getBoundingSphere());
}

void ModelNode::enqueue(render::Scene& scene, const render::Camera& camera) const
{
  Node::enqueue(scene, camera);

  if (model)
  {
    if (scene.getPhase() == render::PHASE_SHADOWMAP && !shadowCaster)
      return;

    model->enqueue(scene, camera, getWorldTransform());
  }
}

///////////////////////////////////////////////////////////////////////

CameraNode::CameraNode():
  Node(true)
{
}

render::Camera* CameraNode::getCamera() const
{
  return camera;
}

void CameraNode::setCamera(render::Camera* newCamera)
{
  camera = newCamera;
}

void CameraNode::update()
{
  Node::update();

  if (!camera)
    return;

  camera->setTransform(getWorldTransform());
}

///////////////////////////////////////////////////////////////////////

  } /*namespace scene*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
