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
   */
  ScreenMode(unsigned int width, unsigned int height, unsigned int colorBits);
  /*! Resets all value to their defaults.
   */
  void setDefaults(void);
  /*! Sets the 
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
  /*! Sets the 
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
  friend class VertexProgram;
  friend class FragmentProgram;
public:
  /*! Destructor.
   */
  ~Context(void);
  /*! Updates the screen.
   */
  bool update(void);
  /*! @return @c true if the context is windowed, otherwise @c false.
   */
  bool isWindowed(void) const;
  /*! @return The default framebuffer width, in pixels.
   */
  unsigned int getWidth(void) const;
  /*! @return The default framebuffer height, in pixels.
   */
  unsigned int getHeight(void) const;
  /*! @return The default framebuffer color depth, in bits.
   */
  unsigned int getColorBits(void) const;
  /*! @return The default framebuffer depth-buffer depth, in bits.
   */
  unsigned int getDepthBits(void) const;
  /*! @return The default framebuffer stencil buffer depth, in bits.
   */
  unsigned int getStencilBits(void) const;
  /*! @return A copy of the current state of the default framebuffer color buffer.
   *  @todo Rename this to reflect object ownership transfer.
   */
  Image* getColorBuffer(void) const;
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
  static void sizeCallback(int width, int height);
  static int closeCallback(void);
  Signal0<void> finishSignal;
  Signal0<bool> closeRequestSignal;
  Signal2<void, unsigned int, unsigned int> resizedSignal;
  ContextMode mode;
  String title;
  Ptr<Limits> limits;
  void* cgContextID;
  int cgVertexProfile;
  int cgFragmentProfile;
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
