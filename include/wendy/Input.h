///////////////////////////////////////////////////////////////////////
// Wendy input library
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
#ifndef WENDY_INPUT_H
#define WENDY_INPUT_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Transform.h>
#include <wendy/Signal.h>

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
  namespace input
  {

///////////////////////////////////////////////////////////////////////

/*! @brief Keyboard key enumeration.
 *  @ingroup input
 */
enum Key
{
  KEY_SPACE = 32,
  KEY_ESCAPE = 256,
  KEY_TAB,
  KEY_ENTER,
  KEY_BACKSPACE,
  KEY_INSERT,
  KEY_DELETE,
  KEY_LSHIFT,
  KEY_RSHIFT,
  KEY_LCTRL,
  KEY_RCTRL,
  KEY_LALT,
  KEY_RALT,
  KEY_LSUPER,
  KEY_RSUPER,
  KEY_UP,
  KEY_DOWN,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_PAGEUP,
  KEY_PAGEDOWN,
  KEY_HOME,
  KEY_END,
  KEY_F1,
  KEY_F2,
  KEY_F3,
  KEY_F4,
  KEY_F5,
  KEY_F6,
  KEY_F7,
  KEY_F8,
  KEY_F9,
  KEY_F10,
  KEY_F11,
  KEY_F12,
};

///////////////////////////////////////////////////////////////////////

/* @brief Mouse button enumeration.
 * @ingroup input
 */
enum Button
{
  BUTTON_LEFT,
  BUTTON_RIGHT,
  BUTTON_MIDDLE,
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup input
 */
class Hook
{
public:
  virtual ~Hook(void);
  virtual bool onKeyPressed(Key key, bool pressed);
  virtual bool onCharInput(wchar_t character);
  virtual bool onButtonClicked(Button button, bool clicked);
  virtual bool onCursorMoved(const ivec2& position);
  virtual bool onWheelTurned(int offset);
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup input
 */
class Target
{
public:
  virtual ~Target(void);
  virtual void onWindowResized(unsigned int width, unsigned int height);
  virtual void onKeyPressed(Key key, bool pressed);
  virtual void onCharInput(wchar_t character);
  virtual void onButtonClicked(Button button, bool clicked);
  virtual void onCursorMoved(const ivec2& position);
  virtual void onWheelTurned(int offset);
  virtual void onFocusChanged(bool activated);
};

///////////////////////////////////////////////////////////////////////

/*! @brief Input manager.
 *  @ingroup input
 *
 *  This class provides basic HID (input) signals.
 */
class Context : public Singleton<Context>
{
public:
  /*! Destructor.
   */
  virtual ~Context(void);
  void captureCursor(void);
  void releaseCursor(void);
  /*! @return @c true if the specified key is pressed, otherwise @c false.
   *  @param[in] key The desired key.
   */
  bool isKeyDown(const Key& key) const;
  /*! @return @c true if the specified mouse button is pressed, otherwise @c false.
   *  @param[in] button The desired mouse button.
   */
  bool isButtonDown(Button button) const;
  bool isCursorCaptured(void) const;
  /*! @return The width, in pixels, of the mousable screen area.
   */
  unsigned int getWidth(void) const;
  /*! @return The height, in pixels, of the mousable screen area.
   */
  unsigned int getHeight(void) const;
  /*! @return The current mouse position.
   */
  const ivec2& getCursorPosition(void) const;
  /*! Places the the mouse cursor at the specified position.
   *  @param[in] newPosition The desired mouse position.
   */
  void setCursorPosition(const ivec2& newPosition);
  Hook* getHook(void) const;
  void setHook(Hook* newHook);
  Target* getTarget(void) const;
  void setTarget(Target* newTarget);
  /*! @return The context underlying this input manager.
   */
  GL::Context& getContext(void) const;
  static bool createSingleton(GL::Context& context);
private:
  Context(GL::Context& context);
  Context(const Context& source);
  Context& operator = (const Context& source);
  void onContextResized(unsigned int width, unsigned int height);
  static void keyboardCallback(int key, int action);
  static void characterCallback(int character, int action);
  static void mousePosCallback(int x, int y);
  static void mouseButtonCallback(int button, int action);
  static void mouseWheelCallback(int position);
  GL::Context& context;
  int wheelPosition;
  Hook* currentHook;
  Target* currentTarget;
  bool cursorCaptured;
  mutable ivec2 cursorPosition;
  static Context* instance;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup input
 */
class MayaCamera : public Target
{
public:
  MayaCamera(void);
  void onKeyPressed(Key key, bool pressed);
  void onButtonClicked(Button button, bool clicked);
  void onCursorMoved(const ivec2& position);
  void onWheelTurned(int offset);
  void onFocusChanged(bool activated);
  const Transform3& getTransform(void) const;
private:
  enum Mode
  {
    NONE,
    TUMBLE,
    TRACK,
    DOLLY,
  };
  void updateTransform(void);
  Transform3 transform;
  ivec2 lastPosition;
  vec3 target;
  float angleX;
  float angleY;
  float distance;
  Mode mode;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup input
 */
class SpectatorCamera : public Target
{
public:
  SpectatorCamera(void);
  void update(Time deltaTime);
  void onKeyPressed(Key key, bool pressed);
  void onButtonClicked(Button button, bool clicked);
  void onCursorMoved(const ivec2& position);
  void onFocusChanged(bool activated);
  const Transform3& getTransform(void) const;
  float getSpeed(void) const;
  void setSpeed(float newSpeed);
private:
  enum Direction
  {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    FORWARD,
    BACK,
  };
  void updateTransform(void);
  Transform3 transform;
  ivec2 lastPosition;
  float angleX;
  float angleY;
  float speed;
  bool directions[6];
  bool turbo;
};

///////////////////////////////////////////////////////////////////////

class TextController : public Target
{
public:
  TextController(void);
  void onKeyPressed(Key key, bool pressed);
  void onCharInput(wchar_t character);
  const String& getText(void) const;
  void setText(const String& newText);
  size_t getCaretPosition(void) const;
  void setCaretPosition(size_t newPosition);
private:
  bool isCtrlKeyDown(void) const;
  String text;
  size_t caretPosition;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace input*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_INPUT_H*/
///////////////////////////////////////////////////////////////////////
