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
#ifndef WENDY_GLCONTEXT_H
#define WENDY_GLCONTEXT_H
///////////////////////////////////////////////////////////////////////

#include <GL/glfw.h>

///////////////////////////////////////////////////////////////////////

namespace moira
{
  class Image;
}

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
  Key(unsigned int value);
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
  ContextMode(unsigned int width,
	      unsigned int height,
	      unsigned int colorBits,
	      unsigned int depthBits = 0,
	      unsigned int stencilBits = 0,
	      unsigned int flags = 0);
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
  ~Context(void);
  /*! Updates the screen.
   */
  bool update(void);
  /*! Finds the entry point with the specified name.
   *  @param name The name of the entry point.
   *  @return The address of the specified entry point, or @c NULL.
   */
  EntryPoint findEntryPoint(const std::string& name);
  /*! @return @c true if the context is windowed, otherwise @c false.
   */
  bool isWindowed(void) const;
  /*! @return @c true if the specified key is pressed, otherwise @c false.
   *  @param key The desired key.
   */
  bool isKeyDown(const Key& key) const;
  /*! @return @c true if the specified mouse button is pressed, otherwise @c false.
   *  @param button The desired mouse button.
   */
  bool isButtonDown(unsigned int button) const;
  /*! Checks for the presence of the specified extension.
   *  @param name The name of the desired extension.
   *  @return @c true if the extension is present, otherwise @c false.
   */
  bool hasExtension(const std::string& name) const;
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
   */
  Image* getColorBuffer(void) const;
  /*! @return The title of the context window.
   */
  const std::string& getTitle(void) const;
  /*! Sets the title of the context window.
   *  @param newTitle The desired title.
   */
  void setTitle(const std::string& newTitle);
  /*! @return The current mouse position.
   */
  const Point2& getMousePosition(void) const;
  /*! Places the the mouse cursor at the specified position.
   *  @param position The desired mouse position.
   */
  void setMousePosition(const Point2& newPosition);
  /*! Creates the context singleton object, using the specified settings.
    *  @param mode The requested context settings.
    *  @return @c true if successful, or @c false otherwise.
    */
  static bool create(const ContextMode& mode);
private:
  Context(void);
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
  static Context* instance;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLCONTEXT_H*/
///////////////////////////////////////////////////////////////////////
