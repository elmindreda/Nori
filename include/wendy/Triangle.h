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
#ifndef WENDY_TRIANGLE_H
#define WENDY_TRIANGLE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class Plane;
class Ray3;

///////////////////////////////////////////////////////////////////////

/*! @brief Generic 2D triangle.
 */
class Triangle2
{
public:
  Triangle2();
  Triangle2(const vec2& P0, const vec2& P1, const vec2& P2);
  vec2 center() const;
  bool contains(const vec2& point) const;
  void set(const vec2& P0, const vec2& P1, const vec2& P2);
  vec2 P[3];
};

///////////////////////////////////////////////////////////////////////

/*! @brief Generic 3D triangle.
 */
class Triangle3
{
public:
  Triangle3();
  Triangle3(const vec3& P0, const vec3& P1, const vec3& P2);
  vec3 center() const;
  bool intersects(const Plane& plane) const;
  bool intersects(const Ray3& ray, float& distance) const;
  bool intersects(const Ray3& ray, float& distance, vec3& normal, bool& inside) const;
  void set(const vec3& P0, const vec3& P1, const vec3& P2);
  vec3 P[3];
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_TRIANGLE_H*/
///////////////////////////////////////////////////////////////////////
