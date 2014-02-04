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

void Ray3::set(const vec3& newOrigin, const vec3& newDirection)
{
  origin = newOrigin;
  direction = newDirection;
}

Ray3 operator * (const Transform3& transform, const Ray3& ray)
{
  return Ray3(transform * ray.origin, transform.rotation * ray.direction);
}

///////////////////////////////////////////////////////////////////////

Plane::Plane(const vec3& P0, const vec3& P1, const vec3& P2)
{
  set(P0, P1, P2);
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

Plane operator * (const Transform3& transform, const Plane& plane)
{
  const vec3 normal(transform.rotation * plane.normal);
  const float offset = dot(normal, transform.position) * transform.scale;
  return Plane(normal, plane.distance + offset);
}

///////////////////////////////////////////////////////////////////////

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

bool Sphere::intersects(const Plane& plane) const
{
  const float projection = dot(center, plane.normal);
  const float difference = abs(projection - plane.distance);

  return difference < radius;
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

Sphere operator * (const Transform3& transform, const Sphere& sphere)
{
  return Sphere(transform * sphere.center, transform.scale * sphere.radius);
}

///////////////////////////////////////////////////////////////////////

bool AABB::contains(vec3 point) const
{
  vec3 minimum, maximum;
  bounds(minimum, maximum);

  return all(lessThanEqual(minimum, point)) && all(greaterThan(maximum, point));
}

bool AABB::contains(const AABB& other) const
{
  vec3 minimum, maximum;
  bounds(minimum, maximum);

  vec3 otherMin, otherMax;
  other.bounds(otherMin, otherMax);

  return all(lessThanEqual(minimum, otherMin)) &&
         all(greaterThan(maximum, otherMax));
}

bool AABB::intersects(const AABB& other) const
{
  vec3 minimum, maximum;
  bounds(minimum, maximum);

  vec3 otherMin, otherMax;
  other.bounds(otherMin, otherMax);

  return all(lessThanEqual(minimum, otherMax)) &&
         all(greaterThan(maximum, otherMin));
}

void AABB::envelop(const AABB& other)
{
  vec3 minimum, maximum;
  bounds(minimum, maximum);

  vec3 otherMin, otherMax;
  other.bounds(otherMin, otherMax);

  setBounds(min(minimum, otherMin), max(maximum, otherMax));
}

void AABB::envelop(vec3 point)
{
  vec3 minimum, maximum;
  bounds(minimum, maximum);

  minimum = min(minimum, point);
  maximum = max(maximum, point);

  setBounds(minimum, maximum);
}

void AABB::normalize()
{
  size = abs(size);
}

void AABB::bounds(vec3& minimum, vec3& maximum) const
{
  const vec3 half(abs(size / 2.f));
  minimum = center - half;
  maximum = center + half;
}

void AABB::setBounds(vec3 minimum, vec3 maximum)
{
  center = (minimum + maximum) / 2.f;
  size = maximum - minimum;
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
