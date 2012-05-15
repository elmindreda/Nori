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

#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#include <cstring>
#include <deque>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

class System;

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

  SHARED_STATE_CUSTOM_BASE
};

///////////////////////////////////////////////////////////////////////

typedef uint16 StateID;

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
  float getViewportWidth() const;
  float getViewportHeight() const;
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
  virtual void setViewportSize(float newWidth, float newHeight);
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
  float cameraNearZ;
  float cameraFarZ;
  float cameraAspect;
  float cameraFOV;
  vec3 cameraPos;
  float viewportWidth;
  float viewportHeight;
  float time;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Opaque program state uniform index.
 *  @ingroup renderer
 */
class UniformStateIndex
{
  friend class ProgramState;
public:
  UniformStateIndex();
private:
  UniformStateIndex(uint16 index, uint16 offset);
  uint16 index;
  uint16 offset;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Opaque program state sampler index.
 *  @ingroup renderer
 */
class SamplerStateIndex
{
  friend class ProgramState;
public:
  SamplerStateIndex();
private:
  SamplerStateIndex(uint16 index, uint16 unit);
  uint16 index;
  uint16 unit;
};

///////////////////////////////////////////////////////////////////////

/*! @brief GLSL program state.
 *  @ingroup renderer
 */
class ProgramState
{
public:
  /*! Constructor.
   */
  ProgramState();
  /*! Copy constructor.
   */
  ProgramState(const ProgramState& source);
  /*! Destructor.
   */
  ~ProgramState();
  /*! Applies this GLSL program state to the current context.
   */
  void apply() const;
  bool hasUniformState(const char* name) const;
  bool hasSamplerState(const char* name) const;
  template <typename T>
  void getUniformState(const char* name, T& result) const
  {
    std::memcpy(&result, getData(name, getUniformType<T>()), sizeof(T));
  }
  template <typename T>
  void getUniformState(UniformStateIndex index, T& result) const
  {
    std::memcpy(&result, getData(index, getUniformType<T>()), sizeof(T));
  }
  template <typename T>
  void setUniformState(const char* name, const T& newValue)
  {
    std::memcpy(getData(name, getUniformType<T>()), &newValue, sizeof(T));
  }
  template <typename T>
  void setUniformState(UniformStateIndex index, const T& newValue)
  {
    std::memcpy(getData(index, getUniformType<T>()), &newValue, sizeof(T));
  }
  GL::Texture* getSamplerState(const char* name) const;
  GL::Texture* getSamplerState(SamplerStateIndex index) const;
  void setSamplerState(const char* name, GL::Texture* newTexture);
  void setSamplerState(SamplerStateIndex index, GL::Texture* newTexture);
  UniformStateIndex getUniformStateIndex(const char* name) const;
  SamplerStateIndex getSamplerStateIndex(const char* name) const;
  GL::Program* getProgram() const;
  /*! Sets the GLSL program used by this state object.
   *  @param[in] newProgram The desired GLSL program, or @c NULL to detach
   *  the current program.
   */
  void setProgram(GL::Program* newProgram);
  StateID getID() const;
private:
  static StateID allocateID();
  static void releaseID(StateID ID);
  template <typename T>
  static GL::UniformType getUniformType();
  void* getData(const char* name, GL::UniformType type);
  const void* getData(const char* name, GL::UniformType type) const;
  void* getData(UniformStateIndex index, GL::UniformType type);
  const void* getData(UniformStateIndex index, GL::UniformType type) const;
  typedef std::deque<StateID> IDQueue;
  StateID ID;
  Ref<GL::Program> program;
  std::vector<float> floats;
  GL::TextureList textures;
  static IDQueue usedIDs;
  static StateID nextID;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Render state object.
 *  @ingroup renderer
 *
 *  This class and its associated classes encapsulates most rendering state,
 *  notable exceptions being the transformation and stencil buffer state.
 *
 *  @remarks Yes, it's big.
 */
class Pass : public ProgramState
{
public:
  /*! Applies this render state to the current context.
   */
  void apply() const;
  /*! @return @c true if this render state uses any form of culling, otherwise
   *  @c false.
   */
  bool isCulling() const;
  /*! @return @c true if this render state uses any form of blending with the
   *  framebuffer, otherwise @c false.
   */
  bool isBlending() const;
  /*! @return @c true if this render state uses depth buffer testing, otherwise
   *  @c false.
   */
  bool isDepthTesting() const;
  /*! @return @c true if this render state writes to the depth buffer, otherwise
   *  @c false.
   */
  bool isDepthWriting() const;
  /*! @return @c true if this render state writes to the color buffer, otherwise
   *  @c false.
   */
  bool isColorWriting() const;
  /*! @return @c true if this render state uses stencil buffer testing,
   *  otherwise @c false.
   */
  bool isStencilTesting() const;
  /*! @return @c true if this render state uses wireframe rendering, otherwise
   *  @c false.
   */
  bool isWireframe() const;
  /*! @return @c true if this render state uses line smoothing, otherwise @c false.
   */
  bool isLineSmoothing() const;
  /*! @return @c true if this render state uses multisampling, otherwise @c false.
   */
  bool isMultisampling() const;
  /*! @return @c the width of lines, in pixels.
   */
  float getLineWidth() const;
  /*! @return The culling mode of this render state.
   */
  GL::CullMode getCullMode() const;
  /*! @return The source factor for color buffer blending.
   */
  GL::BlendFactor getSrcFactor() const;
  /*! @return The destination factor for color buffer blending.
   */
  GL::BlendFactor getDstFactor() const;
  /*! @return The depth buffer testing function used by this render state.
   */
  GL::Function getDepthFunction() const;
  /*! @return The stencil buffer testing function used by this render state.
   */
  GL::Function getStencilFunction() const;
  /*! @return The operation to perform when the stencil test fails.
   */
  GL::Operation getStencilFailOperation() const;
  /*! @return The operation to perform when the depth test fails.
   */
  GL::Operation getDepthFailOperation() const;
  /*! @return The operation to perform when the depth test succeeds.
   */
  GL::Operation getDepthPassOperation() const;
  /*! @return The stencil test reference value used by this render state.
   */
  unsigned int getStencilReference() const;
  /*! @return The stencil buffer write mask used by this render state.
   */
  unsigned int getStencilWriteMask() const;
  /*! Sets whether this render state uses depth buffer testing.
   *  @param[in] enable Set to @c true to enable depth buffer testing, or @c
   *  false to disable it.
   */
  void setDepthTesting(bool enable);
  /*! Sets whether this render state writes to the depth buffer.
   *  @param[in] enable Set to @c true to enable depth buffer writing, or @c
   *  false to disable it.
   */
  void setDepthWriting(bool enable);
  /*! Sets whether this render state uses stencil buffer testing.
   *  @param[in] enable Set to @c true to enable stencil buffer testing, or @c
   *  false to disable it.
   */
  void setStencilTesting(bool enable);
  /*! Sets the depth buffer testing function for this render state.
   *  @param[in] function The desired depth testing function.
   */
  void setDepthFunction(GL::Function function);
  /*! Sets the stencil test function for this render state.
   *  @param[in] newFunction The desired stencil testing function.
   */
  void setStencilFunction(GL::Function newFunction);
  /*! Sets the stencil test reference value for this render state.
   *  @param[in] newReference The desired stencil test reference value.
   */
  void setStencilReference(unsigned int newReference);
  /*! Sets the stencil buffer write mask for this render state.
   *  @param[in] newMask The desired stencil buffer write mask.
   */
  void setStencilWriteMask(unsigned int newMask);
  /*! Sets the operation to perform when the stencil test fails.
   */
  void setStencilFailOperation(GL::Operation newOperation);
  /*! Sets the operation to perform when the depth test fails.
   */
  void setDepthFailOperation(GL::Operation newOperation);
  /*! Sets the operation to perform when the depth test succeeds.
   */
  void setDepthPassOperation(GL::Operation newOperation);
  /*! Sets whether writing to the color buffer is enabled.
   *  @param[in] enabled @c true to enable writing to the color buffer, or @c
   *  false to disable it.
   */
  void setColorWriting(bool enabled);
  /*! Sets whether wireframe rendering is enabled.
   *  @param[in] enabled @c true to enable wireframe rendering, or @c false to
   *  disable it.
   */
  void setWireframe(bool enabled);
  /*! Sets whether line smoothing is enabled.
   *  @param[in] enabled @c true to enable line smoothing, or @c false to disable it.
   */
  void setLineSmoothing(bool enabled);
  /*! Sets whether multisampling is enabled.
   *  @param[in] enabled @c true to enable multisampling, or @c false to disable it.
   */
  void setMultisampling(bool enabled);
  /*! Sets the width of lines, in pixels.
   *  @param[in] newWidth The desired new line width.
   */
  void setLineWidth(float newWidth);
  /*! Sets the specified primitive culling mode.
   *  @param[in] mode The desired new primitive culling mode.
   */
  void setCullMode(GL::CullMode mode);
  /*! Sets the factors for color buffer blending.
   *  @param[in] src The desired source factor.
   *  @param[in] dst The desired destination factor.
   */
  void setBlendFactors(GL::BlendFactor src, GL::BlendFactor dst);
private:
  GL::RenderState data;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
typedef std::vector<Pass> PassList;

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERSTATE_H*/
///////////////////////////////////////////////////////////////////////
