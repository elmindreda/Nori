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
#ifndef WENDY_PLANE_H
#define WENDY_PLANE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class Ray3;
class Sphere;
class Transform3;

///////////////////////////////////////////////////////////////////////

class Plane
{
public:
  Plane();
  Plane(const vec3& initNormal, float initDistance);
  Plane(const vec3& P0, const vec3& P1, const vec3& P2);
  void transformBy(const Transform3& transform);
  bool contains(const vec3& point) const;
  bool contains(const Sphere& sphere) const;
  bool intersects(const Ray3& ray, float& distance) const;
  bool intersects(const Ray3& ray, float& distance, vec3& normal, bool& inside) const;
  void set(const vec3& newPosition, float newDistance);
  void set(const vec3& P0, const vec3& P1, const vec3& P2);
  vec3 normal;
  float distance;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_PLANE_H*/
///////////////////////////////////////////////////////////////////////
