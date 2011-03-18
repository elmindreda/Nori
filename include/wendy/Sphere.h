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
#ifndef WENDY_SPHERE_H
#define WENDY_SPHERE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class Plane;
class Ray3;
class Transform3;

///////////////////////////////////////////////////////////////////////

class Sphere
{
public:
  Sphere(void);
  Sphere(const vec3& center, float radius);
  void transformBy(const Transform3& transform);
  bool contains(const vec3& point) const;
  bool contains(const Sphere& sphere) const;
  bool intersects(const Sphere& sphere) const;
  bool intersects(const Plane& plane, float& distance) const;
  bool intersects(const Ray3& ray, float& distance) const;
  bool intersects(const Ray3& ray, float& distance, vec3& normal, bool& inside) const;
  void envelop(const vec3& point);
  void envelop(const Sphere& sphere);
  void set(const vec3& newCenter, float newRadius);
  void setDefaults(void);
  vec3 center;
  float radius;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_SPHERE_H*/
///////////////////////////////////////////////////////////////////////
