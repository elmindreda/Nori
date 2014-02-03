///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2013 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <glm/gtx/norm.hpp>
#include <glm/gtx/intersect.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Ray3::Ray3(const vec3& initOrigin, const vec3& initDirection):
  origin(initOrigin),
  direction(initDirection)
{
}

void Ray3::transformBy(const Transform3& transform)
{
  origin = transform * origin;
  direction = transform.rotation * direction;
}

void Ray3::set(const vec3& newOrigin, const vec3& newDirection)
{
  origin = newOrigin;
  direction = newDirection;
}

///////////////////////////////////////////////////////////////////////

Plane::Plane(vec3 initNormal, float initDistance):
  normal(initNormal),
  distance(initDistance)
{
}

Plane::Plane(const vec3& P0, const vec3& P1, const vec3& P2)
{
  set(P0, P1, P2);
}

void Plane::transformBy(const Transform3& transform)
{
  normal = transform.rotation * normal;
  distance += dot(normal, transform.position) * transform.scale;
}

bool Plane::contains(vec3 point) const
{
  return dot(point, normal) < distance;
}

bool Plane::contains(const Sphere& sphere) const
{
  return dot(sphere.center, normal) + sphere.radius < distance;
}

bool Plane::intersects(const Ray3& ray, float& distance) const
{
  // TODO: Use epsilon.

  const float incidence = dot(normal, ray.direction);
  if (incidence == 0.f)
    return false;

  distance = (distance - dot(normal, ray.origin)) / incidence;
  if (distance < 0.f)
    return false;

  return true;
}

bool Plane::intersects(const Ray3& ray, float& distance, vec3& normal, bool& inside) const
{
  // TODO: Use epsilon.

  const float incidence = dot(normal, ray.direction);
  if (incidence == 0.f)
    return false;

  const float difference = distance - dot(normal, ray.origin);
  distance = difference / incidence;
  if (distance < 0.f)
    return false;

  if (incidence < 0.f)
    normal = normal;
  else
    normal = -normal;

  if (difference < 0.f)
    inside = true;
  else
    inside = false;

  return true;
}

void Plane::set(vec3 newNormal, float newDistance)
{
  normal = newNormal;
  distance = newDistance;
}

void Plane::set(const vec3& P0, const vec3& P1, const vec3& P2)
{
  normal = normalize(cross(P1 - P0, P2 - P0));
  distance = dot(normal, P0);
}

///////////////////////////////////////////////////////////////////////

Sphere::Sphere(vec3 initCenter, float initRadius):
  center(initCenter),
  radius(initRadius)
{
}

void Sphere::transformBy(const Transform3& transform)
{
  center += transform.position;
  radius *= transform.scale;
}

bool Sphere::contains(vec3 point) const
{
  return length2(point - center) <= radius * radius;
}

bool Sphere::contains(const Sphere& sphere) const
{
  if (sphere.radius > radius)
    return false;

  vec3 difference = center - sphere.center;

  const float radiusSquared = (radius - sphere.radius) *
                              (radius - sphere.radius);

  if (length2(difference) >= radiusSquared)
    return false;

  return true;
}

bool Sphere::intersects(const Sphere& sphere) const
{
  const float distanceSquared = length2(center - sphere.center);

  return distanceSquared < (radius + sphere.radius) * (radius + sphere.radius);
}

bool Sphere::intersects(const Plane& plane, float& distance) const
{
  const float projection = dot(center, plane.normal);
  const float difference = abs(projection - plane.distance);

  if (difference > radius)
    return false;

  distance = difference;
  return true;
}

bool Sphere::intersects(const Ray3& ray, float& distance) const
{
  return intersectRaySphere(ray.origin, ray.direction,
                            center, radius * radius,
                            distance);
}

void Sphere::envelop(vec3 point)
{
  const vec3 vector = center - point;

  const float lengthSquared = length2(vector);
  if (lengthSquared <= radius * radius)
    return;

  const float length = sqrt(lengthSquared);
  const float distance = (length + radius) / 2.f;

  center = point + vector * distance / length;
  radius = distance;
}

void Sphere::envelop(const Sphere& sphere)
{
  const vec3 difference = sphere.center - center;

  const float radiusSquared = (radius - sphere.radius) *
                              (radius - sphere.radius);
  const float distanceSquared = length2(difference);

  if (distanceSquared < radiusSquared)
  {
    if (sphere.radius > radius)
      operator = (sphere);

    return;
  }

  const float distance = sqrt(distanceSquared);

  center = center + (difference / distance) * (distance - radius);
  radius = (distance + radius + sphere.radius) / 2.f;
}

void Sphere::set(vec3 newCenter, float newRadius)
{
  center = newCenter;
  radius = newRadius;
}

///////////////////////////////////////////////////////////////////////

AABB::AABB(vec3 initCenter, vec3 initSize):
  center(initCenter),
  size(initSize)
{
}

AABB::AABB(float width, float height, float depth):
  size(width, height, depth)
{
}

bool AABB::contains(vec3 point) const
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  bounds(minX, minY, minZ, maxX, maxY, maxZ);

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
  bounds(minX, minY, minZ, maxX, maxY, maxZ);

  float otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ;
  other.bounds(otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ);

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
  bounds(minX, minY, minZ, maxX, maxY, maxZ);

  float otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ;
  other.bounds(otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ);

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
  bounds(minX, minY, minZ, maxX, maxY, maxZ);

  float otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ;
  other.bounds(otherMinX, otherMinY, otherMinZ, otherMaxX, otherMaxY, otherMaxZ);

  minX = min(minX, otherMinX);
  minY = min(minY, otherMinY);
  minZ = min(minZ, otherMinZ);
  maxX = max(maxX, otherMaxX);
  maxY = max(maxY, otherMaxY);
  maxZ = max(maxZ, otherMaxZ);

  setBounds(minX, minY, minZ, maxX, maxY, maxZ);
}

void AABB::envelop(vec3 point)
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  bounds(minX, minY, minZ, maxX, maxY, maxZ);

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

void AABB::bounds(float& minX, float& minY, float& minZ,
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

void AABB::set(vec3 newCenter, vec3 newSize)
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
