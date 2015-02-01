///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/Core.hpp>
#include <wendy/Rect.hpp>
#include <wendy/Pixel.hpp>
#include <wendy/Signal.hpp>
#include <wendy/Time.hpp>
#include <wendy/Window.hpp>

#include <deque>

namespace wendy
{

class AABB;
class VertexBuffer;
class IndexBuffer;
class RenderContext;
class PrimitiveRange;

/*! @brief Polygon face enumeration.
 */
enum PolygonFace
{
  /*! Cull front-facing geometry (i.e. render back-facing geometry).
   */
  FACE_FRONT,
  /*! Cull back-facing geometry (i.e. render front-facing geometry).
   */
  FACE_BACK,
  /*! Cull all cullable geometry (i.e. front and back faces).
   */
  FACE_BOTH,
  /*! Do not cull any geometry.
   */
  FACE_NONE
};

/*! Blend factor enumeration.
 */
enum BlendFactor
{
  BLEND_ZERO,
  BLEND_ONE,
  BLEND_SRC_COLOR,
  BLEND_DST_COLOR,
  BLEND_SRC_ALPHA,
  BLEND_DST_ALPHA,
  BLEND_ONE_MINUS_SRC_COLOR,
  BLEND_ONE_MINUS_DST_COLOR,
  BLEND_ONE_MINUS_SRC_ALPHA,
  BLEND_ONE_MINUS_DST_ALPHA
};

/*! Stencil operation enumeration.
 */
enum StencilOp
{
  STENCIL_KEEP,
  STENCIL_ZERO,
  STENCIL_REPLACE,
  STENCIL_INCREASE,
  STENCIL_DECREASE,
  STENCIL_INVERT,
  STENCIL_INCREASE_WRAP,
  STENCIL_DECREASE_WRAP
};

/*! Comparison function enumeration.
 */
enum FragmentFunction
{
  ALLOW_NEVER,
  ALLOW_ALWAYS,
  ALLOW_EQUAL,
  ALLOW_NOT_EQUAL,
  ALLOW_LESSER,
  ALLOW_LESSER_EQUAL,
  ALLOW_GREATER,
  ALLOW_GREATER_EQUAL
};

enum
{
  SHARED_MODEL_MATRIX,
  SHARED_VIEW_MATRIX,
  SHARED_PROJECTION_MATRIX,
  SHARED_MODELVIEW_MATRIX,
  SHARED_VIEWPROJECTION_MATRIX,
  SHARED_MODELVIEWPROJECTION_MATRIX,

