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
#include <wendy/Vector.h>
#include <wendy/Quaternion.h>
#include <wendy/Transform.h>

#include <wendy/OpenGL.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace input
  {

///////////////////////////////////////////////////////////////////////

/*! @brief Input key.
 *  @ingroup input
 *
 *  Represents a single input key (from a keyboard).
 */
class Key
{
public:
  enum Symbol
  {
    SPACE = 32,
    ESCAPE = 256, TAB, ENTER, BACKSPACE, INSERT, DELETE,
    LSHIFT, RSHIFT, LCTRL, RCTRL, LALT, RALT, LSUPER, RSUPER,
    UP, DOWN, LEFT, RIGHT, PAGEUP, PAGEDOWN, HOME, END,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
  };
  Key(unsigned int symbol);
  operator unsigned int (void) const;
private:
  unsigned int symbol;
};

///////////////////////////////////////////////////////////////////////

/* @brief Mouse button symbol.
 * @ingroup input
 */
class Button
{
public:
  enum Symbol
  {
    LEFT,
    RIGHT,
    MIDDLE,
  };
  Button(unsigned int symbol);
  operator unsigned int (void) const;
private:
  unsigned int symbol;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup input
 */
class Focus
{
public:
  virtual ~Focus(void);
  virtual void onContextResized(unsigned int width, unsigned int height);
  virtual void onKeyPressed(Key key, bool pressed);
  virtual void onCharInput(wchar_t character);
  virtual void onButtonClicked(Button button, bool clicked);
  virtual void onCursorMoved(const Vec2i& position);
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
  const Vec2i& getCursorPosition(void) const;
  /*! Places the the mouse cursor at the specified position.
   *  @param[in] newPosition The desired mouse position.
   */
  void setCursorPosition(const Vec2i& newPosition);
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
  SignalProxy2<void, Button, bool> getButtonClickedSignal(void);
  /*! @return The signal for mouse cursor movement events.
   */
  SignalProxy1<void, const Vec2i&> getCursorMovedSignal(void);
  /*! @return The signal for mouse wheel events.
    */
  SignalProxy1<void, int> getWheelTurnedSignal(void);
  Focus* getFocus(void) const;
  void setFocus(Focus* newFocus);
  /*! @return The context underlying this input manager.
   */
  GL::Context& getContext(void) const;
  static bool createSingleton(GL::Context& context);
private:
  Context(GL::Context& context);
  Context(const Context& source);
  Context& operator = (const Context& source);
  void sizeCallback(unsigned int width, unsigned int height);
  static void keyboardCallback(int key, int action);
  static void characterCallback(int character, int action);
  static void mousePosCallback(int x, int y);
  static void mouseButtonCallback(int button, int action);
  static void mouseWheelCallback(int position);
  Signal2<void, unsigned int, unsigned int> resizedSignal;
  Signal2<void, Key, bool> keyPressedSignal;
  Signal1<void, wchar_t> charInputSignal;
  Signal2<void, Button, bool> buttonClickedSignal;
  Signal1<void, const Vec2i&> cursorMovedSignal;
  Signal1<void, int> wheelTurnedSignal;
  GL::Context& context;
  int wheelPosition;
  Focus* currentFocus;
  bool cursorCaptured;
  mutable Vec2i cursorPosition;
  static Context* instance;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup input
 */
class MayaCamera : public Focus
{
public:
  MayaCamera(void);
  void onKeyPressed(Key key, bool pressed);
  void onButtonClicked(Button button, bool clicked);
  void onCursorMoved(const Vec2i& position);
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
  Vec2i lastPosition;
  Vec3 target;
  float angleX;
  float angleY;
  float distance;
  Mode mode;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup input
 */
class SpectatorCamera : public Focus
{
public:
  SpectatorCamera(void);
  void update(Time deltaTime);
  void onKeyPressed(Key key, bool pressed);
  void onButtonClicked(Button button, bool clicked);
  void onCursorMoved(const Vec2i& position);
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
  Vec2i lastPosition;
  float angleX;
  float angleY;
  float speed;
  bool directions[6];
  bool turbo;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace input*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_INPUT_H*/
///////////////////////////////////////////////////////////////////////
