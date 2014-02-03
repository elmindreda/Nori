///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#include <wendy/Config.hpp>

#include <wendy/Core.hpp>
#include <wendy/Transform.hpp>
#include <wendy/Primitive.hpp>
#include <wendy/Frustum.hpp>
#include <wendy/Camera.hpp>

#include <glm/gtc/matrix_transform.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Camera::Camera():
  m_mode(PERSPECTIVE),
  m_FOV(90.f),
  m_aspectRatio(4.f / 3.f),
  m_nearZ(0.1f),
  m_farZ(1000.f),
  m_dirtyFrustum(true),
  m_dirtyInverse(true)
{
}

void Camera::setMode(Mode newMode)
{
  m_mode = newMode;
  m_dirtyFrustum = true;
}

void Camera::setOrthoVolume(const AABB& newVolume)
{
  m_volume = newVolume;
  m_volume.normalize();
  m_dirtyFrustum = true;
}

void Camera::setFOV(float newFOV)
{
  assert(newFOV > 0.f);
  assert(newFOV < 180.f);

  m_FOV = newFOV;
  m_dirtyFrustum = true;
}

void Camera::setAspectRatio(float newAspectRatio)
{
  assert(newAspectRatio > 0.f);

  m_aspectRatio = newAspectRatio;
  m_dirtyFrustum = true;
}

void Camera::setNearZ(float newNearZ)
{
  assert(newNearZ > 0.f);

  m_nearZ = newNearZ;
  m_dirtyFrustum = true;
}

void Camera::setFarZ(float newFarZ)
{
  assert(newFarZ > 0.f);

  m_farZ = newFarZ;
  m_dirtyFrustum = true;
}

const Transform3& Camera::viewTransform() const
{
  if (m_dirtyInverse)
  {
    m_inverse = m_transform.inverse();
    m_dirtyInverse = false;
  }

  return m_inverse;
}

mat4 Camera::projectionMatrix() const
{
  if (m_mode == PERSPECTIVE)
    return perspective(m_FOV, m_aspectRatio, m_nearZ, m_farZ);
  else
  {
    float minX, minY, minZ, maxX, maxY, maxZ;
    m_volume.bounds(minX, minY, minZ, maxX, maxY, maxZ);
    return ortho(minX, maxX, minY, maxY, minZ, maxZ);
  }
}

void Camera::setTransform(const Transform3& newTransform)
{
  m_transform = newTransform;
  m_dirtyFrustum = true;
  m_dirtyInverse = true;
}

const Frustum& Camera::frustum() const
{
  if (m_dirtyFrustum)
  {
    if (m_mode == ORTHOGRAPHIC)
      m_frustum.setOrtho(m_volume);
    else
      m_frustum.setPerspective(m_FOV, m_aspectRatio, m_nearZ, m_farZ);

    m_frustum.transformBy(m_transform);
    m_dirtyFrustum = false;
  }

  return m_frustum;
}

float Camera::normalizedDepth(const vec3& point) const
{
  return length(viewTransform() * point) / m_farZ;
}

Ray3 Camera::viewSpacePickingRay(const vec2& position) const
{
  Ray3 result;

  if (m_mode == ORTHOGRAPHIC)
  {
    result.direction = vec3(0.f, 0.f, -1.f);

    float minX, minY, minZ, maxX, maxY, maxZ;
    m_volume.bounds(minX, minY, minZ, maxX, maxY, maxZ);

    result.origin = vec3(minX + position.x * (maxX - minX),
                         minY + position.y * (maxY - minY),
                         maxZ);
  }
  else
  {
    // Figure out the camera space ray direction
    result.direction = normalize(vec3((position.x - 0.5f) * m_aspectRatio,
                                      position.y - 0.5f,
                                      -0.5f / tan(radians(m_FOV) / 2.f)));

    // Shift the ray origin along the ray direction to the near plane
    result.origin = result.direction * (m_nearZ / -result.direction.z);
  }

  return result;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