  SHARED_INVERSE_MODEL_MATRIX,
  SHARED_INVERSE_VIEW_MATRIX,
  SHARED_INVERSE_PROJECTION_MATRIX,
  SHARED_INVERSE_MODELVIEW_MATRIX,
  SHARED_INVERSE_VIEWPROJECTION_MATRIX,
  SHARED_INVERSE_MODELVIEWPROJECTION_MATRIX,

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

/*! @brief Render context configuration.
 *
 *  This class provides the settings parameters available for render
 *  context creation, as provided through RenderContext::create.
 */
class RenderConfig
{
public:
  /*! Constructor.
   */
  RenderConfig(uint colorBits = 32,
               uint depthBits = 24,
               uint stencilBits = 0,
               uint samples = 0,
               bool debug = false);
  /*! The desired color buffer bit depth.
   */
  uint colorBits;
  /*! The desired depth buffer bit depth.
   */
  uint depthBits;
  /*! The desired stencil buffer bit depth.
   */
  uint stencilBits;
  /*! The desired number of FSAA samples.
   */
  uint samples;
  /*! Whether to create a debug context.
   */
  bool debug;
};

/*! Render state.
 */
class RenderState
{
public:
  RenderState();
  bool depthTesting;
  bool depthWriting;
  bool colorWriting;
  bool stencilTesting;
  bool wireframe;
  bool lineSmoothing;
  bool multisampling;
  float lineWidth;
  PolygonFace cullFace;
  BlendFactor srcFactor;
  BlendFactor dstFactor;
  FragmentFunction depthFunction;
  struct {
    FragmentFunction function;
    uint reference;
    uint mask;
    StencilOp stencilFailOp;
    StencilOp depthFailOp;
    StencilOp depthPassOp;
  } stencil[2];
};

/*! Render context limits data.
 */
class RenderLimits
{
public:
  /*! Constructor.
   */
  RenderLimits(RenderContext& context);
  /*! The maximum number of color buffers that can be attached to to an image
   *  framebuffer (FBO).
   */
  uint maxColorAttachments;
  /*! The maximum number of simultaneously active color buffers.
   */
  uint maxDrawBuffers;
  /*! The number of available vertex shader texture image units.
   */
  uint maxVertexTextureImageUnits;
  /*! The number of available fragment shader texture image units.
   */
  uint maxFragmentTextureImageUnits;
  /*! The total number of available shader texture image units.
   */
  uint maxCombinedTextureImageUnits;
  /*! The maximum size, in pixels, of 2D POT textures.
   */
  uint maxTextureSize;
  /*! The maximum size, in pixels, of 3D POT textures.
   */
  uint maxTexture3DSize;
  /*! The maximum size, in pixels, of cube map texture faces.
   */
  uint maxTextureCubeSize;
  /*! The maximum size, in pixels, of non-POT 2D textures.
   */
  uint maxTextureRectangleSize;
  /*! The number of available texture coordinates.
   */
  uint maxTextureCoords;
  /*! The maximum texture anisotropy.
   */
  float maxTextureAnisotropy;
  /*! The number of available vertex attributes.
   */
  uint maxVertexAttributes;
};

/*! @brief %Render statistics.
 */
class RenderStats
{
public:
  class Frame
  {
  public:
    Frame();
    uint operationCount;
    uint stateChangeCount;
    uint vertexCount;
    uint pointCount;
    uint lineCount;
    uint triangleCount;
    Time duration;
  };
  RenderStats();
  void addFrame();
  void addStateChange();
  void addPrimitives(PrimitiveType type, uint vertexCount);
  void addTexture(size_t size);
  void removeTexture(size_t size);
  void addVertexBuffer(size_t size);
  void removeVertexBuffer(size_t size);
  void addIndexBuffer(size_t size);
  void removeIndexBuffer(size_t size);
  void addProgram();
  void removeProgram();
  float frameRate() const { return m_frameRate; }
  uint frameCount() const { return m_frameCount; }
  const Frame& currentFrame() const { return m_frames.front(); }
  uint textureCount() const { return m_textureCount; }
  uint vertexBufferCount() const { return m_vertexBufferCount; }
  uint indexBufferCount() const { return m_indexBufferCount; }
  uint programCount() const { return m_programCount; }
  size_t totalTextureSize() const { return m_textureSize; }
  size_t totalVertexBufferSize() const { return m_vertexBufferSize; }
  size_t totalIndexBufferSize() const { return m_indexBufferSize; }
private:
  uint m_frameCount;
  float m_frameRate;
  std::deque<Frame> m_frames;
  uint m_textureCount;
  uint m_vertexBufferCount;
  uint m_indexBufferCount;
  uint m_programCount;
  size_t m_textureSize;
  size_t m_vertexBufferSize;
  size_t m_indexBufferSize;
  Timer m_timer;
};

class SharedProgramState : public RefObject
{
public:
  /*! Constructor.
   */
  SharedProgramState();
  virtual void updateTo(Uniform& uniform);
  virtual void updateTo(Sampler& uniform);
  /*! @return The current model matrix.
   */
  const mat4& modelMatrix() const { return m_modelMatrix; }
  /*! @return The current view matrix.
   */
  const mat4& viewMatrix() const { return m_viewMatrix; }
  /*! @return The current projection matrix.
   */
  const mat4& projectionMatrix() const { return m_projectionMatrix; }
  void cameraProperties(vec3& position,
                        float& FOV,
                        float& aspect,
                        float& nearZ,
                        float& farZ) const;
  float viewportWidth() const { return m_viewportWidth; }
  float viewportHeight() const { return m_viewportHeight; }
  float time() const { return m_time; }
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
private:
  bool m_dirtyModelView;
  bool m_dirtyViewProj;
  bool m_dirtyModelViewProj;
  bool m_dirtyInvModel;
  bool m_dirtyInvView;
  bool m_dirtyInvProj;
  bool m_dirtyInvModelView;
  bool m_dirtyInvViewProj;
  bool m_dirtyInvModelViewProj;
  mat4 m_modelMatrix;
  mat4 m_viewMatrix;
  mat4 m_projectionMatrix;
  mat4 m_modelViewMatrix;
  mat4 m_viewProjMatrix;
  mat4 m_modelViewProjMatrix;
  mat4 m_invModelMatrix;
  mat4 m_invViewMatrix;
  mat4 m_invProjMatrix;
  mat4 m_invModelViewMatrix;
  mat4 m_invViewProjMatrix;
  mat4 m_invModelViewProjMatrix;
  float m_cameraNearZ;
  float m_cameraFarZ;
  float m_cameraAspect;
  float m_cameraFOV;
  vec3 m_cameraPos;
  float m_viewportWidth;
  float m_viewportHeight;
  float m_time;
};

/*! @brief Render context.
 */
class RenderContext : public Trackable
{
public:
  /*! Destructor.
   */
  ~RenderContext();
  /*! Clears the color buffers of the current framebuffer with the specified
   *  color.
   *  @param[in] color The color value to use.
   */
  void clearColorBuffer(const vec4& color = vec4(0.f));
  /*! Clears the depth buffer of the current framebuffer with the specified
   *  depth value.
   *  @param[in] depth The depth value to use.
   */
  void clearDepthBuffer(float depth = 1.f);
  /*! Clears the stencil buffer of the current framebuffer with the specified
   *  stencil value.
   *  @param[in] value The stencil value to use.
   */
  void clearStencilBuffer(uint value = 0);
  /*! Clears all buffers of the current framebuffer with the specified values.
   *  @param[in] color The color value to use.
   *  @param[in] depth The depth value to use.
   *  @param[in] value The stencil value to use.
   */
  void clearBuffers(const vec4& color = vec4(0.f),
                    float depth = 1.f,
                    uint value = 0);
  /*! Renders the specified primitive range to the current framebuffer, using
   *  the current GLSL program.
   *  @pre A GLSL program must be set before calling this method.
   */
  void render(const PrimitiveRange& range);
  /*! Renders the specified primitive range to the current framebuffer, using
   *  the current GLSL program.
   *  @pre A GLSL program must be set before calling this method.
   */
  void render(PrimitiveType type, uint start, uint count, uint base = 0);
  /*! Allocates a range of temporary vertices of the specified format.
   *  @param[in] count The number of vertices to allocate.
   *  @param[in] format The format of vertices to allocate.
   *  @return @c The newly allocated vertex range.
   *
   *  @remarks The allocated vertex range is only valid until the end of the
   *  current frame.
   */
  VertexRange allocateVertices(uint count, const VertexFormat& format);
  /*! Reserves the specified sampler uniform signature as shared.
   */
  void createSharedSampler(const char* name, SamplerType type, int ID);
  /*! Reserves the specified non-sampler uniform signature as shared.
   */
  void createSharedUniform(const char* name, UniformType type, int ID);
  /*! @return The shared ID of the specified sampler uniform signature.
   */
  int sharedSamplerID(const char* name, SamplerType type) const;
  /*! @return The shared ID of the specified non-sampler uniform signature.
   */
  int sharedUniformID(const char* name, UniformType type) const;
  /*! @return The current shared program state, or @c nullptr if no shared
   *  program state is currently set.
   */
  SharedProgramState* currentSharedProgramState() const;
  /*! Sets the current shared program state.
   *  @param[in] newState The new state object.
   */
  void setCurrentSharedProgramState(SharedProgramState* newState);
  /*! @return GLSL declarations of all shared samplers and uniforms.
   */
  const char* sharedProgramStateDeclaration() const;
  /*! @return The swap interval of this context.
   */
  int swapInterval() const;
  /*! Sets the swap interval of this context.
   *  @param[in] newInterval The desired swap interval.
   */
  void setSwapInterval(int newInterval);
  /*! @return The current scissor rectangle.
   */
  const Recti& scissorArea() const;
  /*! Sets the scissor area of this context.
   *
   *  @remarks Scissor testing is enabled if the area doesn't include the
   *  entire current framebuffer.
   */
  void setScissorArea(const Recti& newArea);
  /*! @return The current viewport rectangle.
   */
  const Recti& viewportArea() const;
  /*! Sets the current viewport rectangle.
   *  @param[in] newArea The desired viewport rectangle.
   */
  void setViewportArea(const Recti& newArea);
  /*! @return The current framebuffer.
   */
  Framebuffer& currentFramebuffer() const;
  /*! @return The screen framebuffer.
   */
  DefaultFramebuffer& defaultFramebuffer() const;
  /*! Makes the default framebuffer current.
   */
  void setDefaultFramebufferCurrent();
  /*! Makes the specified framebuffer current.
   *  @param[in] newFramebuffer The desired framebuffer.
   *  @return @c true if successful, or @c false otherwise.
   */
  bool setCurrentFramebuffer(Framebuffer& newFramebuffer);
  /*! Sets the current GLSL program for use when rendering.
   *  @param[in] newProgram The desired GLSL program, or @c nullptr to unbind
   *  the current program.
   */
  void setCurrentProgram(Program* newProgram);
  /*! Sets the current vertex buffer.
   */
  void setCurrentVertexBuffer(VertexBuffer* newVertexBuffer);
  /*! Sets the current index buffer.
   */
  void setCurrentIndexBuffer(IndexBuffer* newIndexBuffer);
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  void setCurrentTexture(Texture* newTexture);
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  void setActiveTextureUnit(uint unit);
  bool isCullingInverted();
  void setCullingInversion(bool newState);
  const RenderState& currentRenderState() const;
  void setCurrentRenderState(const RenderState& newState);
  bool debug() const { return m_debug; }
  RenderStats* stats() const;
  void setStats(RenderStats* newStats);
  /*! @return The limits of this context.
   */
  const RenderLimits& limits() const;
  /*! @return The resource cache used by this context.
   */
  ResourceCache& cache() const;
  /*! @return The window of this context.
   */
  Window& window();
  /*! Creates the context object, using the specified settings.
   *  @param[in] cache The resource cache to use.
   *  @param[in] wndconfig The desired window configuration.
   *  @param[in] ctxconfig The desired context configuration.
   *  @return @c true if successful, or @c false otherwise.
   */
  static std::unique_ptr<RenderContext> create(ResourceCache& cache,
                                               const WindowConfig& wc = WindowConfig(),
                                               const RenderConfig& rc = RenderConfig());
private:
  RenderContext(ResourceCache& cache);
  RenderContext(const RenderContext&) = delete;
  bool init(const WindowConfig& wc, const RenderConfig& rc);
  void applyState(const RenderState& newState);
  void forceState(const RenderState& newState);
  RenderContext& operator = (const RenderContext&) = delete;
  void onFrame();
  struct Slot
  {
    Ref<VertexBuffer> buffer;
    uint available;
  };
  class SharedSampler;
  class SharedUniform;
  ResourceCache& m_cache;
  Window m_window;
  GLFWwindow* m_handle;
  bool m_debug;
  std::unique_ptr<RenderLimits> m_limits;
  int m_swapInterval;
  Recti m_scissorArea;
  Recti m_viewportArea;
  bool m_dirtyBinding;
  bool m_dirtyState;
  bool m_cullingInverted;
  std::vector<Ref<Texture>> m_textureUnits;
  uint m_activeTextureUnit;
  RenderState m_currentState;
  Ref<Program> m_currentProgram;
  Ref<VertexBuffer> m_currentVertexBuffer;
  Ref<IndexBuffer> m_currentIndexBuffer;
  Ref<Framebuffer> m_currentFramebuffer;
  Ref<SharedProgramState> m_currentSharedState;
  Ref<DefaultFramebuffer> m_defaultFramebuffer;
  std::vector<SharedSampler> m_samplers;
  std::vector<SharedUniform> m_uniforms;
  std::vector<Slot> m_slots;
  std::string m_declaration;
  RenderStats* m_stats;
};

} /*namespace wendy*/

