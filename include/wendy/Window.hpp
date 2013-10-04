///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2009 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_WINDOW_HPP
#define WENDY_WINDOW_HPP
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.hpp>
#include <wendy/Transform.hpp>
#include <wendy/Signal.hpp>

///////////////////////////////////////////////////////////////////////

typedef struct GLFWwindow GLFWwindow;

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
    class Context;
  }
}

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
    class Context;
  }
}

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief Keyboard key enumeration.
 */
enum Key
{
  KEY_SPACE            = 32,
  KEY_APOSTROPHE       = 39,
  KEY_COMMA            = 44,
  KEY_MINUS            = 45,
  KEY_PERIOD           = 46,
  KEY_SLASH            = 47,
  KEY_0                = 48,
  KEY_1                = 49,
  KEY_2                = 50,
  KEY_3                = 51,
  KEY_4                = 52,
  KEY_5                = 53,
  KEY_6                = 54,
  KEY_7                = 55,
  KEY_8                = 56,
  KEY_9                = 57,
  KEY_SEMICOLON        = 59,
  KEY_EQUAL            = 61,
  KEY_A                = 65,
  KEY_B                = 66,
  KEY_C                = 67,
  KEY_D                = 68,
  KEY_E                = 69,
  KEY_F                = 70,
  KEY_G                = 71,
  KEY_H                = 72,
  KEY_I                = 73,
  KEY_J                = 74,
  KEY_K                = 75,
  KEY_L                = 76,
  KEY_M                = 77,
  KEY_N                = 78,
  KEY_O                = 79,
  KEY_P                = 80,
  KEY_Q                = 81,
  KEY_R                = 82,
  KEY_S                = 83,
  KEY_T                = 84,
  KEY_U                = 85,
  KEY_V                = 86,
  KEY_W                = 87,
  KEY_X                = 88,
  KEY_Y                = 89,
  KEY_Z                = 90,
  KEY_LEFT_BRACKET     = 91,
  KEY_BACKSLASH        = 92,
  KEY_RIGHT_BRACKET    = 93,
  KEY_GRAVE_ACCENT     = 96,
  KEY_WORLD_1          = 161,
  KEY_WORLD_2          = 162,
  KEY_ESCAPE           = 256,
  KEY_ENTER            = 257,
  KEY_TAB              = 258,
  KEY_BACKSPACE        = 259,
  KEY_INSERT           = 260,
  KEY_DELETE           = 261,
  KEY_RIGHT            = 262,
  KEY_LEFT             = 263,
  KEY_DOWN             = 264,
  KEY_UP               = 265,
  KEY_PAGE_UP          = 266,
  KEY_PAGE_DOWN        = 267,
  KEY_HOME             = 268,
  KEY_END              = 269,
  KEY_CAPS_LOCK        = 280,
  KEY_SCROLL_LOCK      = 281,
  KEY_NUM_LOCK         = 282,
  KEY_PRINT_SCREEN     = 283,
  KEY_PAUSE            = 284,
  KEY_F1               = 290,
  KEY_F2               = 291,
  KEY_F3               = 292,
  KEY_F4               = 293,
  KEY_F5               = 294,
  KEY_F6               = 295,
  KEY_F7               = 296,
  KEY_F8               = 297,
  KEY_F9               = 298,
  KEY_F10              = 299,
  KEY_F11              = 300,
  KEY_F12              = 301,
  KEY_F13              = 302,
  KEY_F14              = 303,
  KEY_F15              = 304,
  KEY_F16              = 305,
  KEY_F17              = 306,
  KEY_F18              = 307,
  KEY_F19              = 308,
  KEY_F20              = 309,
  KEY_F21              = 310,
  KEY_F22              = 311,
  KEY_F23              = 312,
  KEY_F24              = 313,
  KEY_F25              = 314,
  KEY_KP_0             = 320,
  KEY_KP_1             = 321,
  KEY_KP_2             = 322,
  KEY_KP_3             = 323,
  KEY_KP_4             = 324,
  KEY_KP_5             = 325,
  KEY_KP_6             = 326,
  KEY_KP_7             = 327,
  KEY_KP_8             = 328,
  KEY_KP_9             = 329,
  KEY_KP_DECIMAL       = 330,
  KEY_KP_DIVIDE        = 331,
  KEY_KP_MULTIPLY      = 332,
  KEY_KP_SUBTRACT      = 333,
  KEY_KP_ADD           = 334,
  KEY_KP_ENTER         = 335,
  KEY_KP_EQUAL         = 336,
  KEY_LEFT_SHIFT       = 340,
  KEY_LEFT_CONTROL     = 341,
  KEY_LEFT_ALT         = 342,
  KEY_LEFT_SUPER       = 343,
  KEY_RIGHT_SHIFT      = 344,
  KEY_RIGHT_CONTROL    = 345,
  KEY_RIGHT_ALT        = 346,
  KEY_RIGHT_SUPER      = 347,
  KEY_MENU             = 348
};

