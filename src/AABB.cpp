///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/AABB.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

AABB::AABB()
{
}

AABB::AABB(const vec3& initCenter, const vec3& initSize):
  center(initCenter),
  size(initSize)
{
}

AABB::AABB(float width, float height, float depth):
  size(width, height, depth)
{
}

bool AABB::contains(const vec3& point) const
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  if (point.x < minX || point.y < minY || point.z < minZ ||
      point.x > maxX || point.y > maxY || point.z > maxZ)
  {
    return false;
  }

  return true;
}

bool AABB::contains(const AABB& other) const
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  float otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ;
  other.getBounds(otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ);

  if (minX > otherMinX || maxX < otherMaxX ||
      minY > otherMinY || maxY < otherMaxY ||
      minZ > otherMinZ || maxZ < otherMaxZ)
  {
    return false;
  }

  return true;
}

bool AABB::intersects(const AABB& other) const
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  float otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ;
  other.getBounds(otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ);

  if (minX > otherMaxX || maxX < otherMinX ||
      minY > otherMaxY || maxY < otherMinY ||
      minZ > otherMaxZ || maxZ < otherMinZ)
  {
    return false;
  }

  return true;
}

void AABB::envelop(const AABB& other)
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  float otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ;
  other.getBounds(otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ);

  minX = min(minX, otherMinX);
  minY = min(minY, otherMinY);
  minZ = min(minZ, otherMinZ);
  maxX = max(maxX, otherMaxX);
  maxY = max(maxY, otherMaxY);
  maxZ = max(maxZ, otherMaxZ);

  setBounds(minX, minY, minZ, maxX, maxY, maxZ);
}

void AABB::envelop(const vec3& point)
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  minX = min(minX, point.x);
  minY = min(minY, point.y);
  minZ = min(minZ, point.z);
  maxX = max(maxX, point.x);
  maxY = max(maxY, point.y);
  maxZ = max(maxZ, point.z);

  setBounds(minX, minY, minZ, maxX, maxY, maxZ);
}

void AABB::normalize()
{
  size = abs(size);
}

void AABB::getBounds(float& minX, float& minY, float& minZ,
                     float& maxX, float& maxY, float& maxZ) const
{
  minX = center.x - abs(size.x);
  minY = center.y - abs(size.y);
  minZ = center.z - abs(size.z);
  maxX = center.x + abs(size.x);
  maxY = center.y + abs(size.y);
  maxZ = center.z + abs(size.z);
}

void AABB::setBounds(float minX, float minY, float minZ,
                     float maxX, float maxY, float maxZ)
{
  center = (vec3(minX, minY, minZ) + vec3(maxX, maxY, maxZ)) / 2.f;
  size = vec3(maxX - minX, maxY - minY, maxZ - minZ);
}

void AABB::set(const vec3& newCenter, const vec3& newSize)
{
  center = newCenter;
  size = newSize;
}

void AABB::set(float newWidth, float newHeight, float newDepth)
{
  center = vec3(0.f);
  size = vec3(newWidth, newHeight, newDepth);
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
