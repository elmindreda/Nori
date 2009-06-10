///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/GLContext.h>

#include <wendy/Input.h>

#include <GL/glfw.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace input
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////
  
Key::Key(unsigned int initSymbol):
  symbol(initSymbol)
{
}

Key::operator unsigned int (void) const
{
  return symbol;
}

///////////////////////////////////////////////////////////////////////

Button::Button(unsigned int initSymbol):
  symbol(initSymbol)
{
}

Button::operator unsigned int (void) const
{
  return symbol;
}

///////////////////////////////////////////////////////////////////////

void Focus::onContextResized(unsigned int width, unsigned int height)
{
}

void Focus::onKeyPressed(Key key, bool pressed)
{
}

void Focus::onCharInput(wchar_t character)
{
}

void Focus::onButtonClicked(Button button, bool clicked)
{
}

void Focus::onCursorMoved(const Vec2i& position)
{
}

void Focus::onWheelTurned(int offset)
{
}

void Focus::onFocusChanged(bool activated)
{
}

///////////////////////////////////////////////////////////////////////

Context::~Context(void)
{
  setFocus(NULL);
  instance = NULL;
}

void Context::captureCursor(void)
{
  glfwDisable(GLFW_MOUSE_CURSOR);
  cursorCaptured = true;
}

void Context::releaseCursor(void)
{
  glfwEnable(GLFW_MOUSE_CURSOR);
  cursorCaptured = false;
}

bool Context::isKeyDown(const Key& key) const
{
  int externalKey = key;

  KeyMap::const_iterator i = internalMap.find(externalKey);
  if (i != internalMap.end())
    externalKey = (*i).second;

  return (glfwGetKey(externalKey) == GLFW_PRESS) ? true : false;
}

