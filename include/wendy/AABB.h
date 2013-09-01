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
#ifndef WENDY_AABB_H
#define WENDY_AABB_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

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
  AABB(const vec3& center, const vec3& size);
  /*! Constructor.
   *  @param[in] width The width of the newly constructed bounding box.
   *  @param[in] height The height of the newly constructed bounding box.
   *  @param[in] depth The depth of the newly constructed bounding box.
   *
   *  @remarks The center is placed at the origin.
   */
  AABB(float width, float height, float depth);
  /*! Checks whether this bounding box contains the specified point.
   */
  bool contains(const vec3& point) const;
  /*! Checks whether this bounding box contains the specified bounding box.
   */
  bool contains(const AABB& other) const;
  /*! Checks whether this bounding box intersects the specified bounding box.
   */
  bool intersects(const AABB& other) const;
  /*! Expands this bounding box so as to contain the specified point.
   */
  void envelop(const vec3& point);
  /*! Expands this bounding box so as to contain the specified bounding box.
   */
  void envelop(const AABB& other);
  /*! Ensures that the size of this bounding box uses positive values.
   */
  void normalize();
  /*! Retrieves the minimum and maxiumum bounds of this bounding box.
   */
  void bounds(float& minX, float& minY, float& minZ,
              float& maxX, float& maxY, float& maxZ) const;
  /*! Sets the minimum and maxiumum bounds of this bounding box.
   */
  void setBounds(float minX, float minY, float minZ,
                 float maxX, float maxY, float maxZ);
  /*! Sets the position and size of this bounding box.
   */
  void set(const vec3& newCenter, const vec3& newSize);
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

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_AABB_H*/
///////////////////////////////////////////////////////////////////////
