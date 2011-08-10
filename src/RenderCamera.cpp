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

Camera::Camera():
  FOV(90.f),
  aspectRatio(4.f / 3.f),
  nearZ(0.1f),
  farZ(1000.f),
  dirtyFrustum(true),
  dirtyInverse(true),
  dirtyViewDir(true)
{
}

float Camera::getFOV() const
{
  return FOV;
}

float Camera::getAspectRatio() const
{
  return aspectRatio;
}

float Camera::getNearZ() const
{
  return nearZ;
}

float Camera::getFarZ() const
{
  return farZ;
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

void Camera::setNearZ(float newNearZ)
{
  nearZ = newNearZ;
  dirtyFrustum = true;
  dirtyViewDir = true;
}

void Camera::setFarZ(float newFarZ)
{
  farZ = newFarZ;
  dirtyFrustum = true;
  dirtyViewDir = true;
}

const Transform3& Camera::getTransform() const
{
  return transform;
}

const Transform3& Camera::getViewTransform() const
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

const Frustum& Camera::getFrustum() const
{
  if (dirtyFrustum)
  {
    frustum.setPerspective(FOV, aspectRatio, -nearZ, -farZ);
    frustum.transformBy(transform);
    dirtyFrustum = false;
  }

  return frustum;
}

float Camera::getNormalizedDepth(const vec3& point) const
{
  /*
  if (dirtyViewDir)
  {
    direction = vec3(0.f, 0.f, -1.f);
    transform.rotateVector(direction);
    dirFactor = 1.f / (farZ - nearZ);
    dirOffset = dot(direction, transform.position) + nearZ * dirFactor;
    dirtyViewDir = false;
  }

  return dot(point, direction) * dirFactor + dirOffset;
  */
  vec3 local = point;
  getViewTransform().transformVector(local);
  return length(local) / farZ;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
