///////////////////////////////////////////////////////////////////////
// Wendy default renderer
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
#include <wendy/OpenGL.h>
#include <wendy/GLLight.h>
#include <wendy/GLShader.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderStyle.h>
#include <wendy/RenderQueue.h>
#include <wendy/RenderSprite.h>
#include <wendy/RenderParticle.h>
#include <wendy/RenderMesh.h>
#include <wendy/RenderTerrain.h>
#include <wendy/RenderScene.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

SceneNode::SceneNode(void):
  visible(true),
  dirtyWorld(false),
  dirtyBounds(false)
{
}

SceneNode::~SceneNode(void)
{
}

bool SceneNode::isVisible(void) const
{
  return visible;
}

void SceneNode::setVisible(bool enabled)
{
  visible = enabled;
}

Transform3& SceneNode::getLocalTransform(void)
{
  dirtyWorld = true;
  return local;
}

const Transform3& SceneNode::getLocalTransform(void) const
{
  return local;
}

const Transform3& SceneNode::getWorldTransform(void) const
{
  updateWorldTransform();
  return world;
}

const Sphere& SceneNode::getLocalBounds(void) const
{
  return localBounds;
}

void SceneNode::setLocalBounds(const Sphere& newBounds)
{
  localBounds = newBounds;

  for (SceneNode* parent = this;  parent;  parent = parent->getParent())
    parent->dirtyBounds = true;
}

const Sphere& SceneNode::getTotalBounds(void) const
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

void SceneNode::addedToParent(SceneNode& parent)
{
  dirtyWorld = true;

  for (SceneNode* parent = this;  parent;  parent = parent->getParent())
    parent->dirtyBounds = true;
}

void SceneNode::removedFromParent(void)
{
  dirtyWorld = true;

  for (SceneNode* parent = this;  parent;  parent = parent->getParent())
    parent->dirtyBounds = true;
}

void SceneNode::update(Time deltaTime)
{
  const List& children = getChildren();

  for (List::const_iterator i = children.begin();  i != children.end();  i++)
    (*i)->update(deltaTime);
}

void SceneNode::restart(void)
{
} 

void SceneNode::enqueue(Queue& queue, QueuePhase phase) const
{
  const List& children = getChildren();

  for (List::const_iterator i = children.begin();  i != children.end();  i++)
  {
    if ((*i)->isVisible())
    {
      // TODO: Make less gluäöusch.

      Sphere worldBounds = (*i)->getTotalBounds();
      worldBounds.transformBy((*i)->getWorldTransform());

      if (queue.getCamera().getFrustum().intersects(worldBounds))
	(*i)->enqueue(queue, phase);
    }
  }
}