///////////////////////////////////////////////////////////////////////

/*! @brief Key modifier bits.
 */
enum Modifier
{
  MOD_SHIFT   = 0x01,
  MOD_CONTROL = 0x02,
  MOD_ALT     = 0x04,
  MOD_SUPER   = 0x08
};

///////////////////////////////////////////////////////////////////////

/* @brief Mouse button enumeration.
 */
enum MouseButton
{
  MOUSE_BUTTON_LEFT,
  MOUSE_BUTTON_RIGHT,
  MOUSE_BUTTON_MIDDLE
};

///////////////////////////////////////////////////////////////////////

/*! @brief Input action enumeration.
 */
enum Action
{
  RELEASED,
  PRESSED,
  REPEATED
};

///////////////////////////////////////////////////////////////////////

/*! @brief Input hook interface.
 *
 *  This is intended for hotkeys that should work regardless of which target
 *  currently has focus, such as a key to bring down the console.  It gets first
 *  pick of any input and can prevent it from being passed on to the current
 *  target.
 */
class EventHook
{
public:
  /*! Destructor.
   */
  virtual ~EventHook();
  /*! Called when the window has been resized.
   */
  virtual void onWindowSize(uint width, uint height);
  /*! Called when the window has been resized.
   */
  virtual void onWindowDamage();
  /*! Called when the window is requested to close.
   */
  virtual void onWindowCloseRequest();
  /*! Called when a key has been pressed or released.
   *  @return @c true to prevent this event from reaching the current input
   *  target, or @c false to pass it on.
   */
  virtual bool onKey(Key key, Action action, uint mods);
  /*! Called when a Unicode character has been input.
   *  @return @c true to prevent this event from reaching the current input
   *  target, or @c false to pass it on.
   */
  virtual bool onCharacter(uint32 character, uint mods);
  /*! Called when a mouse button has been clicked or released.
   *  @return @c true to prevent this event from reaching the current input
   *  target, or @c false to pass it on.
   */
  virtual bool onMouseButton(MouseButton button, Action action, uint mods);
  /*! Called when the mouse cursor has been moved.
   *  @return @c true to prevent this event from reaching the current input
   *  target, or @c false to pass it on.
   */
  virtual bool onCursorPos(vec2 position);
  /*! Called when a scrolling device has been used.
   *  @return @c true to prevent this event from reaching the current input
   *  target, or @c false to pass it on.
   */
  virtual bool onScroll(vec2 offset);
};

///////////////////////////////////////////////////////////////////////

/*! @brief Event target interface.
 *
 *  This is intended for use by game modules such as menus, editors, the console
 *  and the game itself.
 */
class EventTarget
{
public:
  /*! Destructor.
   */
  virtual ~EventTarget();
  /*! Called when the window has been resized.
   */
  virtual void onWindowSize(uint width, uint height);
  /*! Called when the window has been resized.
   */
  virtual void onWindowDamage();
  /*! Called when the window is requested to close.
   */
  virtual void onWindowCloseRequest();
  /*! Called when a key has been pressed or released.
   */
  virtual void onKey(Key key, Action action, uint mods);
  /*! Called when a Unicode character has been input.
   */
  virtual void onCharacter(uint32 character, uint mods);
  /*! Called when a mouse button has been clicked or released.
   */
  virtual void onMouseButton(MouseButton button, Action action, uint mods);
  /*! Called when the mouse cursor has been moved.
   */
  virtual void onCursorPos(vec2 position);
  /*! Called when a scrolling device has been used.
   */
  virtual void onScroll(vec2 offset);
  /*! Called when this input target has lost or gained focus.
   */
  virtual void onFocus(bool activated);
};

///////////////////////////////////////////////////////////////////////

/*! Refresh mode enumeration.
 */
enum RefreshMode
{
  /*! The Window::update method does not block.
   */
  AUTOMATIC_REFRESH,
  /*! The Window::update method blocks until the window is damaged or
   * invalidated.
   */
  MANUAL_REFRESH
};

///////////////////////////////////////////////////////////////////////

/*! @brief Window mode enumeration.
 */
enum WindowMode
{
  WINDOWED,
  FULLSCREEN
};

///////////////////////////////////////////////////////////////////////

/*! @brief Monitor resolution.
 */
