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

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Context;

///////////////////////////////////////////////////////////////////////

/*! @defgroup opengl OpenGL wrapper API
 *
 *  These classes wrap parts of the OpenGL API, maintaining a rather close
 *  mapping to the underlying concepts, but providing useful services and a
 *  semblance of automatic resource management. They are used by most
 *  higher-level components such as the 3D rendering pipeline.
 */

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
  ALLOW_GREATER_EQUAL,
};

///////////////////////////////////////////////////////////////////////

/*! @brief Primitive type enumeration.
 *  @ingroup opengl
 */
enum PrimitiveType
{
  POINT_LIST,
  LINE_LIST,
  LINE_STRIP,
  LINE_LOOP,
  TRIANGLE_LIST,
  TRIANGLE_STRIP,
  TRIANGLE_FAN,
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
  enum {
    /*! Create a windowed context, if supported.
     */
    WINDOWED = 1,
    /*! Default flags.
     */
    DEFAULT = WINDOWED,
  };
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
	      unsigned int flags = DEFAULT);
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
	   unsigned int newFlags = DEFAULT);
  /*! The desired depth buffer bit depth.
   */
  unsigned int depthBits;
  /*! The desired stencil buffer bit depth.
   */
  unsigned int stencilBits;
  /*! The desired number of FSAA samples.
   */
  unsigned int samples;
  /*! The desired modification flags.
   */
  unsigned int flags;
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
  /*! @return The number of available clip planes.
   */
  unsigned int getMaxClipPlanes(void) const;
  /*! @return The number of available fragment program texture image units.
   */
  unsigned int getMaxFragmentTextureImageUnits(void) const;
  /*! @return The number of available vertex program texture image units.
   */
  unsigned int getMaxVertexTextureImageUnits(void) const;
  /*! @return The maximum size, in pixels, of 2D POT textures.
   */
  unsigned int getMaxTextureSize(void) const;
  /*! @return The maximum size, in pixels, of cube map texture faces.
   */
  unsigned int getMaxTextureCubeSize(void) const;
  /*! @return The maximum size, in pixels, of non-POT 2D textures.
   */
  unsigned int getMaxTextureRectangleSize(void) const;
  /*! @return The number of available vertex attributes.
   */
  unsigned int getMaxVertexAttributes(void) const;
private:
  Context& context;
  unsigned int maxClipPlanes;
  unsigned int maxFragmentTextureImageUnits;
  unsigned int maxVertexTextureImageUnits;
  unsigned int maxTextureSize;
  unsigned int maxTextureCubeSize;
  unsigned int maxTextureRectangleSize;
  unsigned int maxVertexAttributes;
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
private:
  Canvas(const Canvas& source);
  Canvas& operator = (const Canvas& source);
  Context& context;
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

/*! @ingoup opengl
 */
class Image : public RefObject
{
public:
  virtual ~Image(void);
  virtual unsigned int getWidth(void) const = 0;
  virtual unsigned int getHeight(void) const = 0;
  virtual const PixelFormat& getFormat(void) const = 0;
protected:
  virtual void attach(int attachment) = 0;
  virtual void detach(void) = 0;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
typedef Ref<Image> ImageRef;

///////////////////////////////////////////////////////////////////////

/*! @brief %Canvas for rendering to a texture.
 *  @ingroup opengl
 */
class ImageCanvas : public Canvas
{
public:
  unsigned int getWidth(void) const;
  unsigned int getHeight(void) const;
  /*! @return The texture that this canvas uses as a color buffer.
   */
  Image* getColorBuffer(void) const;
  Image* getDepthBuffer(void) const;
  /*! Sets the image to use as the color buffer for this canvas.
   *  @param[in] newImage The desired image, or @c NULL to detach the currently
   *  set image.
   */
  bool setColorBuffer(Image* newImage);
  bool setDepthBuffer(Image* newImage);
  /*! Creates a texture canvas for the specified texture.
   */
  static ImageCanvas* createInstance(Context& context,
                                     unsigned int width,
                                     unsigned int height);
private:
  ImageCanvas(Context& context);
  bool init(unsigned int width, unsigned int height);
  void apply(void) const;
  unsigned int width;
  unsigned int height;
  unsigned int bufferID;
  ImageRef colorBuffer;
  ImageRef depthBuffer;
};

///////////////////////////////////////////////////////////////////////

/*! @brief OpenGL context singleton.
 *  @ingroup opengl
 *
 *  This class encapsulates the OpenGL context and its associtated window.  It
 *  also initializes the GLEW library, allowing use of the GLEW booleans in
 *  client code for the entire lifetime of the context.
 */
class Context : public Singleton<Context>
{
  friend class Texture;
  friend class TextureImage;
  friend class VertexProgram;
  friend class FragmentProgram;
public:
  /*! Destructor.
   */
  ~Context(void);
  /*! Clears the current color buffer with the specified color.
   *  @param[in] color The color value to clear the color buffer with.
   */
  void clearColorBuffer(const ColorRGBA& color = ColorRGBA::BLACK);
  /*! Clears the current depth buffer with the specified depth value.
   *  @param[in] depth The depth value to clear the depth buffer with.
   */
  void clearDepthBuffer(float depth = 1.f);
  /*! Clears the current stencil buffer with the specified stencil value.
   *  @param[in] value The stencil value to clear the stencil buffer with.
   */
  void clearStencilBuffer(unsigned int value = 0);
  /*! Updates the screen.
   */
  bool update(void);
  /*! @return The current scissor rectangle.
   */
  const Rect& getScissorArea(void) const;
  /*! @return The current viewport rectangle.
   */
  const Rect& getViewportArea(void) const;
  void setScissorArea(const Rect& newArea);
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
  static bool create(const ContextMode& mode);
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
  Context(void);
  Context(const Context& source);
  Context& operator = (const Context& source);
  bool init(const ContextMode& mode);
  void updateScissorArea(void);
  void updateViewportArea(void);
  static void sizeCallback(int width, int height);
  static int closeCallback(void);
  Signal0<void> finishSignal;
  Signal0<bool> closeRequestSignal;
  Signal2<void, unsigned int, unsigned int> resizedSignal;
  String title;
  Ptr<Limits> limits;
  void* cgContextID;
  int cgVertexProfile;
  int cgFragmentProfile;
  Rect scissorArea;
  Rect viewportArea;
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
