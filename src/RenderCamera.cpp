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

#include <wendy/Config.h>

#include <wendy/RenderCamera.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

Camera::Camera(void):
  FOV(90.f),
  aspectRatio(4.f / 3.f),
  minDepth(0.01f),
  maxDepth(1000.f),
  dirtyFrustum(true),
  dirtyInverse(true),
  dirtyViewDir(true)
{
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
  FOV = newFOV;
  dirtyFrustum = true;
}

void Camera::setAspectRatio(float newAspectRatio)
{
  aspectRatio = newAspectRatio;
  dirtyFrustum = true;
}

void Camera::setDepthRange(float newMinDepth, float newMaxDepth)
{
  minDepth = newMinDepth;
  maxDepth = newMaxDepth;
  dirtyFrustum = true;
  dirtyViewDir = true;
}

const Transform3& Camera::getTransform(void) const
{
  return transform;
}

const Transform3& Camera::getViewTransform(void) const
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
  dirtyViewDir = true;
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

float Camera::getNormalizedDepth(const Vec3& point) const
{
  if (dirtyViewDir)
  {
    direction.set(0.f, 0.f, -1.f);
    getViewTransform().rotateVector(direction);
    dirFactor = 1.f / (maxDepth - minDepth);
    dirOffset = (direction.dot(transform.position) + minDepth) * dirFactor;
    dirtyViewDir = false;
  }

  return point.dot(direction) * dirFactor + dirOffset;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