class Resolution
{
public:
  Resolution();
  Resolution(uint width, uint height);
  uint width;
  uint height;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Window configuration.
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
               uint width,
               uint height,
               WindowMode mode,
               bool resizable = true);
  /*! The desired window title.
   */
  String title;
  /*! The desired width of the window.
   */
  uint width;
  /*! The desired height of the window.
   */
  uint height;
  /*! The desired mode of the window.
   */
  WindowMode mode;
  /*! @c true if the window should be resizable, @c false otherwise.
   */
  bool resizable;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Window.
 */
class Window
{
public:
  friend GL::Context;
  /*! Destructor.
   */
  virtual ~Window();
  bool update();
  void invalidate();
  void captureCursor();
  void releaseCursor();
  /*! @return @c true if the specified key is pressed, otherwise @c false.
   *  @param[in] key The desired key.
   */
  bool isKeyDown(Key key) const;
  /*! @return @c true if the specified mouse button is pressed, otherwise @c false.
   *  @param[in] button The desired mouse button.
   */
  bool isButtonDown(MouseButton button) const;
  bool isCursorCaptured() const;
  bool shouldClose() const;
  void setShouldClose(bool newValue);
  /*! @return The mode of this window.
   */
  WindowMode mode() const;
  /*! Sets the title of the context window.
   *  @param[in] newTitle The desired title.
   */
  void setTitle(const char* newTitle);
  /*! @return The width, in pixels, of the mousable screen area.
   */
  uint width() const;
  /*! @return The height, in pixels, of the mousable screen area.
   */
  uint height() const;
  /*! @return The current refresh mode.
   */
  RefreshMode refreshMode() const { return m_refreshMode; }
  /*! Sets the refresh mode.
   *  @param[in] newMode The desired new refresh mode.
   */
  void setRefreshMode(RefreshMode newMode);
  /*! @return The current mouse position.
   */
  vec2 cursorPosition() const;
  /*! Places the the mouse cursor at the specified position.
   *  @param[in] newPosition The desired mouse position.
   */
  void setCursorPosition(vec2 newPosition);
  String clipboardText() const;
  void setClipboardText(const String& newText);
  /*! @return The signal for per-frame post-render clean-up.
   */
  SignalProxy0<void> frameSignal();
  EventHook* hook() const { return m_hook; }
  void setHook(EventHook* newHook);
  EventTarget* target() const { return m_target; }
  void setTarget(EventTarget* newTarget);
  static std::vector<Resolution> resolutions();
private:
  Window();
  Window(const Window&) = delete;
  void init(GLFWwindow* handle);
  Window& operator = (const Window&) = delete;
  static void sizeCallback(GLFWwindow* handle, int width, int height);
  static void damageCallback(GLFWwindow* handle);
  static void closeCallback(GLFWwindow* handle);
  static void keyCallback(GLFWwindow* handle, int key, int scancode, int action, int mods);
  static void characterCallback(GLFWwindow* handle, uint character);
  static void cursorPosCallback(GLFWwindow* handle, double x, double y);
  static void mouseButtonCallback(GLFWwindow* handle, int button, int action, int mods);
  static void scrollCallback(GLFWwindow* handle, double x, double y);
  GLFWwindow* m_handle;
  bool m_needsRefresh;
  RefreshMode m_refreshMode;
  EventHook* m_hook;
  EventTarget* m_target;
  Signal0<void> m_frameSignal;
};

///////////////////////////////////////////////////////////////////////

class SpectatorController
{
public:
  SpectatorController();
  void update(Time deltaTime);
  void release();
  void inputKey(Key key, Action action, uint mods);
  void inputMouseButton(MouseButton button, Action action, uint mods);
  void inputCursorOffset(vec2 offset);
  const Transform3& getTransform() const;
  void setPosition(vec3 newPosition);
  float getAngleX() const;
  float getAngleY() const;
  void setRotation(float newAngleX, float newAngleY);
  float getSpeed() const;
  void setSpeed(float newSpeed);
private:
  enum Direction
  {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    FORWARD,
    BACK
  };
  Transform3 transform;
  float angleX;
  float angleY;
  float speed;
  bool directions[6];
  bool turbo;
};

///////////////////////////////////////////////////////////////////////

class TextController
{
public:
  TextController();
  TextController(const String& text);
  void inputKey(Key key, Action action, uint mods);
  void inputCharacter(uint32 character, uint mods);
  const String& getText() const;
  void setText(const String& newText);
  size_t getCaretPosition() const;
  void setCaretPosition(size_t newPosition);
  SignalProxy0<void> getCaretMovedSignal();
  SignalProxy0<void> getTextChangedSignal();
private:
  String text;
  size_t caretPosition;
  Signal0<void> textChangedSignal;
  Signal0<void> caretMovedSignal;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_WINDOW_HPP*/
///////////////////////////////////////////////////////////////////////
