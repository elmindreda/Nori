///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@home.se>
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
#ifndef WEGLCONTEXT_H
#define WEGLCONTEXT_H
///////////////////////////////////////////////////////////////////////

#include <GL/glfw.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Key
{
public:
  enum
  {
    SPACE = 32,
    ESCAPE = 256, TAB, ENTER, BACKSPACE, INSERT, DELETE, 
    LSHIFT, RSHIFT, LCTRL, RCTRL, LALT, RALT,
    UP, DOWN, LEFT, RIGHT, PAGEUP, PAGEDOWN, HOME, END,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
  };
  Key(unsigned int initValue);
  operator unsigned int (void) const;
private:
  unsigned int value;
};

///////////////////////////////////////////////////////////////////////

class ContextMode
{
public:
  /*! Default constructor.
   */
  ContextMode(void);
  /*! Constructor.
   */
  ContextMode(unsigned int initWidth,
	      unsigned int initHeight,
	      unsigned int initColorBits,
	      unsigned int initDepthBits = 0,
	      unsigned int initStencilBits = 0,
	      unsigned int initFlags = 0);
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
	   unsigned int newFlags = 0);
  /*! The desired width of the context.
   */
  unsigned int width;
  /*! The desired height of the context.
   */
  unsigned int height;
  /*! The desired color buffer bit depth.
   */
  unsigned int colorBits;
  /*! The desired depth buffer bit depth.
   */
  unsigned int depthBits;
  /*! The desired stencil buffer bit depth.
   */
  unsigned int stencilBits;
  /*! The desired modification flags.
   */
  unsigned int flags;
  enum {
    /*! Create a windowed context, if supported.
     */
    WINDOWED = 1,
  };
};

///////////////////////////////////////////////////////////////////////

class ContextObserver : public Observer<ContextObserver>
{
  friend class Context;
  friend class GLFWContext;
protected:
  virtual void onContextResize(unsigned int width, unsigned int height);
  virtual bool onContextUpdate(void);
  virtual bool onContextClose(void);
  virtual void onContextKeyEvent(Key key, bool pressed);
  virtual void onContextMouseMove(const Point2& position);
  virtual void onContextMouseClick(unsigned int button, bool clicked);
};

///////////////////////////////////////////////////////////////////////

class Context : public Observable<ContextObserver>, public Singleton<Context>
{
public:
  /*! Destructor.
   */
  virtual ~Context(void);
  /*! Updates the screen.
   */
  virtual bool update(void) = 0;
  /*! Finds the entry point with the specified name.
   *  @param name [in] The name of the entry point.
   *  @return The address of the specified entry point, or @c NULL.
   */
  virtual EntryPoint findEntryPoint(const std::string& name) = 0;
  /*! @return @c true if the context is windowed, otherwise @c false.
   */
  virtual bool isWindowed(void) const = 0;
  /*! @return @c true if the specified key is pressed, otherwise @c false.
   *  @param key The desired key.
   */
  virtual bool isKeyDown(const Key& key) const = 0;
  /*! @return @c true if the specified mouse button is pressed, otherwise @c false.
   *  @param button The desired mouse button.
   */
  virtual bool isButtonDown(unsigned int button) const = 0;
  /*! Checks for the presence of the specified extension.
   *  @param name [in] The name of the desired extension.
   *  @return @c true if the extension is present, otherwise @c false.
   */
  virtual bool hasExtension(const std::string& name) const = 0;
  /*! @return The width, in pixels.
   */
  virtual unsigned int getWidth(void) const = 0;
  /*! @return The height, in pixels.
   */
  virtual unsigned int getHeight(void) const = 0;
  /*! @return The color depth, in bits.
   */
  virtual unsigned int getColorBits(void) const = 0;
  /*! @return The depth-buffer depth, in bits.
   */
  virtual unsigned int getDepthBits(void) const = 0;
  /*! @return The stencil buffer depth, in bits.
   */
  virtual unsigned int getStencilBits(void) const = 0;
  /*! @return The title of the context window.
   */
  virtual const std::string& getTitle(void) const = 0;
  /*! Sets the title of the context window.
   *  @param title [in] The desired title.
   */
  virtual void setTitle(const std::string& title) = 0;
  /*! @return The current mouse position.
   */
  virtual const Point2& getMousePosition(void) const = 0;
  /*! Places the the mouse cursor at the specified position.
   *  @param position The desired mouse position.
   */
  virtual void setMousePosition(const Point2& position) = 0;
  /*! Creates a context singleton object with the specified settings.
    *  @param mode [in] The requested context settings.
    *  @return @c true if successful, or @c false otherwise.
    */
  static bool create(const ContextMode& mode);
};

///////////////////////////////////////////////////////////////////////

class GLFWContext : public Context
{
public:
  ~GLFWContext(void);
  bool update(void);
  EntryPoint findEntryPoint(const std::string& name);
  bool isWindowed(void) const;
  bool isKeyDown(const Key& key) const;
  bool isButtonDown(unsigned int button) const;
  bool hasExtension(const std::string& name) const;
  unsigned int getWidth(void) const;
  unsigned int getHeight(void) const;
  unsigned int getColorBits(void) const;
  unsigned int getDepthBits(void) const;
  unsigned int getStencilBits(void) const;
  const std::string& getTitle(void) const;
  void setTitle(const std::string& newTitle);
  const Point2& getMousePosition(void) const;
  void setMousePosition(const Point2& position);
  /*! Creates a GLFW context singleton object with the specified settings.
    *  @param mode [in] The requested context settings.
    *  @return @c true if successful, or @c false otherwise.
    */
  static bool create(const ContextMode& mode);
private:
  GLFWContext(void);
  bool init(const ContextMode& mode);
  static void GLFWCALL sizeCallback(int width, int height);
  static int GLFWCALL closeCallback(void);
  static void GLFWCALL keyboardCallback(int key, int action);
  static void GLFWCALL mousePosCallback(int x, int y);
  static void GLFWCALL mouseButtonCallback(int button, int action);
  typedef std::map<int, int> KeyMap;
  ContextMode mode;
  std::string title;
  static KeyMap internalMap;
  static KeyMap externalMap;
  static GLFWContext* instance;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WEGLCONTEXT_H*/
///////////////////////////////////////////////////////////////////////
