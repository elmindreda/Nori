///////////////////////////////////////////////////////////////////////
// Wendy default renderer
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
#ifndef WENDY_RENDERCAMERA_H
#define WENDY_RENDERCAMERA_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Transform.h>
#include <wendy/Ray.h>
#include <wendy/Plane.h>
#include <wendy/AABB.h>
#include <wendy/Frustum.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

/*! @brief Basic 3D camera.
 *  @ingroup renderer
 *
 *  This class is most often used together with the scene graph, but can
 *  also be used standalone if you wish to render without using a scene
 *  graph but still wish to model a movable camera.
 */
class Camera : public RefObject
{
public:
  enum Mode
  {
    ORTHOGRAPHIC,
    PERSPECTIVE,
  };
  /*! Constructor.
   */
  Camera();
  bool isOrtho() const;
  bool isPerspective() const;
  Mode getMode() const;
  void setMode(Mode newMode);
  const AABB& getOrthoVolume() const;
  void setOrthoVolume(const AABB& newVolume);
  /*! @return The field of view, in degrees, of this camera.
   */
  float getFOV() const;
  /*! @return The aspect ratio of this camera.
   *
   *  @remarks If the aspect ratio is zero, the aspect ratio will be calculated
   *  from the physical dimensions of the current framebuffer when this camera
   *  is made current.
   */
  float getAspectRatio() const;
  /*! @return The distance from the origin to the near clip plane.
   */
  float getNearZ() const;
  /*! @return The distance from the origin to the far clip plane.
   */
  float getFarZ() const;
  /*! Sets the field of view for this camera.
   *  @param newFOV The desired field of view, in degrees.
   */
  void setFOV(float newFOV);
  /*! Sets the aspect ratio of this camera.
   *  @param[in] newAspectRatio The desired aspect ratio, or zero to
   *  automatically calculate the aspect ratio from the physical dimensions of
   *  the current render target each time this camera is made current.
   */
  void setAspectRatio(float newAspectRatio);
  /*! Sets the position of the near clip plane.
   *  @param[in] newNearZ The distance to the near clip plane.
   */
  void setNearZ(float newNearZ);
  /*! Sets the position of the near and far clip planes.
   *  @param[in] newFarZ The distance to the far clip plane.
   */
  void setFarZ(float newFarZ);
  /*! @return The transform for this camera.
   *
   *  @remarks This is the camera-to-world-space transform for this camera. For
   *  the world-to-camera-space transform, see Camera::getViewTransform.
   */
  const Transform3& getTransform() const;
  /*! @return The inverse of the transform for this camera.
   *
   *  @remarks This is the world-to-camera-space transform for this camera. For
   *  the camera-to-world-space transform, see Camera::getTransform.
   */
  const Transform3& getViewTransform() const;
  /*! Sets the transform for this camera.
   *  @param[in] newTransform The desired new transform.
   *
   *  @remarks This sets the camera-to-world-space transform for this camera.
   */
  void setTransform(const Transform3& newTransform);
  /*! @return The view frustum of this camera.
   */
  const Frustum& getFrustum() const;
  /*! @param[in] A point in world space.
   *  @return The normalized depth of the point in camera space, within the
   *  depth range of this camera.
   */
  float getNormalizedDepth(const vec3& point) const;
  /*! @param[in] position The position, in normalized screen coordinates, from
   *  which to construct ray suitable for picking.
   *  @return A view space ray corresponding to the specified screen position.
   */
  Ray3 getViewSpacePickingRay(const vec2& position) const;
private:
  Mode mode;
  float FOV;
  float aspectRatio;
  float nearZ;
  float farZ;
  AABB volume;
  Transform3 transform;
  mutable Transform3 inverse;
  mutable Frustum frustum;
  mutable bool dirtyFrustum;
  mutable bool dirtyInverse;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERCAMERA_H*/
///////////////////////////////////////////////////////////////////////
