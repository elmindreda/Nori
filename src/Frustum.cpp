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
#include <wendy/Plane.h>
#include <wendy/Sphere.h>
#include <wendy/AABB.h>
#include <wendy/Frustum.h>

#include <cmath>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Frustum::Frustum(void)
{
}

Frustum::Frustum(float FOV, float aspectRatio, float farZ)
{
  set(FOV, aspectRatio, farZ);
}

bool Frustum::contains(const Vec3& point) const
{
  for (PlaneList::const_iterator i = planes.begin();  i != planes.end();  i++)
  {
    if (!(*i).contains(point))
      return false;
  }

  return true;
}

bool Frustum::contains(const Sphere& sphere) const
{
  for (PlaneList::const_iterator i = planes.begin();  i != planes.end();  i++)
  {
    if (!(*i).contains(sphere))
      return false;
  }

  return true;
}

bool Frustum::contains(const AABB& box) const
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  box.getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  for (PlaneList::const_iterator i = planes.begin();  i != planes.end();  i++)
  {
    const Plane& plane = *i;

    const Vec3 negative(plane.normal.x < 0.f ? maxX : minX,
                        plane.normal.y < 0.f ? maxY : minY,
                        plane.normal.z < 0.f ? maxZ : minZ);

    if (!plane.contains(negative))
      return false;
  }

  return true;
}

bool Frustum::intersects(const Sphere& sphere) const
{
  for (PlaneList::const_iterator i = planes.begin();  i != planes.end();  i++)
  {
    const Plane& plane = *i;

    if (plane.normal.dot(sphere.center) - sphere.radius > plane.distance)
      return false;
  }

  return true;
}

bool Frustum::intersects(const AABB& box) const
{
  float minX, minY, minZ, maxX, maxY, maxZ;
  box.getBounds(minX, minY, minZ, maxX, maxY, maxZ);

  for (PlaneList::const_iterator i = planes.begin();  i != planes.end();  i++)
  {
    const Plane& plane = *i;

    const Vec3 positive(plane.normal.x < 0.f ? minX : maxX,
                        plane.normal.y < 0.f ? minY : maxY,
                        plane.normal.z < 0.f ? minZ : maxZ);

    if (!plane.contains(positive))
      return false;
  }

  return true;
}

void Frustum::transformBy(const Transform3& transform)
{
  for (PlaneList::iterator i = planes.begin();  i != planes.end();  i++)
    (*i).transformBy(transform);
}

void Frustum::set(float FOV, float aspectRatio, float farZ)
{
  planes.clear();

  const float radians = FOV * (float) M_PI / 180.f;
  const float distance = 0.5f / tanf(radians / 2.f);

  float sign;

  if (farZ > 0.f)
    sign = 1.f;
  else
    sign = -1.f;

  Vec3 points[5];
  points[0].set(0.f, 0.f, 0.f);
  points[1].set(-0.5f * aspectRatio, 0.5f, sign * distance);
  points[2].set(0.5f * aspectRatio, 0.5f, sign * distance);
  points[3].set(0.5f * aspectRatio, -0.5f, sign * distance);
  points[4].set(-0.5f * aspectRatio, -0.5f, sign * distance);

  for (unsigned int i = 0;  i < 4;  i++)
    planes.push_back(Plane(points[0], points[(i + 1) % 4 + 1], points[i + 1]));

  planes.push_back(Plane(Vec3(0.f, 0.f, sign), -farZ));
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
