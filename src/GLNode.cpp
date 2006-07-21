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

#include <moira/Config.h>
#include <moira/Portability.h>
#include <moira/Core.h>
#include <moira/Signal.h>
#include <moira/Node.h>
#include <moira/Color.h>
#include <moira/Vector.h>
#include <moira/Matrix.h>
#include <moira/Quaternion.h>
#include <moira/Transform.h>
#include <moira/Bezier.h>
#include <moira/Stream.h>
#include <moira/Resource.h>
#include <moira/Image.h>
#include <moira/XML.h>
#include <moira/Mesh.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLLight.h>
#include <wendy/GLRender.h>
#include <wendy/GLSprite.h>
#include <wendy/GLMesh.h>
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

CameraNode::CameraNode(void):
  FOV(90.f),
  aspectRatio(0.f)
{
}

void CameraNode::prepareTree(void)
{
  getRoot()->prepare();
}

void CameraNode::renderTree(void) const
{
  RenderQueue queue;
  enqueueTree(queue);

  Canvas* canvas = Canvas::getCurrent();

  if (aspectRatio)
    canvas->begin3D(FOV, aspectRatio);
  else
    canvas->begin3D(FOV, (float) canvas->getPhysicalWidth() /
                         (float) canvas->getPhysicalHeight());

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
  getRoot()->enqueue(queue);
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

  RenderStyle* style = RenderStyle::findInstance(styleName);
  if (!style)
  {
    Log::writeError("Render style %s not found", styleName.c_str());
    return;
  }

  Sprite3 sprite;
  sprite.size = spriteSize;
  sprite.enqueue(queue, getWorldTransform(), *style);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
