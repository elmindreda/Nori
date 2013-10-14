///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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
#ifndef WENDY_GLCONTEXT_HPP
#define WENDY_GLCONTEXT_HPP
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.hpp>
#include <wendy/Rect.hpp>
#include <wendy/AABB.hpp>
#include <wendy/Pixel.hpp>
#include <wendy/Signal.hpp>
#include <wendy/Timer.hpp>
#include <wendy/Window.hpp>

#include <deque>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

class VertexBuffer;
class IndexBuffer;
class Context;
class PrimitiveRange;

///////////////////////////////////////////////////////////////////////

/*! The invalid value for shared program state member IDs.
 */
const int INVALID_SHARED_STATE_ID = -1;

///////////////////////////////////////////////////////////////////////

/*! @brief Cull mode enumeration.
 *  @ingroup opengl
 */
enum CullMode
{
  /*! Do not cull any geometry.
   */
  CULL_NONE,
  /*! Cull front-facing geometry (i.e. render back-facing geometry).
   */
  CULL_FRONT,
  /*! Cull back-facing geometry (i.e. render front-facing geometry).
   */
  CULL_BACK,
  /*! Cull all cullable geometry (i.e. front and back faces).
   */
  CULL_BOTH
};

///////////////////////////////////////////////////////////////////////

/*! Blend factor enumeration.
 *  @ingroup opengl
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

///////////////////////////////////////////////////////////////////////

/*! Stencil operation enumeration.
 *  @ingroup opengl
 */
enum StencilOp
{
  OP_KEEP,
  OP_ZERO,
  OP_REPLACE,
  OP_INCREASE,
  OP_DECREASE,
  OP_INVERT,
  OP_INCREASE_WRAP,
  OP_DECREASE_WRAP
};

///////////////////////////////////////////////////////////////////////

/*! Comparison function enumeration.
 *  @ingroup opengl
 */
enum Function
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

///////////////////////////////////////////////////////////////////////

/*! @brief %Context configuration.
 *  @ingroup opengl
 *
 *  This class provides the settings parameters available for OpenGL
 *  context creation, as provided through Context::create.
 */
class ContextConfig
{
public:
  /*! Constructor.
   */
  ContextConfig(uint colorBits = 32,
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
  /*! The OpenGL debug context flag.
   */
  bool debug;
};

///////////////////////////////////////////////////////////////////////

/*! OpenGL render state.
 *  @ingroup opengl
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
  CullMode cullMode;
  BlendFactor srcFactor;
  BlendFactor dstFactor;
  Function depthFunction;
  Function stencilFunction;
  uint stencilRef;
  uint stencilMask;
  StencilOp stencilFailOp;
  StencilOp depthFailOp;
  StencilOp depthPassOp;
};

///////////////////////////////////////////////////////////////////////

/*! OpenGL limits data.
 *  @ingroup opengl
 */
class Limits
{
public:
  /*! Constructor.
   */
  Limits(Context& context);
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

///////////////////////////////////////////////////////////////////////

/*! @brief %Render statistics.
 *  @ingroup opengl
 */
class Stats
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
  Stats();
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

///////////////////////////////////////////////////////////////////////

/*! @brief Interface for global GLSL program state requests.
 *  @ingroup opengl
 */
class SharedProgramState : public RefObject
{
public:
  virtual void updateTo(Uniform& uniform) = 0;
  virtual void updateTo(Sampler& uniform) = 0;
};

///////////////////////////////////////////////////////////////////////

/*! @brief OpenGL context.
 *  @ingroup opengl
 *
 *  This class encapsulates the OpenGL context and its associtated window.
 *
 *  @remarks Yes, it's big.
 */
class Context : public Trackable
{
public:
  /*! Destructor.
   */
  ~Context();
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
  /*! @return The currently set GLSL program, or @c nullptr if no program is
   *  set.
   */
  Program* currentProgram() const;
  /*! Sets the current GLSL program for use when rendering.
   *  @param[in] newProgram The desired GLSL program, or @c nullptr to unbind
   *  the current program.
   */
  void setCurrentProgram(Program* newProgram);
  /*! @return The currently set vertex buffer.
   */
  VertexBuffer* currentVertexBuffer() const;
  /*! Sets the current vertex buffer.
   */
  void setCurrentVertexBuffer(VertexBuffer* newVertexBuffer);
  /*! @return The currently set index buffer.
   */
  IndexBuffer* currentIndexBuffer() const;
  /*! Sets the current index buffer.
   */
  void setCurrentIndexBuffer(IndexBuffer* newIndexBuffer);
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  Texture* currentTexture() const;
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  void setCurrentTexture(Texture* newTexture);
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  uint textureUnitCount() const;
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  uint activeTextureUnit() const;
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  void setActiveTextureUnit(uint unit);
  bool isCullingInverted();
  void setCullingInversion(bool newState);
  const RenderState& currentRenderState() const;
  void setCurrentRenderState(const RenderState& newState);
  Stats* stats() const;
  void setStats(Stats* newStats);
  /*! @return The limits of this context.
   */
  const Limits& limits() const;
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
  static Context* create(ResourceCache& cache,
                         const WindowConfig& wc = WindowConfig(),
                         const ContextConfig& cc = ContextConfig());
private:
  Context(ResourceCache& cache);
  Context(const Context&) = delete;
  bool init(const WindowConfig& wc, const ContextConfig& cc);
  void applyState(const RenderState& newState);
  void forceState(const RenderState& newState);
  Context& operator = (const Context&) = delete;
  void onFrame();
  class SharedSampler;
  class SharedUniform;
  ResourceCache& m_cache;
  Window m_window;
  GLFWwindow* m_handle;
  Ptr<Limits> m_limits;
  int m_swapInterval;
  Recti m_scissorArea;
  Recti m_viewportArea;
  bool m_dirtyBinding;
  bool m_dirtyState;
  bool m_cullingInverted;
  TextureList m_textureUnits;
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
  String m_declaration;
  Stats* m_stats;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLCONTEXT_HPP*/
///////////////////////////////////////////////////////////////////////
