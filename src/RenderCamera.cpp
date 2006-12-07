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
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderCamera.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Camera::Camera(const String& name):
  Managed<Camera>(name),
  FOV(90.f),
  aspectRatio(4.f / 3.f),
  minDepth(0.01f),
  maxDepth(1000.f),
  dirtyFrustum(true),
  dirtyInverse(true)
{
}

void Camera::begin(void) const
{
  if (current)
    throw Exception("Cannot nest cameras");

  GL::Renderer* renderer = GL::Renderer::get();
  if (!renderer)
  {
    Log::writeError("Cannot make camera current without a renderer");
    return;
  }

  renderer->begin3D(FOV, aspectRatio);
  renderer->pushTransform(getInverseTransform());

  current = const_cast<Camera*>(this);
}

void Camera::end(void) const
{
  if (current != this)
    throw Exception("No current camera or camera invalidated during rendering");

  GL::Renderer* renderer = GL::Renderer::get();
  if (!renderer)
  {
    Log::writeError("Cannot make camera current without a renderer");
    return;
  }

  renderer->popTransform();
  renderer->end();

  current = NULL;
}

float Camera::getFOV(void) const
{
  return FOV;
}

float Camera::getAspectRatio(void) const
{
  return aspectRatio;
}

float Camera::getMinDepth(void) const
{
  return minDepth;
}

float Camera::getMaxDepth(void) const
{
  return maxDepth;
}

void Camera::setFOV(float newFOV)
{
  if (current == this)
    throw Exception("Cannot change properties on an active camera");

  FOV = newFOV;
  dirtyFrustum = true;
}

void Camera::setAspectRatio(float newAspectRatio)
{
  if (current == this)
    throw Exception("Cannot change properties on an active camera");

  aspectRatio = newAspectRatio;
  dirtyFrustum = true;
}

void Camera::setDepthRange(float newMinDepth, float newMaxDepth)
{
  if (current == this)
    throw Exception("Cannot change properties on an active camera");

  minDepth = newMinDepth;
  maxDepth = newMaxDepth;
  dirtyFrustum = true;
}

const Transform3& Camera::getTransform(void) const
{
  return transform;
}

const Transform3& Camera::getInverseTransform(void) const
{
  if (dirtyInverse)
  {
    inverse = transform;
    inverse.invert();
    dirtyInverse = false;
  }

  return inverse;
}

void Camera::setTransform(const Transform3& newTransform)
{
  transform = newTransform;
  dirtyFrustum = true;
  dirtyInverse = true;
}

const Frustum& Camera::getFrustum(void) const
{
  if (dirtyFrustum)
  {
    frustum.set(FOV, aspectRatio, -maxDepth);
    frustum.transformBy(transform);
    dirtyFrustum = false;
  }

  return frustum;
}

Camera* Camera::getCurrent(void)
{
  return current;
}

Camera* Camera::current = NULL;

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////