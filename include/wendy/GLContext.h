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
#include <wendy/Plane.h>
#include <wendy/Rectangle.h>
#include <wendy/AABB.h>
#include <wendy/Pixel.h>
#include <wendy/Signal.h>
#include <wendy/Timer.h>

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

enum
{
  SHARED_MODEL_MATRIX,
  SHARED_VIEW_MATRIX,
  SHARED_PROJECTION_MATRIX,
  SHARED_MODELVIEW_MATRIX,
  SHARED_VIEWPROJECTION_MATRIX,
  SHARED_MODELVIEWPROJECTION_MATRIX,

  SHARED_STATE_CUSTOM_BASE,

  INVALID_SHARED_STATE_ID = -1,
};

///////////////////////////////////////////////////////////////////////

/*! @brief Context window mode.
 *  @ingroup opengl
 */
enum WindowMode
{
  WINDOWED,
  FULLSCREEN,
};

///////////////////////////////////////////////////////////////////////

/*! @brief Screen mode.
 *  @ingroup opengl
 */
class ScreenMode
{
public:
  /*! Default constructor.
   */
  ScreenMode(void);
  /*! Constructor.
   *  @param[in] width The desired width.
   *  @param[in] height The desired height.
   *  @param[in] colorBits The desired number of color bits.
   */
  ScreenMode(unsigned int width, unsigned int height, unsigned int colorBits);
  /*! Resets all value to their defaults.
   */
  void setDefaults(void);
  /*! Sets the specified values.
   *  @param[in] newWidth The desired width.
   *  @param[in] newHeight The desired height.
   *  @param[in] newColorBits The desired number of color bits.
   */
  void set(unsigned int newWidth, unsigned int newHeight, unsigned int newColorBits);
  /*! The desired width of the context.
   */
  unsigned int width;
  /*! The desired height of the context.
   */
  unsigned int height;
  /*! The desired color buffer bit depth.
   */
  unsigned int colorBits;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
typedef std::vector<ScreenMode> ScreenModeList;

///////////////////////////////////////////////////////////////////////

/*! @brief %Context settings.
 *  @ingroup opengl
 *
 *  This class provides the settings parameters available for OpenGL
 *  context creation, as provided through Context::create.
 */
class ContextMode : public ScreenMode
{
public:
  /*! Default constructor.
   */
  ContextMode(void);
  /*! Constructor.
   */
  ContextMode(unsigned int width,
	      unsigned int height,
	      unsigned int colorBits,
	      unsigned int depthBits = 0,
	      unsigned int stencilBits = 0,
	      unsigned int samples = 0,
	      WindowMode mode = WINDOWED);
  /*! Resets all value to their defaults.
   */
  void setDefaults(void);
  /*! Sets the specified value.
   */
  void set(unsigned int newWidth,
	   unsigned int newHeight,
	   unsigned int newColorBits,
	   unsigned int newDepthBits = 0,
	   unsigned int newStencilBits = 0,
	   unsigned int newSamples = 0,
	   WindowMode newFlags = WINDOWED);
  /*! The desired depth buffer bit depth.
   */
  unsigned int depthBits;
  /*! The desired stencil buffer bit depth.
   */
  unsigned int stencilBits;
  /*! The desired number of FSAA samples.
   */
  unsigned int samples;
  /*! The desired window mode.
   */
  WindowMode mode;
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
  /*! @return The maximum number of color buffers that can be attached to to an
   *  image canvas (FBO).
   */
  unsigned int getMaxColorAttachments(void) const;
  /*! @return The maximum number of simultaneously active color buffers.
   */
  unsigned int getMaxDrawBuffers(void) const;
  /*! @return The number of available clip planes.
   */
  unsigned int getMaxClipPlanes(void) const;
  /*! @return The number of available vertex shader texture image units.
   */
  unsigned int getMaxVertexTextureImageUnits(void) const;
  /*! @return The number of available fragment shader texture image units.
   */
  unsigned int getMaxFragmentTextureImageUnits(void) const;
  /*! @return The number of available vertex and fragment shader texture image units.
   */
  unsigned int getMaxCombinedTextureImageUnits(void) const;
  /*! @return The maximum size, in pixels, of 2D POT textures.
   */
  unsigned int getMaxTextureSize(void) const;
  /*! @return The maximum size, in pixels, of cube map texture faces.
   */
  unsigned int getMaxTextureCubeSize(void) const;
  /*! @return The maximum size, in pixels, of non-POT 2D textures.
   */
  unsigned int getMaxTextureRectangleSize(void) const;
  /*! @return The number of available texture coordinates.
   */
  unsigned int getMaxTextureCoords(void) const;
  /*! @return The number of available vertex attributes.
   */
  unsigned int getMaxVertexAttributes(void) const;
private:
  Context& context;
  unsigned int maxColorAttachments;
  unsigned int maxDrawBuffers;
  unsigned int maxClipPlanes;
  unsigned int maxVertexTextureImageUnits;
  unsigned int maxFragmentTextureImageUnits;
  unsigned int maxCombinedTextureImageUnits;
  unsigned int maxTextureSize;
  unsigned int maxTextureCubeSize;
  unsigned int maxTextureRectangleSize;
  unsigned int maxVertexAttributes;
  unsigned int maxTextureCoords;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Rendering canvas.
 *  @ingroup opengl
 *
 *  This class represents a render target, i.e. a framebuffer.
 */
class Canvas : public RefObject
{
  friend class Context;
  friend class ImageCanvas;
public:
  /*! Destructor.
   */
  virtual ~Canvas(void);
  /*! @return The width, in pixels, of this canvas.
   */
  virtual unsigned int getWidth(void) const = 0;
  /*! @return The height, in pixels, of this canvas.
   */
  virtual unsigned int getHeight(void) const = 0;
  /*! @return The aspect ratio of the dimensions, in pixels, of this canvas.
   */
  float getAspectRatio(void) const;
  /*! @return The context this canvas was created for.
   */
  Context& getContext(void) const;
protected:
  /*! Constructor.
   */
  Canvas(Context& context);
  /*! Called when this canvas is to be made current.
   */
  virtual void apply(void) const = 0;
  bool isCurrent(void) const;
  static const Canvas* getCurrent(void);
private:
  Canvas(const Canvas& source);
  Canvas& operator = (const Canvas& source);
  Context& context;
  static const Canvas* current;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Canvas for rendering to the screen.
 *  @ingroup opengl
 */
class ScreenCanvas : public Canvas
{
  friend class Context;
public:
  /*! @return The default framebuffer color depth, in bits.
   */
  unsigned int getColorBits(void) const;
  /*! @return The default framebuffer depth-buffer depth, in bits.
   */
  unsigned int getDepthBits(void) const;
  /*! @return The default framebuffer stencil buffer depth, in bits.
   */
  unsigned int getStencilBits(void) const;
  unsigned int getWidth(void) const;
  unsigned int getHeight(void) const;
private:
  ScreenCanvas(Context& context);
  void apply(void) const;
  ContextMode mode;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Canvas for rendering to a texture.
 *  @ingroup opengl
 */
class ImageCanvas : public Canvas
{
public:
  enum Attachment
  {
    COLOR_BUFFER0,
    COLOR_BUFFER1,
    COLOR_BUFFER2,
    COLOR_BUFFER3,
    DEPTH_BUFFER,
  };
  ~ImageCanvas(void);
  unsigned int getWidth(void) const;
  unsigned int getHeight(void) const;
  /*! @return The texture that this canvas uses as a color buffer.
   */
  Image* getColorBuffer(void) const;
  Image* getDepthBuffer(void) const;
  Image* getBuffer(Attachment attachment) const;
  /*! Sets the image to use as the color buffer for this canvas.
   *  @param[in] newImage The desired image, or @c NULL to detach the currently
   *  set image.
   */
  bool setColorBuffer(Image* newImage);
  bool setDepthBuffer(Image* newImage);
  bool setBuffer(Attachment attachment, Image* newImage);
  /*! Creates a texture canvas for the specified texture.
   */
  static ImageCanvas* create(Context& context,
                             unsigned int width,
                             unsigned int height);
private:
  ImageCanvas(Context& context);
  bool init(unsigned int width, unsigned int height);
  void apply(void) const;
  unsigned int width;
  unsigned int height;
  unsigned int bufferID;
  ImageRef buffers[5];
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
    Frame(void);
    unsigned int passCount;
    unsigned int vertexCount;
    unsigned int pointCount;
    unsigned int lineCount;
    unsigned int triangleCount;
    Time duration;
  };
  typedef std::deque<Frame> FrameQueue;
  Stats(void);
  void addFrame(void);
  void addPasses(unsigned int count);
  void addPrimitives(PrimitiveType type, unsigned int count);
  float getFrameRate(void) const;
  unsigned int getFrameCount(void) const;
  const Frame& getFrame(void) const;
private:
  unsigned int frameCount;
  float frameRate;
  FrameQueue frames;
  Timer timer;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Interface for global GPU program state requests.
 *  @ingroup opengl
 */
class SharedProgramState
{
  friend class ProgramState;
public:
  /*! Constructor.
   */
  SharedProgramState(void);
  /*! Destructor.
   */
  virtual ~SharedProgramState(void);
  /*! @return The current model matrix.
   */
  const mat4& getModelMatrix(void) const;
  /*! @return The current view matrix.
   */
  const mat4& getViewMatrix(void) const;
  /*! @return The current projection matrix.
   */
  const mat4& getProjectionMatrix(void) const;
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
   *  [-1, * 1]).
   *  @param[in] width The desired width of the clipspace volume.
   *  @param[in] height The desired height of the clipspace volume.
   */
  virtual void setOrthoProjectionMatrix(float width, float height);
  /*! Sets an orthographic projection matrix as ([minX..maxX], [minY..maxY],
   *  [minZ, maxZ]).
   *  @param[in] volume The desired clipspace volume.
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
protected:
  virtual void updateTo(Uniform& uniform);
  virtual void updateTo(Sampler& uniform);
private:
  mat4 modelMatrix;
  mat4 viewMatrix;
  mat4 projectionMatrix;
  mat4 modelViewMatrix;
  mat4 viewProjMatrix;
  mat4 modelViewProjMatrix;
  bool dirtyModelView;
  bool dirtyViewProj;
  bool dirtyModelViewProj;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
class SharedSampler
{
public:
  SharedSampler(const String& name, Sampler::Type type, int ID);
  String name;
  Sampler::Type type;
  int ID;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
class SharedUniform
{
public:
  SharedUniform(const String& name, Uniform::Type type, int ID);
  String name;
  Uniform::Type type;
  int ID;
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
    MANUAL_REFRESH,
  };
  typedef std::vector<Plane> PlaneList;
  /*! Destructor.
   */
  ~Context(void);
  /*! Clears the current color buffer with the specified color.
   *  @param[in] color The color value to clear the color buffer with.
   */
  void clearColorBuffer(const vec4& color = vec4(0.f));
  /*! Clears the current depth buffer with the specified depth value.
   *  @param[in] depth The depth value to clear the depth buffer with.
   */
  void clearDepthBuffer(float depth = 1.f);
  /*! Clears the current stencil buffer with the specified stencil value.
   *  @param[in] value The stencil value to clear the stencil buffer with.
   */
  void clearStencilBuffer(unsigned int value = 0);
  /*! Renders the specified primitive range to the current canvas, using the
   *  current GPU program.
   *  @pre A GPU program must be set before calling this method.
   */
  void render(const PrimitiveRange& range);
  /*! Renders the specified primitive range to the current canvas, using the
   *  current GPU program.
   *  @pre A GPU program must be set before calling this method.
   */
  void render(PrimitiveType type, unsigned int start, unsigned int count);
  /*! Makes Context::update to return when in manual refresh mode, forcing
   *  a new iteration of the render loop.
   */
  void refresh(void);
  /*! Swaps the buffer chain, processes any queued events and, in manual
   *  refresh mode, blocks until either the window is closed or a call to
   *  Context::refresh is made.
   */
  bool update(void);
  /*! Reserves the specified sampler uniform signature as shared.
   */
  void createSharedSampler(const String& name, Sampler::Type type, int ID);
  /*! Reserves the specified non-sampler uniform signature as shared.
   */
  void createSharedUniform(const String& name, Uniform::Type type, int ID);
  /*! @return The shared ID of the specified sampler uniform signature.
   */
  int getSharedSamplerID(const String& name, Sampler::Type type) const;
  /*! @return The shared ID of the specified non-sampler uniform signature.
   */
  int getSharedUniformID(const String& name, Uniform::Type type) const;
  SharedProgramState* getSharedProgramState(void) const;
  void setSharedProgramState(SharedProgramState* newState);
  /*! @return The current refresh mode.
   */
  RefreshMode getRefreshMode(void) const;
  /*! Sets the refresh mode.
   *  @param[in] newMode The desired new refresh mode.
   */
  void setRefreshMode(RefreshMode newMode);
  /*! @return The current scissor rectangle.
   */
  const Rect& getScissorArea(void) const;
  void setScissorArea(const Rect& newArea);
  /*! @return The current viewport rectangle.
   */
  const Rect& getViewportArea(void) const;
  /*! Sets the current viewport rectangle.
   *  @param[in] newArea The desired viewport rectangle.
   */
  void setViewportArea(const Rect& newArea);
  /*! @return The current canvas.
   */
  Canvas& getCurrentCanvas(void) const;
  /*! @return The screen canvas.
   */
  ScreenCanvas& getScreenCanvas(void) const;
  /*! Sets the screen canvas as the current canvas.
   */
  void setScreenCanvasCurrent(void);
  /*! Makes the specified canvas the current canvas.
   *  @param[in] newCanvas The desired canvas.
   *  @return @c true if successful, or @c false otherwise.
   */
  bool setCurrentCanvas(Canvas& newCanvas);
  Program* getCurrentProgram(void) const;
  /*! Sets the current shader program for use when rendering.
   *  @param[in] newProgram The desired shader program, or @c NULL to detach
   *  the current shader program.
   */
  void setCurrentProgram(Program* newProgram);
  VertexBuffer* getCurrentVertexBuffer(void) const;
  void setCurrentVertexBuffer(VertexBuffer* newVertexBuffer);
  IndexBuffer* getCurrentIndexBuffer(void) const;
  void setCurrentIndexBuffer(IndexBuffer* newIndexBuffer);
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  Texture* getCurrentTexture(void) const;
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  void setCurrentTexture(Texture* newTexture);
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  unsigned int getActiveTextureUnit(void) const;
  /*! @note Unless you are Wendy, you probably don't need to call this.
   */
  void setActiveTextureUnit(unsigned int unit);
  const PlaneList& getClipPlanes(void) const;
  bool setClipPlanes(const PlaneList& newPlanes);
  Stats* getStats(void) const;
  void setStats(Stats* newStats);
  /*! @return The title of the context window.
   */
  const String& getTitle(void) const;
  /*! Sets the title of the context window.
   *  @param[in] newTitle The desired title.
   */
  void setTitle(const String& newTitle);
  /*! @return The limits of this context.
   */
  const Limits& getLimits(void) const;
  /*! @return The resource index used by this context.
   */
  ResourceIndex& getIndex(void) const;
  /*! @return The signal for per-frame post-render clean-up.
   */
  SignalProxy0<void> getFinishSignal(void);
  /*! @return The signal for user-initiated close requests.
   */
  SignalProxy0<bool> getCloseRequestSignal(void);
  /*! @return The signal for context resizing.
   */
  SignalProxy2<void, unsigned int, unsigned int> getResizedSignal(void);
  /*! Creates the context singleton object, using the specified settings.
    *  @param[in] mode The requested context settings.
    *  @return @c true if successful, or @c false otherwise.
    */
  static bool createSingleton(ResourceIndex& index, const ContextMode& mode = ContextMode());
  /*! @return The signal for creation of a context object.
   */
  static SignalProxy0<void> getCreateSignal(void);
  /*! @return The signal for destruction of a context object.
   */
  static SignalProxy0<void> getDestroySignal(void);
  /*! Retrieves the supported screen modes.
   *  @param[out] result The supported modes.
   */
  static void getScreenModes(ScreenModeList& result);
private:
  Context(ResourceIndex& index);
  Context(const Context& source);
  Context& operator = (const Context& source);
  bool init(const ContextMode& mode);
  void updateScissorArea(void);
  void updateViewportArea(void);
  static void sizeCallback(int width, int height);
  static int closeCallback(void);
  static void refreshCallback(void);
  typedef std::vector<SharedSampler> SamplerList;
  typedef std::vector<SharedUniform> UniformList;
  ResourceIndex& index;
  Signal0<void> finishSignal;
  Signal0<bool> closeRequestSignal;
  Signal2<void, unsigned int, unsigned int> resizedSignal;
  String title;
  Ptr<Limits> limits;
  RefreshMode refreshMode;
  bool needsRefresh;
  bool needsClosing;
  Rect scissorArea;
  Rect viewportArea;
  bool dirtyBinding;
  SharedProgramState* state;
  SamplerList samplers;
  UniformList uniforms;
  TextureList textureUnits;
  unsigned int activeTextureUnit;
  Ref<Program> currentProgram;
  Ref<VertexBuffer> currentVertexBuffer;
  Ref<IndexBuffer> currentIndexBuffer;
  PlaneList planes;
  Stats* stats;
  Ref<Canvas> currentCanvas;
  Ref<ScreenCanvas> screenCanvas;
  static Context* instance;
  static Signal0<void> createSignal;
  static Signal0<void> destroySignal;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLCONTEXT_H*/
///////////////////////////////////////////////////////////////////////
