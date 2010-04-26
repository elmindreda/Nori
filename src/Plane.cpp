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
#include <wendy/Sphere.h>
#include <wendy/Plane.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Plane::Plane(void):
  normal(0.f, 1.f, 0.f),
  distance(0.f)
{
}

Plane::Plane(const Vec3& newNormal, float newDistance):
  normal(newNormal),
  distance(newDistance)
{
}

Plane::Plane(const Vec3& P0, const Vec3& P1, const Vec3& P2)
{
  set(P0, P1, P2);
}

void Plane::transformBy(const Transform3& transform)
{
  transform.rotateVector(normal);
  distance += normal.dot(transform.position);
}

bool Plane::contains(const Vec3& point) const
{
  return point.dot(normal) < distance;
}

bool Plane::contains(const Sphere& sphere) const
{
  return sphere.center.dot(normal) + sphere.radius < distance;
}

bool Plane::intersects(const Ray3& ray, float& distance) const
{
  // TODO: Use epsilon.

  const float incidence = normal.dot(ray.direction);
  if (incidence == 0.f)
    return false;

  distance = (distance - normal.dot(ray.origin)) / incidence;
  if (distance < 0.f)
    return false;

  return true;
}

bool Plane::intersects(const Ray3& ray, float& distance, Vec3& normal, bool& inside) const
{
  // TODO: Use epsilon.

  const float incidence = normal.dot(ray.direction);
  if (incidence == 0.f)
    return false;

  const float difference = distance - normal.dot(ray.origin);
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

void Plane::set(const Vec3& newNormal, float newDistance)
{
  normal = newNormal;
  distance = newDistance;
}

void Plane::set(const Vec3& P0, const Vec3& P1, const Vec3& P2)
{
  normal = (P1 - P0).cross(P2 - P0).normalized();

  distance = normal.dot(P0);
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
