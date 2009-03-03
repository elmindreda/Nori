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

#include <Cg/cg.h>
#include <Cg/cgGL.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @brief Input key.
 *
 *  Represents a single input key (as on a keyboard).
 */
class Key
{
public:
  enum Symbol
  {
    SPACE = 32,
    ESCAPE = 256, TAB, ENTER, BACKSPACE, INSERT, DELETE, 
    LSHIFT, RSHIFT, LCTRL, RCTRL, LALT, RALT,
    UP, DOWN, LEFT, RIGHT, PAGEUP, PAGEDOWN, HOME, END,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
  };
  Key(unsigned int value);
  operator unsigned int (void) const;
private:
  unsigned int value;
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
  friend class VertexShader;
  friend class FragmentShader;
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
  /*! @return @c true if the specified key is pressed, otherwise @c false.
   *  @param[in] key The desired key.
   */
  bool isKeyDown(const Key& key) const;
  /*! @return @c true if the specified mouse button is pressed, otherwise @c false.
   *  @param[in] button The desired mouse button.
   */
  bool isButtonDown(unsigned int button) const;
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
  /*! @return The current mouse position.
   */
  const Vector2& getCursorPosition(void) const;
  /*! Places the the mouse cursor at the specified position.
   *  @param[in] newPosition The desired mouse position.
   */
  void setCursorPosition(const Vector2& newPosition);
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
  /*! @return The signal for key press and release events.
   */
  SignalProxy2<void, Key, bool> getKeyPressedSignal(void);
  /*! @return The signal for character input events.
   */
  SignalProxy1<void, wchar_t> getCharInputSignal(void);
  /*! @return The signal for mouse button click and release events.
   */
  SignalProxy2<void, unsigned int, bool> getButtonClickedSignal(void);
  /*! @return The signal for mouse cursor movement events.
   */
  SignalProxy1<void, const Vector2&> getCursorMovedSignal(void);
  /*! @return The signal for mouse wheel events.
    */
  SignalProxy1<void, int> getWheelTurnedSignal(void);
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
  static void keyboardCallback(int key, int action);
  static void characterCallback(int character, int action);
  static void mousePosCallback(int x, int y);
  static void mouseButtonCallback(int button, int action);
  static void mouseWheelCallback(int position);
  typedef std::map<int, int> KeyMap;
  Signal0<bool> renderSignal;
  Signal0<void> finishSignal;
  Signal0<bool> closeRequestSignal;
  Signal2<void, unsigned int, unsigned int> resizedSignal;
  Signal2<void, Key, bool> keyPressedSignal;
  Signal1<void, wchar_t> charInputSignal;
  Signal2<void, unsigned int, bool> buttonClickedSignal;
  Signal1<void, const Vector2&> cursorMovedSignal;
  Signal1<void, int> wheelTurnedSignal;
  ContextMode mode;
  String title;
  int wheelPosition;
  CGcontext cgContextID;
  CGprofile cgVertexProfile;
  CGprofile cgFragmentProfile;
  mutable Vector2 cursorPosition;
  static KeyMap internalMap;
  static KeyMap externalMap;
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
