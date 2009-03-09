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

/*! @brief OpenGL context singleton.
 *  @ingroup opengl
 *
 *  This class encapsulates the OpenGL context, provides display mode
 *  control and basic HID (input) signals.  It also initializes the
 *  GLEW library, allowing use of the GLEW booleans in client code
 *  for the entire lifetime of the context.
 */
class Context : public Singleton<Context>
{
public:
  /*! Destructor.
   */
  ~Context(void);
  /*! Updates the screen.
   */
  bool update(void);
  /*! Finds the entry point with the specified name.
   *  @param[in] name The name of the entry point.
   *  @return The address of the specified entry point, or @c NULL.
   */
  EntryPoint findEntryPoint(const String& name);
  /*! @return @c true if the context is windowed, otherwise @c false.
   */
  bool isWindowed(void) const;
  /*! Checks for the presence of the specified extension.
   *  @param[in] name The name of the desired extension.
   *  @return @c true if the extension is present, otherwise @c false.
   */
  bool hasExtension(const String& name) const;
  /*! @return The width, in pixels.
   */
  unsigned int getWidth(void) const;
  /*! @return The height, in pixels.
   */
  unsigned int getHeight(void) const;
  /*! @return The color depth, in bits.
   */
  unsigned int getColorBits(void) const;
  /*! @return The depth-buffer depth, in bits.
   */
  unsigned int getDepthBits(void) const;
  /*! @return The stencil buffer depth, in bits.
   */
  unsigned int getStencilBits(void) const;
  /*! @return A copy of the current state of the color buffer.
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
  /*! @return The signal for rendering.
   */
  SignalProxy0<bool> getRenderSignal(void);
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
  Signal0<bool> renderSignal;
  Signal0<void> finishSignal;
  Signal0<bool> closeRequestSignal;
  Signal2<void, unsigned int, unsigned int> resizedSignal;
  ContextMode mode;
  String title;
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
