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
#include <wendy/Vector.h>
#include <wendy/AABB.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

AABB::AABB(void)
{
}

AABB::AABB(const Vec3& initCenter, const Vec3& initSize):
  center(initCenter),
  size(initSize)
{
}

AABB::AABB(float width, float height, float depth):
  center(0.f, 0.f, 0.f),
  size(width, height, depth)
{
}

bool AABB::contains(const Vec3& point) const
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  if (point.x < minX || point.y < minY || point.z < minZ)
    return false;

  if (point.x > maxX || point.y > maxY || point.z > maxZ)
    return false;

  return true;
}

bool AABB::contains(const AABB& other) const
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  float otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ;
  other.getBounds(otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ);

  if (minX > otherMinX || maxX < otherMaxX)
    return false;

  if (minY > otherMinY || maxY < otherMaxY)
    return false;

  if (minZ > otherMinZ || maxZ < otherMaxZ)
    return false;

  return true;
}

bool AABB::intersects(const AABB& other) const
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  float otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ;
  other.getBounds(otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ);

  if (minX > otherMaxX || maxX < otherMinX)
    return false;

  if (minY > otherMaxY || maxY < otherMinY)
    return false;

  if (minZ > otherMaxZ || maxZ < otherMinZ)
    return false;

  return true;
}

void AABB::envelop(const AABB& other)
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  float otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ;
  other.getBounds(otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ);

  if (minX > otherMinX)
    minX = otherMinX;
  if (minY > otherMinY)
    minY = otherMinY;
  if (minZ > otherMinZ)
    minZ = otherMinZ;
  if (maxX < otherMaxX)
    maxX = otherMaxX;
  if (maxY < otherMaxY)
    maxY = otherMaxY;
  if (maxZ < otherMaxZ)
    maxZ = otherMaxZ;

  setBounds(minX, minY, minZ, maxX, maxY, maxZ);
}

void AABB::envelop(const Vec3& point)
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  if (minX > point.x)
    minX = point.x;
  if (minY > point.y)
    minY = point.y;
  if (minZ > point.z)
    minZ = point.z;
  if (maxX < point.x)
    maxX = point.x;
  if (maxY < point.y)
    maxY = point.y;
  if (maxZ < point.z)
    maxZ = point.z;

  setBounds(minX, minY, minZ, maxX, maxY, maxZ);
}

void AABB::normalize(void)
{
  size.x = fabsf(size.x);
  size.y = fabsf(size.y);
  size.z = fabsf(size.z);
}

void AABB::getBounds(float& minX, float& minY, float& minZ,
                     float& maxX, float& maxY, float& maxZ) const
{
  minX = center.x - fabsf(size.x);
  minY = center.y - fabsf(size.y);
  minZ = center.z - fabsf(size.z);
  maxX = center.x + fabsf(size.x);
  maxY = center.y + fabsf(size.y);
  maxZ = center.z + fabsf(size.z);
}

void AABB::setBounds(float minX, float minY, float minZ,
                     float maxX, float maxY, float maxZ)
{
  center.x = (minX + maxX) / 2.f;
  center.y = (minY + maxY) / 2.f;
  center.z = (minZ + maxZ) / 2.f;
  size.set(maxX - minX, maxY - minY, maxZ - minZ);
}

void AABB::set(const Vec3& newCenter, const Vec3& newSize)
{
  center = newCenter;
  size = newSize;
}

void AABB::set(float newWidth, float newHeight, float newDepth)
{
  center.set(0.f, 0.f, 0.f);
  size.set(newWidth, newHeight, newDepth);
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