bool SceneNode::updateWorldTransform(void) const
{
  world = local;
  if (const SceneNode* parent = getParent())
  {
    parent->updateWorldTransform();
    world.concatenate(parent->world);
  }
  
  /* TODO: Fix this.

  const SceneNode* parent = getParent();
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

Scene::Scene(const String& name):
  Managed<Scene>(name),
  currentTime(0.0)
{
}

Scene::~Scene(void)
{
  while (!roots.empty())
  {
    delete roots.back();
    roots.pop_back();
  }
}

void Scene::enqueue(Queue& queue) const
{
  NodeList nodes;
  query(queue.getCamera().getFrustum(), nodes);

  for (NodeList::const_iterator i = nodes.begin();  i != nodes.end();  i++)
    (*i)->enqueue(queue, COLLECT_LIGHTS);

  for (NodeList::const_iterator i = nodes.begin();  i != nodes.end();  i++)
    (*i)->enqueue(queue, COLLECT_GEOMETRY);
}

void Scene::query(const Sphere& bounds, NodeList& nodes) const
{
  for (NodeList::const_iterator i = roots.begin();  i != roots.end();  i++)
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

void Scene::query(const Frustum& frustum, NodeList& nodes) const
{
  for (NodeList::const_iterator i = roots.begin();  i != roots.end();  i++)
  {
    if ((*i)->isVisible())
    {
      Sphere total = (*i)->getTotalBounds();
      total.transformBy((*i)->getWorldTransform());

      if (frustum.intersects(total))
	nodes.push_back(*i);
    }
  }
}

void Scene::addNode(SceneNode& node)
{
  if (std::find(roots.begin(), roots.end(), &node) != roots.end())
    return;

  roots.push_back(&node);
}

void Scene::removeNode(SceneNode& node)
{
  NodeList::iterator i = std::find(roots.begin(), roots.end(), &node);
  if (i != roots.end())
    roots.erase(i);
}

void Scene::removeNodes(void)
{
  roots.clear();
}

const Scene::NodeList& Scene::getNodes(void) const
{
  return roots;
}

Time Scene::getTimeElapsed(void) const
{
  return currentTime;
}

void Scene::setTimeElapsed(Time newTime)
{
  Time deltaTime = newTime - currentTime;

  if (deltaTime < 0.0)
  {
    for (NodeList::const_iterator i = roots.begin();  i != roots.end();  i++)
      (*i)->restart();

    deltaTime = newTime;
  }

  for (NodeList::const_iterator i = roots.begin();  i != roots.end();  i++)
    (*i)->update(deltaTime);

  currentTime = newTime;
}

///////////////////////////////////////////////////////////////////////

GL::Light* LightNode::getLight(void) const
{
  return light;
}

void LightNode::setLight(GL::Light* newLight)
{
  light = newLight;
}

void LightNode::update(Time deltaTime)
{
  SceneNode::update(deltaTime);

  setLocalBounds(light->getBounds());
}

void LightNode::enqueue(Queue& queue, QueuePhase phase) const
{
  SceneNode::enqueue(queue, phase);

  if (phase != COLLECT_LIGHTS)
    return;

  const Transform3& world = getWorldTransform();

  switch (light->getType())
  {
    case GL::Light::DIRECTIONAL:
    {
      Vector3 direction(0.f, 0.f, 1.f);
      world.rotateVector(direction);
      light->setDirection(direction);
      break;
    }

    case GL::Light::POINT:
    {
      Vector3 position(0.f, 0.f, 0.f);
      world.transformVector(position);
      light->setPosition(position);
      break;
    }
  }

  queue.attachLight(*light);
}

///////////////////////////////////////////////////////////////////////

Mesh* MeshNode::getMesh(void) const
{
  return mesh;
}

void MeshNode::setMesh(Mesh* newMesh)
{
  mesh = newMesh;
  setLocalBounds(mesh->getBounds());
}

void MeshNode::enqueue(Queue& queue, QueuePhase phase) const
{
  SceneNode::enqueue(queue, phase);

  if (phase == COLLECT_GEOMETRY)
  {
    if (mesh)
      mesh->enqueue(queue, getWorldTransform());
  }
}

///////////////////////////////////////////////////////////////////////

const String& CameraNode::getCameraName(void) const
{
  return cameraName;
}

void CameraNode::setCameraName(const String& newName)
{
  cameraName = newName;
}

void CameraNode::update(Time deltaTime)
{
  SceneNode::update(deltaTime);

  Camera* camera = Camera::findInstance(cameraName);
  if (!camera)
  {
    Log::writeError("Cannot find camera %s for camera node", cameraName.c_str());
    return;
  }

  camera->setTransform(getWorldTransform());
}

///////////////////////////////////////////////////////////////////////

Terrain* TerrainNode::getTerrain(void) const
{
  return terrain;
}

void TerrainNode::setTerrain(Terrain* newTerrain)
{
  terrain = newTerrain;
}

void TerrainNode::update(Time deltaTime)
{
  SceneNode::update(deltaTime);

  if (terrain)
    setLocalBounds(terrain->getBounds());
}

void TerrainNode::enqueue(Queue& queue, QueuePhase phase) const
{
  SceneNode::enqueue(queue, phase);

  if (phase == COLLECT_GEOMETRY)
  {
    if (terrain)
      terrain->enqueue(queue, getWorldTransform());
  }
}

///////////////////////////////////////////////////////////////////////

SpriteNode::SpriteNode(void)
{
  setSpriteSize(Vector2(1.f, 1.f));
}

Style* SpriteNode::getStyle(void) const
{
  return style;
}

void SpriteNode::setStyle(Style* newStyle)
{
  style = newStyle;
}

const Vector2& SpriteNode::getSpriteSize(void) const
{
  return spriteSize;
}

void SpriteNode::setSpriteSize(const Vector2& newSize)
{
  spriteSize = newSize;

  setLocalBounds(Sphere(Vector3::ZERO, (newSize / 2.f).length()));
}

void SpriteNode::enqueue(Queue& queue, QueuePhase phase) const
{
  SceneNode::enqueue(queue, phase);

  if (phase == COLLECT_GEOMETRY)
  {
    Sprite3 sprite;
    sprite.size = spriteSize;
    sprite.style = style;
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
  SceneNode::update(deltaTime);

  elapsed += deltaTime;

  ParticleSystem* system = ParticleSystem::findInstance(systemName);
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

void ParticleSystemNode::enqueue(Queue& queue, QueuePhase phase) const
{
  SceneNode::enqueue(queue, phase);

  if (phase == COLLECT_GEOMETRY)
  {
    ParticleSystem* system = ParticleSystem::findInstance(systemName);
    if (!system)
    {
      Log::writeError("Cannot find particle system %s for enqueueing", systemName.c_str());
      return;
    }

    system->enqueue(queue, Transform3());
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
