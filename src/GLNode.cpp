///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2005 Camilla Berglund <elmindreda@home.se>
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

#include <moira/Config.h>
#include <moira/Core.h>
#include <moira/Node.h>
#include <moira/Color.h>
#include <moira/Vector.h>
#include <moira/Matrix.h>
#include <moira/Quaternion.h>
#include <moira/Transform.h>
#include <moira/Stream.h>
#include <moira/XML.h>
#include <moira/Mesh.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLVertex.h>
#include <wendy/GLIndexBuffer.h>
#include <wendy/GLVertexBuffer.h>
#include <wendy/GLLight.h>
#include <wendy/GLShader.h>
#include <wendy/GLSprite.h>
#include <wendy/GLRender.h>
#include <wendy/GLNode.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

SceneNode::SceneNode(void):
  visible(true)
{
  local.setIdentity();
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
  return local;
}

const Transform3& SceneNode::getLocalTransform(void) const
{
  return local;
}

const Transform3& SceneNode::getWorldTransform(void) const
{
  world = local;
  if (const SceneNode* parent = getParent())
    world.concatenate(parent->getWorldTransform());

  return world;
}

void SceneNode::prepare(void)
{
  for (SceneNode* node = getFirstChild();  node;  node = node->getNextSibling())
    node->prepare();
}

void SceneNode::enqueue(RenderQueue& queue) const
{
  for (const SceneNode* node = getFirstChild();  node;  node = node->getNextSibling())
  {
    if (node->isVisible())
      node->enqueue(queue);
  }
}

///////////////////////////////////////////////////////////////////////

const std::string& LightNode::getLightName(void) const
{
  return lightName;
}

void LightNode::setLightName(const std::string& newLightName)
{
  lightName = newLightName;
}

void LightNode::enqueue(RenderQueue& queue) const
{
  SceneNode::enqueue(queue);

  if (Light* light = Light::findInstance(lightName))
  {
    const Transform3& transform = getWorldTransform();

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

const std::string& MeshNode::getMeshName(void) const
{
  return meshName;
}

void MeshNode::setMeshName(const std::string& newMeshName)
{
  meshName = newMeshName;
}

void MeshNode::enqueue(RenderQueue& queue) const
{
  SceneNode::enqueue(queue);

  RenderMesh* mesh = RenderMesh::findInstance(meshName);
  if (mesh)
    mesh->enqueue(queue, getWorldTransform());
}

///////////////////////////////////////////////////////////////////////

CameraNode::CameraNode(void):
  FOV(90.f),
  aspectRatio(0.f)
{
}

void CameraNode::prepareTree(void)
{
  SceneNode* root = this;
  while (root->getParent())
    root = root->getParent();

  root->prepare();
}

void CameraNode::renderTree(void) const
{
  RenderQueue queue;
  enqueueTree(queue);

  Canvas* canvas = Canvas::getCurrent();

  if (aspectRatio)
    canvas->begin3D(aspectRatio, FOV);
  else
    canvas->begin3D((float) canvas->getPhysicalWidth() /
                    (float) canvas->getPhysicalHeight(), FOV);

  Transform3 worldToLocal = getWorldTransform();
  worldToLocal.invert();

  Matrix4 matrix = worldToLocal;

  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadMatrixf(matrix);
  glPopAttrib();

  queue.renderOperations();

  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();

  canvas->end();
}

void CameraNode::enqueueTree(RenderQueue& queue) const
{
  const SceneNode* root = this;
  while (root->getParent())
    root = root->getParent();

  root->enqueue(queue);
}

float CameraNode::getFOV(void) const
{
  return FOV;
}

float CameraNode::getAspectRatio(void) const
{
  return aspectRatio;
}

void CameraNode::setFOV(float newFOV)
{
  FOV = newFOV;
}

void CameraNode::setAspectRatio(float newAspectRatio)
{
  aspectRatio = newAspectRatio;
}

///////////////////////////////////////////////////////////////////////

const std::string& SpriteNode::getSpriteName(void) const
{
  return spriteName;
}

void SpriteNode::setSpriteName(const std::string& newSpriteName)
{
  spriteName = newSpriteName;
}

void SpriteNode::enqueue(RenderQueue& queue) const
{
  SceneNode::enqueue(queue);

  RenderSprite* sprite = RenderSprite::findInstance(spriteName);
  if (!sprite)
    return;

  sprite->enqueue(queue, getWorldTransform());
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
