///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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
#ifndef WENDY_GLCAMERA_H
#define WENDY_GLCAMERA_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @brief Basic 3D camera.
 *  @ingroup renderer
 *
 *  This class is most often used together with the scene graph, but can
 *  also be used standalone, if you wish to render without using a scene
 *  graph but still wish to model a camera.
 */
class Camera : public Managed<Camera>
{
public:
  /*! Constructor.
   *  @param[in] name The desired name of the camera, or the empty string to
   *  automatically generate a name.
   */
  Camera(const String& name = "");
  /*! Applies this camera to the projection and modelview matrix stacks and
   *  makes it current.
   *  @pre You must have created the @link Renderer renderer @endlink before
   *  calling Camera::begin, as it uses Renderer::begin3D.
   *
   *  @remarks You cannot nest cameras, i.e. you cannot call Camera::begin
   *  if there is already a current camera.
   *  @see Renderer::begin3D
   */
  void begin(void) const;
  /*! Removes this camera from the projection and modelview matrix stacks and 
   *  removes its current status.
   */
  void end(void) const;
  /*! @return The field of view, in degrees, of this camera.
   */
  float getFOV(void) const;
  /*! @return The aspect ratio of this camera.
   *
   *  @remarks If the aspect ratio is zero, the aspect ratio will be calculated
   *  from the physical dimensions of the current render target when this is
   *  made the current camera.
   */
  float getAspectRatio(void) const;
  /*! @return The distance from the origin to the near clip plane.
   */
  float getMinDepth(void) const;
  /*! @return The distance from the origin to the far clip plane.
   */
  float getMaxDepth(void) const;
  /*! Sets the field of view for this camera.
   *  @param newFOV The desired field of view, in degrees.
   *
   *  @remarks You cannot call this on the current camera.
   */
  void setFOV(float newFOV);
  /*! Sets the aspect ratio of this camera.
   *  @param[in] newAspectRatio The desired aspect ratio, or zero to
   *  automatically calculate the aspect ratio from the physical dimensions of
   *  the current render target.
   *
   *  @remarks You cannot call this on the current camera.
   */
  void setAspectRatio(float newAspectRatio);
  /*! Sets the position of the near and far clip planes.
   *  @param[in] minDepth The distance to the near clip plane.
   *  @param[in] maxDepth The distance to the far clip plane.
   *
   *  @remarks You cannot call this on the current camera.
   */
  void setDepthRange(float newMinDepth, float newMaxDepth);
  /*! @return The transform for this camera.
   *
   *  @remarks This is the camera-to-world-space transform for this camera. For
   *  the world-to-camera-space transform, see Camera::getInverseTransform.
   */
  const Transform3& getTransform(void) const;
  /*! @return The inverse of the transform for this camera.
   *
   *  @remarks This is the world-to-camera-space transform for this camera. For
   *  the camera-to-world-space transform, see Camera::getTransform.
   */
  const Transform3& getInverseTransform(void) const;
  /*! Sets the transform for this camera.
   *  @param[in] newTransform The desired new transform.
   *
   *  @remarks This sets the camera-to-world-space transform for this camera.
   */
  void setTransform(const Transform3& newTransform);
  /*! @return The view frustum of this camera.
   */
  const Frustum& getFrustum(void) const;
  /*! @return The current camera, or @c NULL if no camera is current.
   */
  static Camera* getCurrent(void);
private:
  float FOV;
  float aspectRatio;
  float minDepth;
  float maxDepth;
  Transform3 transform;
  mutable Transform3 inverse;
  mutable Frustum frustum;
  mutable bool dirtyFrustum;
  mutable bool dirtyInverse;
  static Camera* current;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLCAMERA_H*/
///////////////////////////////////////////////////////////////////////
