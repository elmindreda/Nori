///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLCamera.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLLight.h>
#include <wendy/GLRender.h>
#include <wendy/GLSprite.h>
#include <wendy/GLParticle.h>
#include <wendy/GLMesh.h>
#include <wendy/GLTerrain.h>
#include <wendy/GLScene.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

SceneNode::SceneNode(void):
  visible(true),
  dirtyWorld(false)
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

void SceneNode::addedToParent(SceneNode& parent)
{
  dirtyWorld = true;
}

void SceneNode::removedFromParent(void)
{
  dirtyWorld = true;
}

void SceneNode::update(void)
{
  for (SceneNode* node = getFirstChild();  node;  node = node->getNextSibling())
    node->update();
}

void SceneNode::enqueue(RenderQueue& queue) const
{
  for (const SceneNode* node = getFirstChild();  node;  node = node->getNextSibling())
  {
    if (node->isVisible())
      node->enqueue(queue);
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
  fogging(false),
  fogColor(ColorRGB::BLACK)
{
}

void Scene::updateTree(void)
{
  for (NodeList::const_iterator i = roots.begin();  i != roots.end();  i++)
    (*i)->update();
}

void Scene::renderTree(const Camera& camera) const
{
  RenderQueue queue(camera);
  enqueueTree(queue);

  if (fogging)
  {
    ColorRGBA color = fogColor;

    glEnable(GL_FOG);
    glFogfv(GL_FOG_COLOR, color);
  }
  else
    glDisable(GL_FOG);

  camera.begin();
  queue.renderOperations();
  camera.end();

  if (fogging)
    glDisable(GL_FOG);
}

void Scene::enqueueTree(RenderQueue& queue) const
{
  for (NodeList::const_iterator i = roots.begin();  i != roots.end();  i++)
  {
    if ((*i)->isVisible())
      (*i)->enqueue(queue);
  }
}

void Scene::addRootNode(SceneNode& node)
{
  if (std::find(roots.begin(), roots.end(), &node) != roots.end())
    return;

  roots.push_back(&node);
}

void Scene::removeRootNode(SceneNode& node)
{
  roots.remove(&node);
}

void Scene::removeRootNodes(void)
{
  roots.clear();
}

bool Scene::isFogging(void) const
{
  return fogging;
}

void Scene::setFogging(bool newState)
{
  fogging = newState;
}

const ColorRGB& Scene::getFogColor(void) const
{
  return fogColor;
}

void Scene::setFogColor(const ColorRGB& newColor)
{
  fogColor = newColor;
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

void LightNode::enqueue(RenderQueue& queue) const
{
  SceneNode::enqueue(queue);

  if (Light* light = Light::findInstance(lightName))
  {
    Transform3 transform = getWorldTransform();
    transform.concatenate(queue.getCamera().getInverseTransform());

    switch (light->getType())
    {
      case Light::DIRECTIONAL:
      {
        Vector3 direction(0.f, 0.f, 1.f);
        transform.rotateVector(direction);
        light->setDirection(direction);
        break;
      }

      case Light::POSITIONAL:
      {
        Vector3 position(0.f, 0.f, 0.f);
        transform.transformVector(position);
        light->setPosition(position);
        break;
      }
    }

    queue.addLight(*light);
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

void MeshNode::enqueue(RenderQueue& queue) const
{
  SceneNode::enqueue(queue);

  Mesh* mesh = Mesh::findInstance(meshName);
  if (mesh)
    mesh->enqueue(queue, getWorldTransform());
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

void CameraNode::update(void)
{
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

void TerrainNode::enqueue(RenderQueue& queue) const
{
  SceneNode::enqueue(queue);

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

void SpriteNode::enqueue(RenderQueue& queue) const
{
  SceneNode::enqueue(queue);

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

void ParticleSystemNode::update(void)
{
  ParticleSystem* system = ParticleSystem::findInstance(systemName);
  if (!system)
  {
    Log::writeError("Cannot find particle system %s", systemName.c_str());
    return;
  }

  system->setTransform(getWorldTransform());
}

void ParticleSystemNode::enqueue(RenderQueue& queue) const
{
  SceneNode::enqueue(queue);

  ParticleSystem* system = ParticleSystem::findInstance(systemName);
  if (!system)
  {
    Log::writeError("Cannot find particle system %s", systemName.c_str());
    return;
  }

  system->enqueue(queue, Transform3());
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
