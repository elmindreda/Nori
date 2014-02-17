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

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Camera::Camera():
  m_mode(PERSPECTIVE),
  m_FOV(half_pi<float>()),
  m_aspectRatio(4.f / 3.f),
  m_nearZ(0.1f),
  m_farZ(1000.f)
{
}

void Camera::setMode(Mode newMode)
{
  m_mode = newMode;
  updateFrustum();
}

void Camera::setOrthoVolume(const AABB& newVolume)
{
  m_volume = newVolume;
  m_volume.normalize();
  updateFrustum();
}

void Camera::setFOV(float newFOV)
{
  assert(newFOV > 0.f);
  assert(newFOV < pi<float>());

  m_FOV = newFOV;
  updateFrustum();
}

void Camera::setAspectRatio(float newAspectRatio)
{
  assert(newAspectRatio > 0.f);

  m_aspectRatio = newAspectRatio;
  updateFrustum();
}

void Camera::setNearZ(float newNearZ)
{
  assert(newNearZ > 0.f);

  m_nearZ = newNearZ;
  updateFrustum();
}

void Camera::setFarZ(float newFarZ)
{
  assert(newFarZ > 0.f);

  m_farZ = newFarZ;
  updateFrustum();
}

mat4 Camera::projectionMatrix() const
{
  if (m_mode == PERSPECTIVE)
    return perspective(m_FOV, m_aspectRatio, m_nearZ, m_farZ);
  else
  {
    vec3 minimum, maximum;
    m_volume.bounds(minimum, maximum);

    return ortho(minimum.x, maximum.x,
                 minimum.y, maximum.y,
                 minimum.z, maximum.z);
  }
}

void Camera::setTransform(const Transform3& newTransform)
{
  m_transform = newTransform;
  updateFrustum();
  updateInverse();
}

float Camera::normalizedDepth(vec3 point) const
{
  return length(m_inverse * point) / m_farZ;
}

Ray3 Camera::viewSpacePickingRay(vec2 position) const
{
  Ray3 result;

  if (m_mode == ORTHOGRAPHIC)
  {
    result.direction = vec3(0.f, 0.f, -1.f);

    vec3 minimum, maximum;
    m_volume.bounds(minimum, maximum);

    result.origin = vec3(minimum.x + position.x * (maximum.x - minimum.x),
                         minimum.y + position.y * (maximum.y - minimum.y),
                         maximum.z);
  }
  else
  {
    // Figure out the camera space ray direction
    result.direction = normalize(vec3((position.x - 0.5f) * m_aspectRatio,
                                      position.y - 0.5f,
                                      -0.5f / tan(m_FOV / 2.f)));

    // Shift the ray origin along the ray direction to the near plane
    result.origin = result.direction * (m_nearZ / -result.direction.z);
  }

  return result;
}

void Camera::updateFrustum()
{
  if (m_mode == ORTHOGRAPHIC)
    m_frustum.setOrtho(m_volume);
  else
    m_frustum.setPerspective(m_FOV, m_aspectRatio, m_nearZ, m_farZ);

  m_frustum.transformBy(m_transform);
}

void Camera::updateInverse()
{
  m_inverse = m_transform.inverse();
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