bool Context::isButtonDown(Button button) const
{
  if (glfwGetMouseButton(button + GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
    return true;

  return false;
}

bool Context::isCursorCaptured(void) const
{
  return cursorCaptured;
}

unsigned int Context::getWidth(void) const
{
  return context.getWidth();
}

unsigned int Context::getHeight(void) const
{
  return context.getHeight();
}

const Vec2i& Context::getCursorPosition(void) const
{
  glfwGetMousePos(&cursorPosition.x, &cursorPosition.y);

  return cursorPosition;
}

void Context::setCursorPosition(const Vec2i& newPosition)
{
  cursorPosition = newPosition;
  glfwSetMousePos(newPosition.x, newPosition.y);
}

SignalProxy2<void, unsigned int, unsigned int> Context::getResizedSignal(void)
{
  return resizedSignal;
}

SignalProxy2<void, Key, bool> Context::getKeyPressedSignal(void)
{
  return keyPressedSignal;
}

SignalProxy1<void, wchar_t> Context::getCharInputSignal(void)
{
  return charInputSignal;
}

SignalProxy2<void, Button, bool> Context::getButtonClickedSignal(void)
{
  return buttonClickedSignal;
}

SignalProxy1<void, const Vec2i&> Context::getCursorMovedSignal(void)
{
  return cursorMovedSignal;
}

SignalProxy1<void, int> Context::getWheelTurnedSignal(void)
{
  return wheelTurnedSignal;
}

Focus* Context::getFocus(void) const
{
  return currentFocus;
}

void Context::setFocus(Focus* newFocus)
{
  if (currentFocus == newFocus)
    return;

  if (currentFocus)
    currentFocus->onFocusChanged(false);

  currentFocus = newFocus;

  if (currentFocus)
    currentFocus->onFocusChanged(true);
}

GL::Context& Context::getContext(void) const
{
  return context;
}

bool Context::create(GL::Context& context)
{
  set(new Context(context));
  return true;
}

Context::Context(GL::Context& initContext):
  context(initContext),
  currentFocus(NULL),
  cursorCaptured(false)
{
  // TODO: Remove this upon the arrival of GLFW_USER_DATA.
  instance = this;

  if (!internalMap.size())
  {
    // Jag kom på en sak. Du luktar struts.
    // Jag tänker inte fira jul med dig.

    internalMap[Key::SPACE] = GLFW_KEY_SPACE;
    internalMap[Key::ESCAPE] = GLFW_KEY_ESC;
    internalMap[Key::TAB] = GLFW_KEY_TAB;
    internalMap[Key::ENTER] = GLFW_KEY_ENTER;
    internalMap[Key::BACKSPACE] = GLFW_KEY_BACKSPACE;
    internalMap[Key::INSERT] = GLFW_KEY_INSERT;
    internalMap[Key::DELETE] = GLFW_KEY_DEL;
    internalMap[Key::LSHIFT] = GLFW_KEY_LSHIFT;
    internalMap[Key::RSHIFT] = GLFW_KEY_RSHIFT;
    internalMap[Key::LCTRL] = GLFW_KEY_LCTRL;
    internalMap[Key::RCTRL] = GLFW_KEY_RCTRL;
    internalMap[Key::LALT] = GLFW_KEY_LALT;
    internalMap[Key::RALT] = GLFW_KEY_RALT;
    internalMap[Key::UP] = GLFW_KEY_UP;
    internalMap[Key::DOWN] = GLFW_KEY_DOWN;
    internalMap[Key::LEFT] = GLFW_KEY_LEFT;
    internalMap[Key::RIGHT] = GLFW_KEY_RIGHT;
    internalMap[Key::PAGEUP] = GLFW_KEY_PAGEUP;
    internalMap[Key::PAGEDOWN] = GLFW_KEY_PAGEDOWN;
    internalMap[Key::HOME] = GLFW_KEY_HOME;
    internalMap[Key::END] = GLFW_KEY_END;
    internalMap[Key::F1] = GLFW_KEY_F1;
    internalMap[Key::F2] = GLFW_KEY_F2;
    internalMap[Key::F3] = GLFW_KEY_F3;
    internalMap[Key::F4] = GLFW_KEY_F4;
    internalMap[Key::F5] = GLFW_KEY_F5;
    internalMap[Key::F6] = GLFW_KEY_F6;
    internalMap[Key::F7] = GLFW_KEY_F7;
    internalMap[Key::F8] = GLFW_KEY_F8;
    internalMap[Key::F9] = GLFW_KEY_F9;
    internalMap[Key::F10] = GLFW_KEY_F10;
    internalMap[Key::F11] = GLFW_KEY_F11;
    internalMap[Key::F12] = GLFW_KEY_F12;
  }

  if (!externalMap.size())
  {
    // Jag ska förgöra er alla med min blöta katt.

    externalMap[GLFW_KEY_SPACE] = Key::SPACE;
    externalMap[GLFW_KEY_ESC] = Key::ESCAPE;
    externalMap[GLFW_KEY_TAB] = Key::TAB;
    externalMap[GLFW_KEY_ENTER] = Key::ENTER;
    externalMap[GLFW_KEY_BACKSPACE] = Key::BACKSPACE;
    externalMap[GLFW_KEY_INSERT] = Key::INSERT;
    externalMap[GLFW_KEY_DEL] = Key::DELETE;
    externalMap[GLFW_KEY_LSHIFT] = Key::LSHIFT;
    externalMap[GLFW_KEY_RSHIFT] = Key::RSHIFT;
    externalMap[GLFW_KEY_LCTRL] = Key::LCTRL;
    externalMap[GLFW_KEY_RCTRL] = Key::RCTRL;
    externalMap[GLFW_KEY_LALT] = Key::LALT;
    externalMap[GLFW_KEY_RALT] = Key::RALT;
    externalMap[GLFW_KEY_UP] = Key::UP;
    externalMap[GLFW_KEY_DOWN] = Key::DOWN;
    externalMap[GLFW_KEY_LEFT] = Key::LEFT;
    externalMap[GLFW_KEY_RIGHT] = Key::RIGHT;
    externalMap[GLFW_KEY_PAGEUP] = Key::PAGEUP;
    externalMap[GLFW_KEY_PAGEDOWN] = Key::PAGEDOWN;
    externalMap[GLFW_KEY_HOME] = Key::HOME;
    externalMap[GLFW_KEY_END] = Key::END;
    externalMap[GLFW_KEY_F1] = Key::F1;
    externalMap[GLFW_KEY_F2] = Key::F2;
    externalMap[GLFW_KEY_F3] = Key::F3;
    externalMap[GLFW_KEY_F4] = Key::F4;
    externalMap[GLFW_KEY_F5] = Key::F5;
    externalMap[GLFW_KEY_F6] = Key::F6;
    externalMap[GLFW_KEY_F7] = Key::F7;
    externalMap[GLFW_KEY_F8] = Key::F8;
    externalMap[GLFW_KEY_F9] = Key::F9;
    externalMap[GLFW_KEY_F10] = Key::F10;
    externalMap[GLFW_KEY_F11] = Key::F11;
    externalMap[GLFW_KEY_F12] = Key::F12;
  }

  context.getResizedSignal().connect(*this, &Context::sizeCallback);

  glfwSetMousePosCallback(mousePosCallback);
  glfwSetMouseButtonCallback(mouseButtonCallback);
  glfwSetKeyCallback(keyboardCallback);
  glfwSetCharCallback(characterCallback);
  glfwSetMouseWheelCallback(mouseWheelCallback);

  wheelPosition = glfwGetMouseWheel();
  
  glfwEnable(GLFW_MOUSE_CURSOR);
}

Context::Context(const Context& source):
  context(source.context)
{
  // NOTE: Not implemented.
}

Context& Context::operator = (const Context& source)
{
  // NOTE: Not implemented.

  return *this;
}

void Context::sizeCallback(unsigned int width, unsigned int height)
{
  resizedSignal.emit(width, height);

  if (currentFocus)
    currentFocus->onContextResized(width, height);
}

void Context::keyboardCallback(int key, int action)
{
  if (key > GLFW_KEY_SPECIAL)
  {
    KeyMap::const_iterator i = externalMap.find(key);
    if (i == externalMap.end())
      return;

    key = (*i).second;
  }

  const bool pressed = (action == GLFW_PRESS) ? true : false;

  instance->keyPressedSignal.emit(key, pressed);

  if (instance->currentFocus)
    instance->currentFocus->onKeyPressed(key, pressed);
}

void Context::characterCallback(int character, int action)
{
  if (action != GLFW_PRESS)
    return;

  instance->charInputSignal.emit((wchar_t) character);

  if (instance->currentFocus)
    instance->currentFocus->onCharInput((wchar_t) character);
}

void Context::mousePosCallback(int x, int y)
{
  const Vec2i position(x, y);

  instance->cursorMovedSignal.emit(position);

  if (instance->currentFocus)
    instance->currentFocus->onCursorMoved(position);
}

void Context::mouseButtonCallback(int button, int action)
{
  const bool clicked = (action == GLFW_PRESS) ? true : false;

  button -= GLFW_MOUSE_BUTTON_1;

  instance->buttonClickedSignal.emit(button, clicked); 

  if (instance->currentFocus)
    instance->currentFocus->onButtonClicked(button, clicked);
}

void Context::mouseWheelCallback(int position)
{
  instance->wheelTurnedSignal.emit(instance->wheelPosition - position);

  if (instance->currentFocus)
    instance->currentFocus->onWheelTurned(instance->wheelPosition - position);

  instance->wheelPosition = position;
}

Context::KeyMap Context::internalMap;
Context::KeyMap Context::externalMap;

Context* Context::instance = NULL;

///////////////////////////////////////////////////////////////////////

  } /*namespace input*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
