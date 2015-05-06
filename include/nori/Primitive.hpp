///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

#pragma once

namespace nori
{

class Transform3;
class Sphere;

/*! Simple 3D ray.
 */
class Ray3
{
public:
  /*! Default constructor.
   */
  Ray3() { }
  /*! Constructor.
   *  @param[in] origin The initial origin.
   *  @param[in] direction The initial direction.
   */
  Ray3(const vec3& origin, const vec3& direction):
    origin(origin),
    direction(direction)
  {
  }
  /*! Sets the values of this ray.
   *  @param[in] newOrigin The desired origin.
   *  @param[in] newDirection The desired direction.
   */
  void set(const vec3& newOrigin, const vec3& newDirection);
  /*! The origin of this ray.
   */
  vec3 origin;
  /*! The direction of this ray.
   */
  vec3 direction;
};

Ray3 operator * (const Transform3& transform, const Ray3& ray);

class Plane
{
public:
  Plane():
    normal(0.f, 1.f, 0.f),
    distance(0.f)
  {
  }
  Plane(vec3 normal, float distance):
    normal(normal),
    distance(distance)
  {
  }
  Plane(const vec3& P0, const vec3& P1, const vec3& P2);
  bool contains(vec3 point) const;
  bool contains(const Sphere& sphere) const;
  bool intersects(const Ray3& ray, float& distance) const;
  bool intersects(const Ray3& ray, float& distance, vec3& normal, bool& inside) const;
  void set(vec3 newPosition, float newDistance);
  void set(const vec3& P0, const vec3& P1, const vec3& P2);
  vec3 normal;
  float distance;
};

Plane operator * (const Transform3& transform, const Plane& plane);

class Sphere
{
public:
  Sphere():
    center(0.f),
    radius(0.f)
  {
  }
  Sphere(vec3 center, float radius):
    center(center),
    radius(radius)
  {
  }
  bool contains(vec3 point) const;
  bool contains(const Sphere& sphere) const;
  bool intersects(const Sphere& sphere) const;
  bool intersects(const Plane& plane) const;
  bool intersects(const Ray3& ray, float& distance) const;
  void envelop(vec3 point);
  void envelop(const Sphere& sphere);
  void set(vec3 newCenter, float newRadius);
  vec3 center;
  float radius;
};

Sphere operator * (const Transform3& transform, const Sphere& sphere);

/*! @brief Axis-aligned bounding box.
 */
class AABB
{
public:
  /*! Constructor.
   *
   *  @remarks All values are initialized to zero.
   */
  AABB() { }
  /*! Constructor.
   *  @param[in] center The center of the newly constructed bounding box.
   *  @param[in] size The size of the newly constructed bounding box.
   */
  AABB(vec3 center, vec3 size):
    center(center),
    size(size)
  {
  }
  /*! Constructor.
   *  @param[in] width The width of the newly constructed bounding box.
   *  @param[in] height The height of the newly constructed bounding box.
   *  @param[in] depth The depth of the newly constructed bounding box.
   *
   *  @remarks The center is placed at the origin.
   */
  AABB(float width, float height, float depth):
    size(width, height, depth)
  {
  }
  /*! Checks whether this bounding box contains the specified point.
   */
  bool contains(vec3 point) const;
  /*! Checks whether this bounding box contains the specified bounding box.
   */
  bool contains(const AABB& other) const;
  /*! Checks whether this bounding box intersects the specified bounding box.
   */
  bool intersects(const AABB& other) const;
  /*! Expands this bounding box so as to contain the specified point.
   */
  void envelop(vec3 point);
  /*! Expands this bounding box so as to contain the specified bounding box.
   */
  void envelop(const AABB& other);
  /*! Ensures that the size of this bounding box uses positive values.
   */
  void normalize();
  /*! Retrieves the minimum and maxiumum bounds of this bounding box.
   */
  void bounds(vec3& minimum, vec3& maximum) const;
  /*! Sets the minimum and maxiumum bounds of this bounding box.
   */
  void setBounds(vec3 minimum, vec3 maximum);
  /*! Sets the position and size of this bounding box.
   */
  void set(vec3 newCenter, vec3 newSize);
  /*! Sets the size of this bounding box.
   */
  void set(float newWidth, float newHeight, float newDepth);
  /*! The center of this bounding box.
   */
  vec3 center;
  /*! The size of this bounding box.
   */
  vec3 size;
};

} /*namespace nori*/

