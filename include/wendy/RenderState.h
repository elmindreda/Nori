///////////////////////////////////////////////////////////////////////
// Wendy default renderer
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_RENDERSTATE_H
#define WENDY_RENDERSTATE_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>

#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

enum
{
  SHARED_MODEL_MATRIX,
  SHARED_VIEW_MATRIX,
  SHARED_PROJECTION_MATRIX,
  SHARED_MODELVIEW_MATRIX,
  SHARED_VIEWPROJECTION_MATRIX,
  SHARED_MODELVIEWPROJECTION_MATRIX,

  SHARED_CAMERA_NEAR_Z,
  SHARED_CAMERA_FAR_Z,
  SHARED_CAMERA_ASPECT_RATIO,
  SHARED_CAMERA_FOV,
  SHARED_CAMERA_POSITION,

  SHARED_VIEWPORT_WIDTH,
  SHARED_VIEWPORT_HEIGHT,

  SHARED_TIME,

  SHARED_STATE_CUSTOM_BASE,
};

///////////////////////////////////////////////////////////////////////

class SharedProgramState : public GL::SharedProgramState
{
public:
  /*! Constructor.
   */
  SharedProgramState();
  /*! Reserves the supported uniform and sampler signatures as shared in the
   *  specified context.
   */
  virtual bool reserveSupported(GL::Context& context) const;
  /*! @return The current model matrix.
   */
  const mat4& getModelMatrix() const;
  /*! @return The current view matrix.
   */
  const mat4& getViewMatrix() const;
  /*! @return The current projection matrix.
   */
  const mat4& getProjectionMatrix() const;
  void getCameraProperties(vec3& position,
                           float& FOV,
                           float& aspect,
                           float& nearZ,
                           float& farZ) const;
  float getTime() const;
  /*! Sets the model matrix.
   *  @param[in] newMatrix The desired model matrix.
   */
  virtual void setModelMatrix(const mat4& newMatrix);
  /*! Sets the view matrix.
   *  @param[in] newMatrix The desired view matrix.
   */
  virtual void setViewMatrix(const mat4& newMatrix);
  /*! Sets the projection matrix.
   *  @param[in] newMatrix The desired projection matrix.
   */
  virtual void setProjectionMatrix(const mat4& newMatrix);
  /*! Sets an orthographic projection matrix as ([0..width], [0..height],
   *  [-1, 1]).
   *  @param[in] width The desired width of the clipspace volume.
   *  @param[in] height The desired height of the clipspace volume.
   */
  virtual void setOrthoProjectionMatrix(float width, float height);
  /*! Sets an orthographic projection matrix as ([minX..maxX], [minY..maxY],
   *  [minZ, maxZ]).
   *  @param[in] volume The desired projection volume.
   */
  virtual void setOrthoProjectionMatrix(const AABB& volume);
  /*! Sets a perspective projection matrix.
   *  @param[in] FOV The desired field of view of the projection.
   *  @param[in] aspect The desired aspect ratio of the projection.
   *  @param[in] nearZ The desired near plane distance of the projection.
   *  @param[in] farZ The desired far plane distance of the projection.
   */
  virtual void setPerspectiveProjectionMatrix(float FOV,
                                              float aspect,
                                              float nearZ,
                                              float farZ);
  virtual void setCameraProperties(const vec3& position,
                                   float FOV,
                                   float aspect,
                                   float nearZ,
                                   float farZ);
  virtual void setTime(float newTime);
protected:
  virtual void updateTo(GL::Uniform& uniform);
  virtual void updateTo(GL::Sampler& uniform);
private:
  bool dirtyModelView;
  bool dirtyViewProj;
  bool dirtyModelViewProj;
  mat4 modelMatrix;
  mat4 viewMatrix;
  mat4 projectionMatrix;
  mat4 modelViewMatrix;
  mat4 viewProjMatrix;
  mat4 modelViewProjMatrix;
  struct
  {
    float nearZ;
    float farZ;
    float aspect;
    float FOV;
    vec3 position;
  } camera;
  float time;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERSTATE_H*/
///////////////////////////////////////////////////////////////////////
