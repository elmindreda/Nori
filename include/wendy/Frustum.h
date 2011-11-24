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
#ifndef WENDY_FRUSTUM_H
#define WENDY_FRUSTUM_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class AABB;
class Sphere;

///////////////////////////////////////////////////////////////////////

/*! @brief Frustum plane enumeration.
 */
enum FrustumPlane
{
  FRUSTUM_TOP,
  FRUSTUM_RIGHT,
  FRUSTUM_BOTTOM,
  FRUSTUM_LEFT,
  FRUSTUM_NEAR,
  FRUSTUM_FAR,
};

///////////////////////////////////////////////////////////////////////

/*! @brief 3D view frustum.
 */
class Frustum
{
public:
  /*! Constructor.
   */
  Frustum();
  /*! Constructor.
   */
  Frustum(float FOV, float aspectRatio, float nearZ, float farZ);
  /*! Checks whether this frustum contains the specified point.
   */
  bool contains(const vec3& point) const;
  /*! Checks whether this frustum contains the specified sphere.
   *
   *  @remarks The sphere must lie entirely within this frustum for it to count
   *  as being contained.
   */
  bool contains(const Sphere& sphere) const;
  /*! Checks whether this frustum contains the specified bounding box.
   *
   *  @remarks The bounding box must lie entirely within this frustum for it to
   *  count as being contained.
   */
  bool contains(const AABB& box) const;
  /*! Checks whether this frustum intersects the specified sphere.
   *
   *  @remarks Even partial intersection counts.
   */
  bool intersects(const Sphere& sphere) const;
  /*! Checks whether this frustum intersects the specified bounding box.
   *
   *  @remarks Even partial intersection counts.
   */
  bool intersects(const AABB& box) const;
  /*! Transforms the planes of this frustum by the specified transform.
   */
  void transformBy(const Transform3& transform);
  /*! Sets this frustum to a non-rotated, right-handed perspective frustum with
   *  the specified properties.
   */
  void setPerspective(float FOV, float aspectRatio, float nearZ, float farZ);
  /*! Sets this frustum to a non-rotated orthographic frustum with the specified
   *  volume.
   */
  void setOrtho(const AABB& volume);
  /*! Sets this frustum to a non-rotated orthographic frustum with the specified
   *  volume.
   */
  void setOrtho(float minX, float minY, float minZ,
                float maxX, float maxY, float maxZ);
  /*! The planes of this frustum.
   */
  Plane planes[6];
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_FRUSTUM_H*/
///////////////////////////////////////////////////////////////////////
