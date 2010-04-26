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
#include <wendy/Vector.h>
#include <wendy/Matrix.h>
#include <wendy/Quaternion.h>
#include <wendy/Transform.h>
#include <wendy/Ray.h>
#include <wendy/Plane.h>
#include <wendy/Sphere.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Sphere::Sphere(void)
{
  setDefaults();
}

Sphere::Sphere(const Vec3& initCenter, float initRadius):
  center(initCenter),
  radius(initRadius)
{
}

void Sphere::transformBy(const Transform3& transform)
{
  transform.translateVector(center);
}

bool Sphere::contains(const Vec3& point) const
{
  return (point - center).lengthSquared() <= radius * radius;
}

bool Sphere::contains(const Sphere& sphere) const
{
  if (sphere.radius > radius)
    return false;

  Vec3 difference = center - sphere.center;

  const float radiusSquared = (radius - sphere.radius) *
                              (radius - sphere.radius);

  if (difference.lengthSquared() >= radiusSquared)
    return false;

  return true;
}

bool Sphere::intersects(const Sphere& sphere) const
{
  const float distanceSquared = (center - sphere.center).lengthSquared();

  return distanceSquared < (radius + sphere.radius) * (radius + sphere.radius);
}

bool Sphere::intersects(const Plane& plane, float& distance) const
{
  const float projection = center.dot(plane.normal);
  const float difference = fabsf(projection - plane.distance);

  if (difference > radius)
    return false;

  distance = difference;
  return true;
}

bool Sphere::intersects(const Ray3& ray, float& distance) const
{
  const Vec3 difference = ray.origin - center;
  const float B = 2.f * ray.direction.dot(difference);
  const float C = difference.lengthSquared() - radius * radius;
  const float D = B * B - 4.f * C;

  if (D < 0.f)
    return false;

  const float E = sqrtf(D) / 2.f;

  distance = -B / 2.f - E;
  if (distance < 0.f)
  {
    distance = -B / 2.f + E;
    if (distance < 0.f)
      return false;
  }

  return true;
}

bool Sphere::intersects(const Ray3& ray,
                        float& distance,
			Vec3& normal,
			bool& inside) const
{
  const Vec3 difference = ray.origin - center;
  const float B = 2.f * ray.direction.dot(difference);
  const float C = difference.lengthSquared() - radius * radius;
  const float D = B * B - 4.f * C;

  if (D < 0.f)
    return false;

  const float E = sqrtf(D) / 2.f;

  distance = -B / 2.f - E;
  if (distance < 0.f)
  {
    distance = -B / 2.f + E;
    if (distance < 0.f)
      return false;

    normal = (center - ray.origin - ray.direction * distance) / radius;
    inside = true;
  }
  else
  {
    normal = (ray.origin + ray.direction * distance - center) / radius;
    inside = false;
  }

  return true;
}

void Sphere::envelop(const Vec3& point)
{
  Vec3 vector = center - point;

  const float lengthSquared = vector.lengthSquared();
  if (lengthSquared <= radius * radius)
    return;

  const float length = sqrtf(lengthSquared);
  const float distance = (length + radius) / 2.f;

  center = point + vector * distance / length;
  radius = distance;
}

void Sphere::envelop(const Sphere& sphere)
{
  Vec3 difference = sphere.center - center;

  const float radiusSquared = (radius - sphere.radius) *
                              (radius - sphere.radius);
  const float distanceSquared = difference.lengthSquared();

  if (distanceSquared < radiusSquared)
  {
    if (sphere.radius > radius)
      operator = (sphere);

    return;
  }

  const float distance = sqrtf(distanceSquared);

  center = center + (difference / distance) * (distance - radius);
  radius = (distance + radius + sphere.radius) / 2.f;
}

void Sphere::set(const Vec3& newCenter, float newRadius)
{
  center = newCenter;
  radius = newRadius;
}

void Sphere::setDefaults(void)
{
  center.set(0.f, 0.f, 0.f);
  radius = 0.5f;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
