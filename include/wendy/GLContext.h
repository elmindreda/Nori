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
#ifndef WENDY_GLCONTEXT_H
#define WENDY_GLCONTEXT_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Rectangle.h>
#include <wendy/AABB.h>
#include <wendy/Pixel.h>
#include <wendy/Signal.h>
#include <wendy/Timer.h>

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
enum Operation
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

/*! @brief Window mode enumeration.
 *  @ingroup opengl
 */
enum WindowMode
{
  WINDOWED,
  FULLSCREEN
};

///////////////////////////////////////////////////////////////////////

/*! @brief OpenGL profile enumeration.
 *  @ingroup opengl
 */
enum Profile
{
  /*! The default profile provided by the OpenGL implementation.
   */
  PROFILE_DEFAULT,
  /*! The OpenGL core profile.
   */
  PROFILE_CORE,
  /*! The OpenGL compatibility profile.
   */
  PROFILE_COMPAT
};

///////////////////////////////////////////////////////////////////////

/*! @brief OpenGL version descriptor.
 *  @ingroup opengl
 */
class Version
{
public:
  Version();
  Version(unsigned int m, unsigned int n);
  bool operator < (const Version& other) const;
  bool operator > (const Version& other) const;
  unsigned int m;
  unsigned int n;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Window configuration.
 *  @ingroup opengl
 */
class WindowConfig
{
public:
  /*! Default constructor.
   */
  WindowConfig();
  /*! Constructor.
   *  @param[in] title The desired title of the window.
   */
  WindowConfig(const String& title);
  /*! Constructor.
   *  @param[in] title The desired title of the window.
   *  @param[in] width The desired width of the window.
   *  @param[in] height The desired height of the window.
   *  @param[in] mode The desired mode of the window.
   */
  WindowConfig(const String& title,
               unsigned int width,
               unsigned int height,
               WindowMode mode,
               bool resizable = true);
  /*! The desired window title.
   */
  String title;
  /*! The desired width of the window.
   */
  unsigned int width;
  /*! The desired height of the window.
   */
  unsigned int height;
  /*! The desired mode of the window.
   */
  WindowMode mode;
  /*! @c true if the window should be resizable, @c false otherwise.
   */
  bool resizable;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Context configuration.
 *  @ingroup opengl
 *
 *  This class provides the settings parameters available for OpenGL
 *  context creation, as provided through Context::createSingleton.
 */
class ContextConfig
{
public:
  /*! Constructor.
   */
  ContextConfig(unsigned int colorBits = 32,
                unsigned int depthBits = 24,
                unsigned int stencilBits = 0,
                unsigned int samples = 0,
                Version version = Version(2,1),
                Profile profile = PROFILE_DEFAULT);
  /*! The desired color buffer bit depth.
   */
  unsigned int colorBits;
  /*! The desired depth buffer bit depth.
   */
  unsigned int depthBits;
  /*! The desired stencil buffer bit depth.
   */
  unsigned int stencilBits;
  /*! The desired number of FSAA samples.
   */
  unsigned int samples;
  /*! The minimum desired OpenGL version.
   */
  Version version;
  /*! OpenGL profile.
   */
  Profile profile;
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
  unsigned int stencilRef;
  unsigned int stencilMask;
  Operation stencilFailOp;
  Operation depthFailOp;
  Operation depthPassOp;
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
  unsigned int maxColorAttachments;
  /*! The maximum number of simultaneously active color buffers.
   */
  unsigned int maxDrawBuffers;
  /*! The number of available vertex shader texture image units.
   */
  unsigned int maxVertexTextureImageUnits;
  /*! The number of available fragment shader texture image units.
   */
  unsigned int maxFragmentTextureImageUnits;
  /*! The total number of available shader texture image units.
   */
  unsigned int maxCombinedTextureImageUnits;
  /*! The maximum size, in pixels, of 2D POT textures.
   */
  unsigned int maxTextureSize;
  /*! The maximum size, in pixels, of 3D POT textures.
   */
  unsigned int maxTexture3DSize;
  /*! The maximum size, in pixels, of cube map texture faces.
   */
  unsigned int maxTextureCubeSize;
  /*! The maximum size, in pixels, of non-POT 2D textures.
   */
  unsigned int maxTextureRectangleSize;
  /*! The number of available texture coordinates.
   */
  unsigned int maxTextureCoords;
  /*! The maximum texture anisotropy.
   */
  float maxTextureAnisotropy;
  /*! The number of available vertex attributes.
   */
  unsigned int maxVertexAttributes;
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
    unsigned int operationCount;
    unsigned int stateChangeCount;
    unsigned int vertexCount;
    unsigned int pointCount;
    unsigned int lineCount;
    unsigned int triangleCount;
    Time duration;
  };
  Stats();
  void addFrame();
  void addStateChange();
  void addPrimitives(PrimitiveType type, unsigned int vertexCount);
  void addTexture(size_t size);
  void removeTexture(size_t size);
  void addVertexBuffer(size_t size);
  void removeVertexBuffer(size_t size);
  void addIndexBuffer(size_t size);
  void removeIndexBuffer(size_t size);
  void addRenderBuffer(size_t size);
  void removeRenderBuffer(size_t size);
  void addProgram();
  void removeProgram();
  float getFrameRate() const;
  unsigned int getFrameCount() const;
  const Frame& getCurrentFrame() const;
  unsigned int getTextureCount() const;
  unsigned int getVertexBufferCount() const;
  unsigned int getIndexBufferCount() const;
  unsigned int getRenderBufferCount() const;
  unsigned int getProgramCount() const;
  size_t getTotalTextureSize() const;
  size_t getTotalVertexBufferSize() const;
  size_t getTotalIndexBufferSize() const;
  size_t getTotalRenderBufferSize() const;
private:
  typedef std::deque<Frame> FrameQueue;
  unsigned int frameCount;
  float frameRate;
  FrameQueue frames;
  unsigned int textureCount;
  unsigned int vertexBufferCount;
  unsigned int indexBufferCount;
  unsigned int renderBufferCount;
  unsigned int programCount;
  size_t textureSize;
  size_t vertexBufferSize;
  size_t indexBufferSize;
  size_t renderBufferSize;
  Timer timer;
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

/*! @brief OpenGL context singleton.
 *  @ingroup opengl
 *
 *  This class encapsulates the OpenGL context and its associtated window.
 *
 *  @remarks Yes, it's big.
 */
class Context : public Singleton<Context>
{
public:
  /*! Refresh mode enumeration.
   */
  enum RefreshMode
  {
    /*! The Context::update method does not block.
     */
    AUTOMATIC_REFRESH,
    /*! The Context::update method blocks until a refresh is requested.
     */
    MANUAL_REFRESH
  };
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
  void clearStencilBuffer(unsigned int value = 0);
  /*! Clears all buffers of the current framebuffer with the specified values.
   *  @param[in] color The color value to use.
   *  @param[in] depth The depth value to use.
   *  @param[in] value The stencil value to use.
   */
  void clearBuffers(const vec4& color = vec4(0.f),
                    float depth = 1.f,
                    unsigned int value = 0);
  /*! Renders the specified primitive range to the current framebuffer, using
   *  the current GLSL program.
   *  @pre A GLSL program must be set before calling this method.
   */
  void render(const PrimitiveRange& range);
  /*! Renders the specified primitive range to the current framebuffer, using
   *  the current GLSL program.
   *  @pre A GLSL program must be set before calling this method.
   */
  void render(PrimitiveType type, unsigned int start, unsigned int count);
  /*! Makes Context::update to return when in manual refresh mode, forcing
   *  a new iteration of the render loop.
   */
  void refresh();
  /*! Swaps the buffer chain, processes any queued events and, in manual
   *  refresh mode, blocks until either the window is closed or a call to
   *  Context::refresh is made.
   */
  bool update();
  /*! Emulates a user close request, causing a close request signal to be
   *  emitted.
   */
  void requestClose();
  /*! Reserves the specified sampler uniform signature as shared.
   */
  void createSharedSampler(const char* name, SamplerType type, int ID);
  /*! Reserves the specified non-sampler uniform signature as shared.
   */
  void createSharedUniform(const char* name, UniformType type, int ID);
  /*! @return The shared ID of the specified sampler uniform signature.
   */
  int getSharedSamplerID(const char* name, SamplerType type) const;
  /*! @return The shared ID of the specified non-sampler uniform signature.
   */
  int getSharedUniformID(const char* name, UniformType type) const;
  /*! @return The current shared program state, or @c NULL if no shared program
   *  state is currently set.
   */
  SharedProgramState* getCurrentSharedProgramState() const;
  /*! Sets the current shared program state.
   *  @param[in] newState The new state object.
   */
  void setCurrentSharedProgramState(SharedProgramState* newState);
  /*! @return GLSL declarations of all shared samplers and uniforms.
   */
  const char* getSharedProgramStateDeclaration() const;
  /*! @return The window mode of this context.
   */
  WindowMode getWindowMode() const;
  /*! @return The current refresh mode.
   */
  RefreshMode getRefreshMode() const;
  /*! Sets the refresh mode.
   *  @param[in] newMode The desired new refresh mode.
   */
  void setRefreshMode(RefreshMode newMode);
  /*! @return The swap interval of this context.
   */
  int getSwapInterval() const;
  /*! Sets the swap interval of this context.
   *  @param[in] newInterval The desired swap interval.
   */
  void setSwapInterval(int newInterval);
  /*! @return The current scissor rectangle.
   */
  const Recti& getScissorArea() const;
  /*! Sets the scissor area of this context.
   *
   *  @remarks Scissor testing is enabled if the area doesn't include the
   *  entire current framebuffer.
   */
  void setScissorArea(const Recti& newArea);
  /*! @return The current viewport rectangle.
   */
  const Recti& getViewportArea() const;
  /*! Sets the current viewport rectangle.
   *  @param[in] newArea The desired viewport rectangle.
   */
  void setViewportArea(const Recti& newArea);
  /*! @return The current framebuffer.
   */
  Framebuffer& getCurrentFramebuffer() const;
  /*! @return The screen framebuffer.
   */
  DefaultFramebuffer& getDefaultFramebuffer() const;
  /*! Makes the default framebuffer current.
   */
  void setDefaultFramebufferCurrent();
  /*! Makes the specified framebuffer current.
   *  @param[in] newFramebuffer The desired framebuffer.
   *  @return @c true if successful, or @c false otherwise.
   */
  bool setCurrentFramebuffer(Framebuffer& newFramebuffer);
  /*! @return The currently set GLSL program, or @c NULL if no program is set.
   */
  Program* getCurrentProgram() const;
  /*! Sets the current GLSL program for use when rendering.
   *  @param[in] newProgram The desired GLSL program, or @c NULL to unbind
   *  the current program.
   */
  void setCurrentProgram(Program* newProgram);
  /*! @return The currently set vertex buffer.
   */
  VertexBuffer* getCurrentVertexBuffer() const;
  /*! Sets the current vertex buffer.
   */
  void setCurrentVertexBuffer(VertexBuffer* newVertexBuffer);
  /*! @return The currently set index buffer.
   */
  IndexBuffer* getCurrentIndexBuffer() const;
  /*! Sets the current index buffer.
   */
  void setCurrentIndexBuffer(IndexBuffer* newIndexBuffer);
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  Texture* getCurrentTexture() const;
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  void setCurrentTexture(Texture* newTexture);
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  unsigned int getTextureUnitCount() const;
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  unsigned int getActiveTextureUnit() const;
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  void setActiveTextureUnit(unsigned int unit);
  bool isCullingInverted();
  void setCullingInversion(bool newState);
  const RenderState& getCurrentRenderState() const;
  void setCurrentRenderState(const RenderState& newState);
  Stats* getStats() const;
  void setStats(Stats* newStats);
  /*! @return The title of the context window.
   */
  const String& getTitle() const;
  /*! Sets the title of the context window.
   *  @param[in] newTitle The desired title.
   */
  void setTitle(const char* newTitle);
  /*! @return The limits of this context.
   */
  const Limits& getLimits() const;
  /*! @return The resource cache used by this context.
   */
  ResourceCache& getCache() const;
  /*! @return The OpenGL version.
   */
  Version getVersion() const;
  /*! @return The signal for per-frame post-render clean-up.
   */
  SignalProxy0<void> getFinishSignal();
  /*! @return The signal for user-initiated close requests.
   */
  SignalProxy0<bool> getCloseRequestSignal();
  /*! @return The signal for context resizing.
   */
  SignalProxy2<void, unsigned int, unsigned int> getResizedSignal();
  /*! Creates the context singleton object, using the specified settings.
   *  @param[in] cache The resource cache to use.
   *  @param[in] wndconfig The desired window configuration.
   *  @param[in] ctxconfig The desired context configuration.
   *  @return @c true if successful, or @c false otherwise.
   */
  static bool createSingleton(ResourceCache& cache,
                              const WindowConfig& wc = WindowConfig(),
                              const ContextConfig& cc = ContextConfig());
private:
  Context(ResourceCache& cache);
  Context(const Context& source);
  Context& operator = (const Context& source);
  bool init(const WindowConfig& wc, const ContextConfig& cc);
  void applyState(const RenderState& newState);
  void forceState(const RenderState& newState);
  static void sizeCallback(void* window, int width, int height);
  static int closeCallback(void* window);
  static void refreshCallback(void* window);
  class SharedSampler;
  class SharedUniform;
  ResourceCache& cache;
  Signal0<void> finishSignal;
  Signal0<bool> closeRequestSignal;
  Signal2<void, unsigned int, unsigned int> resizedSignal;
  void* handle;
  String title;
  Ptr<Limits> limits;
  WindowMode windowMode;
  RefreshMode refreshMode;
  Version version;
  int swapInterval;
  bool needsRefresh;
  bool needsClosing;
  Recti scissorArea;
  Recti viewportArea;
  bool dirtyBinding;
  bool dirtyState;
  bool cullingInverted;
  TextureList textureUnits;
  unsigned int activeTextureUnit;
  RenderState currentState;
  Ref<Program> currentProgram;
  Ref<VertexBuffer> currentVertexBuffer;
  Ref<IndexBuffer> currentIndexBuffer;
  Ref<Framebuffer> currentFramebuffer;
  Ref<SharedProgramState> currentSharedState;
  Ref<DefaultFramebuffer> defaultFramebuffer;
  std::vector<SharedSampler> samplers;
  std::vector<SharedUniform> uniforms;
  String declaration;
  Stats* stats;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLCONTEXT_H*/
///////////////////////////////////////////////////////////////////////
