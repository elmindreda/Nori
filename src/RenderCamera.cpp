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
  mode(PERSPECTIVE),
  FOV(90.f),
  aspectRatio(4.f / 3.f),
  nearZ(0.1f),
  farZ(1000.f),
  dirtyFrustum(true),
  dirtyInverse(true)
{
}

bool Camera::isPerspective() const
{
  return mode == PERSPECTIVE;
}

bool Camera::isOrtho() const
{
  return mode == ORTHOGRAPHIC;
}

Camera::Mode Camera::getMode() const
{
  return mode;
}

void Camera::setMode(Mode newMode)
{
  mode = newMode;
  dirtyFrustum = true;
}

const AABB& Camera::getOrthoVolume() const
{
  return volume;
}

void Camera::setOrthoVolume(const AABB& newVolume)
{
  volume = newVolume;
  volume.normalize();
  dirtyFrustum = true;
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
  assert(newFOV > 0.f);
  assert(newFOV < 180.f);

  FOV = newFOV;
  dirtyFrustum = true;
}

void Camera::setAspectRatio(float newAspectRatio)
{
  assert(newAspectRatio > 0.f);

  aspectRatio = newAspectRatio;
  dirtyFrustum = true;
}

void Camera::setNearZ(float newNearZ)
{
  assert(newNearZ > 0.f);

  nearZ = newNearZ;
  dirtyFrustum = true;
}

void Camera::setFarZ(float newFarZ)
{
  assert(newFarZ > 0.f);

  farZ = newFarZ;
  dirtyFrustum = true;
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
}

const Frustum& Camera::getFrustum() const
{
  if (dirtyFrustum)
  {
    if (mode == ORTHOGRAPHIC)
      frustum.setOrtho(volume);
    else
      frustum.setPerspective(FOV, aspectRatio, nearZ, farZ);

    frustum.transformBy(transform);
    dirtyFrustum = false;
  }

  return frustum;
}

float Camera::getNormalizedDepth(const vec3& point) const
{
  vec3 local = point;
  getViewTransform().transformVector(local);
  return length(local) / farZ;
}

Ray3 Camera::getViewSpacePickingRay(const vec2& position) const
{
  Ray3 result;

  if (mode == ORTHOGRAPHIC)
  {
    result.direction = vec3(0.f, 0.f, -1.f);

    float minX, minY, minZ, maxX, maxY, maxZ;
    volume.getBounds(minX, minY, minZ, maxX, maxY, maxZ);

    result.origin = vec3(minX + position.x * (maxX - minX),
                         minY + position.y * (maxY - minY),
                         maxZ);
  }
  else
  {
    // Figure out the camera space ray direction
    result.direction = normalize(vec3((position.x - 0.5f) * aspectRatio,
                                      position.y - 0.5f,
                                      -0.5f / tan(radians(FOV) / 2.f)));

    // Shift the ray origin along the ray direction to the near plane
    result.origin = result.direction * (nearZ / -result.direction.z);
  }

  return result;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
