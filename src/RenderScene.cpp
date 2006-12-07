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
    for (const SceneNode* child = getFirstChild();  child;  child = child->getNextSibling())
      totalBounds.envelop(child->getTotalBounds());

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
  for (SceneNode* node = getFirstChild();  node;  node = node->getNextSibling())
    node->update(deltaTime);
}

void SceneNode::restart(void)
{
} 

void SceneNode::enqueue(Queue& queue, QueuePhase phase) const
{
  for (const SceneNode* node = getFirstChild();  node;  node = node->getNextSibling())
  {
    if (node->isVisible())
    {
      // TODO: Make less gluäöusch.

      Sphere worldBounds = node->getTotalBounds();
      worldBounds.transformBy(node->getWorldTransform());

      if (queue.getCamera().getFrustum().intersects(worldBounds))
	node->enqueue(queue, phase);
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

void Scene::enqueue(Queue& queue)
{
  NodeList nodes;
  query(queue.getCamera().getFrustum(), nodes);

  for (NodeList::const_iterator i = nodes.begin();  i != nodes.end();  i++)
    (*i)->enqueue(queue, COLLECT_LIGHTS);

  for (NodeList::const_iterator i = nodes.begin();  i != nodes.end();  i++)
    (*i)->enqueue(queue, COLLECT_GEOMETRY);
}

void Scene::query(const Sphere& bounds, NodeList& nodes)
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

void Scene::query(const Frustum& frustum, NodeList& nodes)
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

  if (deltaTime < 0.f)
  {
    for (NodeList::const_iterator i = roots.begin();  i != roots.end();  i++)
      (*i)->restart();

    deltaTime = newTime;
  }

  if (deltaTime == 0.f)
    return;

  for (NodeList::const_iterator i = roots.begin();  i != roots.end();  i++)
    (*i)->update(deltaTime);

  currentTime = newTime;
}

///////////////////////////////////////////////////////////////////////

const String& LightNode::getLightName(void) const
{
  return lightName;
}

void LightNode::setLightName(const String& newLightName)
{
  lightName = newLightName;
}

void LightNode::enqueue(Queue& queue, QueuePhase phase) const
{
  SceneNode::enqueue(queue, phase);

  if (GL::Light* light = GL::Light::findInstance(lightName))
  {
    Transform3 transform = getWorldTransform();
    transform.concatenate(queue.getCamera().getInverseTransform());

    switch (light->getType())
    {
      case GL::Light::DIRECTIONAL:
      {
        Vector3 direction(0.f, 0.f, 1.f);
        transform.rotateVector(direction);
        light->setDirection(direction);
        break;
      }

      case GL::Light::POINT:
      {
        Vector3 position(0.f, 0.f, 0.f);
        transform.transformVector(position);
        light->setPosition(position);
        break;
      }
    }

    queue.attachLight(*light);
  }
}

///////////////////////////////////////////////////////////////////////

const String& MeshNode::getMeshName(void) const
{
  return meshName;
}

void MeshNode::setMeshName(const String& newMeshName)
{
  meshName = newMeshName;
}

void MeshNode::update(Time deltaTime)
{
  SceneNode::update(deltaTime);

  Mesh* mesh = Mesh::findInstance(meshName);
  if (mesh)
    setLocalBounds(mesh->getBounds());
}

void MeshNode::enqueue(Queue& queue, QueuePhase phase) const
{
  SceneNode::enqueue(queue, phase);

  if (phase == COLLECT_GEOMETRY)
  {
    Mesh* mesh = Mesh::findInstance(meshName);
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

const String& TerrainNode::getTerrainName(void) const
{
  return terrainName;
}

void TerrainNode::setTerrainName(const String& newTerrainName)
{
  terrainName = newTerrainName;
}

void TerrainNode::enqueue(Queue& queue, QueuePhase phase) const
{
  SceneNode::enqueue(queue, phase);

  Terrain* terrain = Terrain::findInstance(terrainName);
  if (terrain)
    terrain->enqueue(queue, getWorldTransform());
}

///////////////////////////////////////////////////////////////////////

const String& SpriteNode::getStyleName(void) const
{
  return styleName;
}

void SpriteNode::setStyleName(const String& newStyleName)
{
  styleName = newStyleName;
}

const Vector2& SpriteNode::getSpriteSize(void) const
{
  return spriteSize;
}

void SpriteNode::setSpriteSize(const Vector2& newSize)
{
  spriteSize = newSize;
}

void SpriteNode::enqueue(Queue& queue, QueuePhase phase) const
{
  SceneNode::enqueue(queue, phase);

  Sprite3 sprite;
  sprite.size = spriteSize;
  sprite.styleName = styleName;
  sprite.enqueue(queue, getWorldTransform());
}

///////////////////////////////////////////////////////////////////////

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

  ParticleSystem* system = ParticleSystem::findInstance(systemName);
  if (!system)
  {
    Log::writeError("Cannot find particle system %s for updating", systemName.c_str());
    return;
  }

  system->setTransform(getWorldTransform());
  setLocalBounds(system->getBounds());
}

void ParticleSystemNode::enqueue(Queue& queue, QueuePhase phase) const
{
  SceneNode::enqueue(queue, phase);

  ParticleSystem* system = ParticleSystem::findInstance(systemName);
  if (!system)
  {
    Log::writeError("Cannot find particle system %s for enqueueing", systemName.c_str());
    return;
  }

  system->enqueue(queue, Transform3());
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////