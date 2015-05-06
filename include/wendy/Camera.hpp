///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

#pragma once

namespace nori
{

class Ray3;

/*! @brief Basic 3D camera.
 */
class Camera : public RefObject
{
public:
  /*! Projection mode enumeration.
   */
  enum Mode
  {
    ORTHOGRAPHIC,
    PERSPECTIVE,
  };
  /*! Constructor.
   */
  Camera();
  /*! @return @c true if this camera uses orthographic projection, or @c false
   *  otherwise.
   */
  bool isOrtho() const { return m_mode == ORTHOGRAPHIC; }
  /*! @return @c true if this camera uses perspective projection, or @c false
   *  otherwise.
   */
  bool isPerspective() const { return m_mode == PERSPECTIVE; }
  /*! @return The projection mode used by this camera.
   */
  Mode mode() const { return m_mode; }
  /*! Sets the projection mode of this camera.
   *  @param[in] newMode The projection mode to use.
   */
  void setMode(Mode newMode);
  /*! @return The volume used by orthographic projection.
   */
  const AABB& orthoVolume() const { return m_volume; }
  /*! Sets the volume used by orthographic projection.
   *  @param[in] newVolume The volume to use.
   */
  void setOrthoVolume(const AABB& newVolume);
  /*! @return The field of view, in radians, of this camera.
   */
  float FOV() const { return m_FOV; }
  /*! @return The aspect ratio of this camera.
   *
   *  @remarks If the aspect ratio is zero, the aspect ratio will be calculated
   *  from the physical dimensions of the current framebuffer when this camera
   *  is made current.
   */
  float aspectRatio() const { return m_aspectRatio; }
  /*! @return The distance from the origin to the near clip plane.
   */
  float nearZ() const { return m_nearZ; }
  /*! @return The distance from the origin to the far clip plane.
   */
  float farZ() const { return m_farZ; }
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
   *  the world-to-camera-space transform, see Camera::viewTransform.
   */
  const Transform3& transform() const { return m_transform; }
  /*! @return The inverse of the transform for this camera.
   *
   *  @remarks This is the world-to-camera-space transform for this camera. For
   *  the camera-to-world-space transform, see Camera::transform.
   */
  const Transform3& viewTransform() const { return m_inverse; }
  /*! @return The projection matrix for this camera.
   *
   *  @remarks This is calculated on every call.
   */
  mat4 projectionMatrix() const;
  /*! Sets the transform for this camera.
   *  @param[in] newTransform The desired new transform.
   *
   *  @remarks This sets the camera-to-world-space transform for this camera.
   */
  void setTransform(const Transform3& newTransform);
  /*! @return The view frustum of this camera.
   */
  const Frustum& frustum() const { return m_frustum; }
  /*! @param[in] A point in world space.
   *  @return The normalized depth of the point in camera space, within the
   *  depth range of this camera.
   */
  float normalizedDepth(vec3 point) const;
  /*! @param[in] position The position, in normalized screen coordinates, from
   *  which to construct ray suitable for picking.
   *  @return A view space ray corresponding to the specified screen position.
   */
  Ray3 viewSpacePickingRay(vec2 position) const;
private:
  void updateFrustum();
  void updateInverse();
  Mode m_mode;
  float m_FOV;
  float m_aspectRatio;
  float m_nearZ;
  float m_farZ;
  AABB m_volume;
  Transform3 m_transform;
  Transform3 m_inverse;
  Frustum m_frustum;
};

} /*namespace nori*/

