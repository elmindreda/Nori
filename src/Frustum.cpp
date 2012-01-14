///////////////////////////////////////////////////////////////////////
// Wendy core library
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

#include <wendy/Core.h>
#include <wendy/Transform.h>
#include <wendy/Plane.h>
#include <wendy/Sphere.h>
#include <wendy/AABB.h>
#include <wendy/Frustum.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Frustum::Frustum()
{
}

Frustum::Frustum(float FOV, float aspectRatio, float nearZ, float farZ)
{
  setPerspective(FOV, aspectRatio, nearZ, farZ);
}

bool Frustum::contains(const vec3& point) const
{
  for (size_t i = 0;  i < 6;  i++)
  {
    if (!planes[i].contains(point))
      return false;
  }

  return true;
}

bool Frustum::contains(const Sphere& sphere) const
{
  for (size_t i = 0;  i < 6;  i++)
  {
    if (!planes[i].contains(sphere))
      return false;
  }

  return true;
}

bool Frustum::contains(const AABB& box) const
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  box.getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  for (size_t i = 0;  i < 6;  i++)
  {
    const vec3 positive((planes[i].normal.x < 0.f) ? minX : maxX,
                        (planes[i].normal.y < 0.f) ? minY : maxY,
                        (planes[i].normal.z < 0.f) ? minZ : maxZ);

    if (!planes[i].contains(positive))
      return false;
  }

  return true;
}

bool Frustum::intersects(const Sphere& sphere) const
{
  for (size_t i = 0;  i < 6;  i++)
  {
    if (dot(planes[i].normal, sphere.center) - sphere.radius > planes[i].distance)
      return false;
  }

  return true;
}

bool Frustum::intersects(const AABB& box) const
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  box.getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  for (size_t i = 0;  i < 6;  i++)
  {
    const vec3 negative(planes[i].normal.x < 0.f ? maxX : minX,
                        planes[i].normal.y < 0.f ? maxY : minY,
                        planes[i].normal.z < 0.f ? maxZ : minZ);

    if (!planes[i].contains(negative))
      return false;
  }

  return true;
}

void Frustum::transformBy(const Transform3& transform)
{
  for (size_t i = 0;  i < 6;  i++)
    planes[i].transformBy(transform);
}

void Frustum::setPerspective(float FOV, float aspectRatio, float nearZ, float farZ)
{
  assert(FOV > 0.f);
  assert(FOV < 180.f);
  assert(aspectRatio > 0.f);
  assert(nearZ > 0.f);
  assert(farZ > 0.f);
  assert(farZ > nearZ);

  const float distance = 0.5f / tan(radians(FOV) / 2.f);

  vec3 points[5];
  points[0] = vec3(0.f);
  points[1] = vec3(-0.5f * aspectRatio,  0.5f, -distance);
  points[2] = vec3( 0.5f * aspectRatio,  0.5f, -distance);
  points[3] = vec3( 0.5f * aspectRatio, -0.5f, -distance);
  points[4] = vec3(-0.5f * aspectRatio, -0.5f, -distance);

  for (size_t i = 0;  i < 4;  i++)
    planes[i].set(points[0], points[(i + 1) % 4 + 1], points[i + 1]);

  planes[FRUSTUM_NEAR].set(vec3(0.f, 0.f, 1.f), -nearZ);
  planes[FRUSTUM_FAR].set(vec3(0.f, 0.f, -1.f), farZ);
}

void Frustum::setOrtho(const AABB& volume)
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  volume.getBounds(minX, minY, minZ, maxX, maxY, maxZ);
  setOrtho(minX, minY, minZ, maxX, maxY, maxZ);
}

void Frustum::setOrtho(float minX, float minY, float minZ,
                       float maxX, float maxY, float maxZ)
{
  assert(minX < maxX);
  assert(minY < maxY);
  assert(minZ < maxZ);

  planes[FRUSTUM_TOP].set(vec3(0.f, 1.f, 0.f), maxY);
  planes[FRUSTUM_RIGHT].set(vec3(1.f, 0.f, 0.f), maxX);
  planes[FRUSTUM_BOTTOM].set(vec3(0.f, -1.f, 0.f), -minY);
  planes[FRUSTUM_LEFT].set(vec3(-1.f, 0.f, 0.f), -minX);
  planes[FRUSTUM_NEAR].set(vec3(0.f, 0.f, 1.f), maxZ);
  planes[FRUSTUM_FAR].set(vec3(0.f, 0.f, -1.f), -minZ);
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
