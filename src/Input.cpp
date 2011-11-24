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

#include <wendy/Config.h>

#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLContext.h>

#include <wendy/Input.h>

#define GLFW_NO_GLU
#include <GL/glfw.h>

#include <map>
#include <algorithm>
#include <cstring>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/constants.hpp>

///////////////////////////////////////////////////////////////////////

namespace
{

typedef std::map<int, int> KeyMap;

KeyMap internalMap;
KeyMap externalMap;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace input
  {

///////////////////////////////////////////////////////////////////////

Hook::~Hook()
{
}

bool Hook::onKeyPressed(Key key, bool pressed)
{
  return false;
}

bool Hook::onCharInput(wchar_t character)
{
  return false;
}

bool Hook::onButtonClicked(Button button, bool clicked)
{
  return false;
}

bool Hook::onCursorMoved(const ivec2& position)
{
  return false;
}

bool Hook::onWheelTurned(int offset)
{
  return false;
}

///////////////////////////////////////////////////////////////////////

Target::~Target()
{
}

void Target::onWindowResized(unsigned int width, unsigned int height)
{
}

void Target::onKeyPressed(Key key, bool pressed)
{
}

void Target::onCharInput(wchar_t character)
{
}

void Target::onButtonClicked(Button button, bool clicked)
{
}

void Target::onCursorMoved(const ivec2& position)
{
}

void Target::onWheelTurned(int offset)
{
}

void Target::onFocusChanged(bool activated)
{
}

///////////////////////////////////////////////////////////////////////

Context::~Context()
{
  glfwSetMousePosCallback(NULL);
  glfwSetMouseButtonCallback(NULL);
  glfwSetKeyCallback(NULL);
  glfwSetCharCallback(NULL);
  glfwSetMouseWheelCallback(NULL);

  instance = NULL;
}

void Context::captureCursor()
{
  cursorCaptured = true;
  glfwDisable(GLFW_MOUSE_CURSOR);
}

void Context::releaseCursor()
{
  cursorCaptured = false;
  glfwEnable(GLFW_MOUSE_CURSOR);
}

bool Context::isKeyDown(const Key& key) const
{
  int externalKey = key;

  KeyMap::const_iterator i = internalMap.find(externalKey);
  if (i != internalMap.end())
    externalKey = i->second;

  return (glfwGetKey(externalKey) == GLFW_PRESS) ? true : false;
}

bool Context::isButtonDown(Button button) const
{
  if (glfwGetMouseButton(button + GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
    return true;

  return false;
}

bool Context::isCursorCaptured() const
{
  return cursorCaptured;
}

unsigned int Context::getWidth() const
{
  return context.getDefaultFramebuffer().getWidth();
}

unsigned int Context::getHeight() const
{
  return context.getDefaultFramebuffer().getHeight();
}

const ivec2& Context::getCursorPosition() const
{
  glfwGetMousePos(&cursorPosition.x, &cursorPosition.y);

  return cursorPosition;
}

void Context::setCursorPosition(const ivec2& newPosition)
{
  cursorPosition = newPosition;
  glfwSetMousePos(newPosition.x, newPosition.y);
}

Hook* Context::getHook() const
{
  return currentHook;
}

void Context::setHook(Hook* newHook)
{
  currentHook = newHook;
}

Target* Context::getTarget() const
{
  return currentTarget;
}

void Context::setTarget(Target* newTarget)
{
  if (currentTarget == newTarget)
    return;

  if (currentTarget)
    currentTarget->onFocusChanged(false);

  currentTarget = newTarget;

  if (currentTarget)
    currentTarget->onFocusChanged(true);
}

GL::Context& Context::getContext() const
{
  return context;
}

bool Context::createSingleton(GL::Context& context)
{
  set(new Context(context));
  return true;
}

Context::Context(GL::Context& initContext):
  context(initContext),
  currentHook(NULL),
  currentTarget(NULL),
  cursorCaptured(false)
{
  // TODO: Remove this upon the arrival of GLFW_USER_DATA.
  instance = this;

  if (internalMap.empty())
  {
    internalMap[KEY_SPACE] = GLFW_KEY_SPACE;
    internalMap[KEY_ESCAPE] = GLFW_KEY_ESC;
    internalMap[KEY_TAB] = GLFW_KEY_TAB;
    internalMap[KEY_ENTER] = GLFW_KEY_ENTER;
    internalMap[KEY_BACKSPACE] = GLFW_KEY_BACKSPACE;
    internalMap[KEY_INSERT] = GLFW_KEY_INSERT;
    internalMap[KEY_DELETE] = GLFW_KEY_DEL;
    internalMap[KEY_LSHIFT] = GLFW_KEY_LSHIFT;
    internalMap[KEY_RSHIFT] = GLFW_KEY_RSHIFT;
    internalMap[KEY_LCTRL] = GLFW_KEY_LCTRL;
    internalMap[KEY_RCTRL] = GLFW_KEY_RCTRL;
    internalMap[KEY_LALT] = GLFW_KEY_LALT;
    internalMap[KEY_RALT] = GLFW_KEY_RALT;
    internalMap[KEY_LSUPER] = GLFW_KEY_LSUPER;
    internalMap[KEY_RSUPER] = GLFW_KEY_RSUPER;
    internalMap[KEY_UP] = GLFW_KEY_UP;
    internalMap[KEY_DOWN] = GLFW_KEY_DOWN;
    internalMap[KEY_LEFT] = GLFW_KEY_LEFT;
    internalMap[KEY_RIGHT] = GLFW_KEY_RIGHT;
    internalMap[KEY_PAGEUP] = GLFW_KEY_PAGEUP;
    internalMap[KEY_PAGEDOWN] = GLFW_KEY_PAGEDOWN;
    internalMap[KEY_HOME] = GLFW_KEY_HOME;
    internalMap[KEY_END] = GLFW_KEY_END;
    internalMap[KEY_F1] = GLFW_KEY_F1;
    internalMap[KEY_F2] = GLFW_KEY_F2;
    internalMap[KEY_F3] = GLFW_KEY_F3;
    internalMap[KEY_F4] = GLFW_KEY_F4;
    internalMap[KEY_F5] = GLFW_KEY_F5;
    internalMap[KEY_F6] = GLFW_KEY_F6;
    internalMap[KEY_F7] = GLFW_KEY_F7;
    internalMap[KEY_F8] = GLFW_KEY_F8;
    internalMap[KEY_F9] = GLFW_KEY_F9;
    internalMap[KEY_F10] = GLFW_KEY_F10;
    internalMap[KEY_F11] = GLFW_KEY_F11;
    internalMap[KEY_F12] = GLFW_KEY_F12;
  }

  if (externalMap.empty())
  {
    externalMap[GLFW_KEY_SPACE] = KEY_SPACE;
    externalMap[GLFW_KEY_ESC] = KEY_ESCAPE;
    externalMap[GLFW_KEY_TAB] = KEY_TAB;
    externalMap[GLFW_KEY_ENTER] = KEY_ENTER;
    externalMap[GLFW_KEY_BACKSPACE] = KEY_BACKSPACE;
    externalMap[GLFW_KEY_INSERT] = KEY_INSERT;
    externalMap[GLFW_KEY_DEL] = KEY_DELETE;
    externalMap[GLFW_KEY_LSHIFT] = KEY_LSHIFT;
    externalMap[GLFW_KEY_RSHIFT] = KEY_RSHIFT;
    externalMap[GLFW_KEY_LCTRL] = KEY_LCTRL;
    externalMap[GLFW_KEY_RCTRL] = KEY_RCTRL;
    externalMap[GLFW_KEY_LALT] = KEY_LALT;
    externalMap[GLFW_KEY_RALT] = KEY_RALT;
    externalMap[GLFW_KEY_LSUPER] = KEY_LSUPER;
    externalMap[GLFW_KEY_RSUPER] = KEY_RSUPER;
    externalMap[GLFW_KEY_UP] = KEY_UP;
    externalMap[GLFW_KEY_DOWN] = KEY_DOWN;
    externalMap[GLFW_KEY_LEFT] = KEY_LEFT;
    externalMap[GLFW_KEY_RIGHT] = KEY_RIGHT;
    externalMap[GLFW_KEY_PAGEUP] = KEY_PAGEUP;
    externalMap[GLFW_KEY_PAGEDOWN] = KEY_PAGEDOWN;
    externalMap[GLFW_KEY_HOME] = KEY_HOME;
    externalMap[GLFW_KEY_END] = KEY_END;
    externalMap[GLFW_KEY_F1] = KEY_F1;
    externalMap[GLFW_KEY_F2] = KEY_F2;
    externalMap[GLFW_KEY_F3] = KEY_F3;
    externalMap[GLFW_KEY_F4] = KEY_F4;
    externalMap[GLFW_KEY_F5] = KEY_F5;
    externalMap[GLFW_KEY_F6] = KEY_F6;
    externalMap[GLFW_KEY_F7] = KEY_F7;
    externalMap[GLFW_KEY_F8] = KEY_F8;
    externalMap[GLFW_KEY_F9] = KEY_F9;
    externalMap[GLFW_KEY_F10] = KEY_F10;
    externalMap[GLFW_KEY_F11] = KEY_F11;
    externalMap[GLFW_KEY_F12] = KEY_F12;
  }

  context.getResizedSignal().connect(*this, &Context::onContextResized);

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
  panic("Input contexts may not be copied");
}

void Context::onContextResized(unsigned int width, unsigned int height)
{
  if (currentTarget)
    currentTarget->onWindowResized(width, height);
}

void Context::keyboardCallback(int key, int action)
{
  if (key > GLFW_KEY_SPECIAL)
  {
    KeyMap::const_iterator i = externalMap.find(key);
    if (i == externalMap.end())
      return;

    key = i->second;
  }

  const bool pressed = (action == GLFW_PRESS) ? true : false;

  if (instance->currentHook)
  {
    if (instance->currentHook->onKeyPressed(Key(key), pressed))
      return;
  }

  if (instance->currentTarget)
    instance->currentTarget->onKeyPressed(Key(key), pressed);
}

void Context::characterCallback(int character, int action)
{
  if (action != GLFW_PRESS)
    return;

  if (instance->currentHook)
  {
    if (instance->currentHook->onCharInput((wchar_t) character))
      return;
  }

  if (instance->currentTarget)
    instance->currentTarget->onCharInput((wchar_t) character);
}

void Context::mousePosCallback(int x, int y)
{
  const ivec2 position(x, y);

  if (instance->currentHook)
  {
    if (instance->currentHook->onCursorMoved(position))
      return;
  }

  if (instance->currentTarget)
    instance->currentTarget->onCursorMoved(position);
}

void Context::mouseButtonCallback(int button, int action)
{
  const bool clicked = (action == GLFW_PRESS) ? true : false;

  button -= GLFW_MOUSE_BUTTON_1;

  if (instance->currentHook)
  {
    if (instance->currentHook->onButtonClicked(Button(button), clicked))
      return;
  }

  if (instance->currentTarget)
    instance->currentTarget->onButtonClicked(Button(button), clicked);
}

void Context::mouseWheelCallback(int position)
{
  const int offset = instance->wheelPosition - position;

  if (instance->currentHook)
  {
    if (instance->currentHook->onWheelTurned(offset))
      return;
  }

  if (instance->currentTarget)
    instance->currentTarget->onWheelTurned(offset);

  instance->wheelPosition = position;
}

Context& Context::operator = (const Context& source)
{
  panic("Input contexts may not be assigned");
}

Context* Context::instance = NULL;

///////////////////////////////////////////////////////////////////////

MayaCamera::MayaCamera():
  angleX(0.f),
  angleY(0.f),
  distance(5.f),
  mode(NONE)
{
  updateTransform();
}

void MayaCamera::onKeyPressed(Key key, bool pressed)
{
}

void MayaCamera::onButtonClicked(Button button, bool clicked)
{
  Context* context = Context::getSingleton();

  if (clicked)
  {
    switch (button)
    {
      case BUTTON_LEFT:
        mode = TUMBLE;
        break;
      case BUTTON_MIDDLE:
        mode = TRACK;
        break;
      case BUTTON_RIGHT:
        mode = DOLLY;
        break;
    }

    if (mode != NONE)
      context->captureCursor();
  }
  else
  {
    mode = NONE;
    context->releaseCursor();
  }
}

void MayaCamera::onCursorMoved(const ivec2& position)
{
  ivec2 offset = position - lastPosition;

  if (mode == TUMBLE)
  {
    angleY += offset.x / 250.f;
    angleX += offset.y / 250.f;
    updateTransform();
  }
  else if (mode == TRACK)
  {
    vec3 axisX(1.f, 0.f, 0.f);
    vec3 axisY(0.f, 1.f, 0.f);

    transform.rotateVector(axisX);
    transform.rotateVector(axisY);

    target -= axisX * (float) offset.x / 50.f;
    target += axisY * (float) offset.y / 50.f;
    updateTransform();
  }

  lastPosition = position;
}

void MayaCamera::onWheelTurned(int offset)
{
  distance += (float) offset;
  updateTransform();
}

void MayaCamera::onFocusChanged(bool activated)
{
}

const Transform3& MayaCamera::getTransform() const
{
  return transform;
}

void MayaCamera::updateTransform()
{
  transform.rotation = quat(vec3(angleX, angleY, 0.f));

  vec3 offset(0.f, 0.f, distance);
  transform.rotateVector(offset);
  transform.position = target + offset;
}

///////////////////////////////////////////////////////////////////////

SpectatorController::SpectatorController():
  angleX(0.f),
  angleY(0.f),
  speed(3.f),
  turbo(false)
{
  std::memset(directions, 0, sizeof(directions));
}

void SpectatorController::update(Time deltaTime)
{
  float multiplier;

  if (turbo)
    multiplier = 3.f;
  else
    multiplier = 1.f;

  vec3 direction;

  if (directions[UP])
    direction.y += 1.f;
  if (directions[DOWN])
    direction.y -= 1.f;
  if (directions[FORWARD])
    direction.z -= 1.f;
  if (directions[BACK])
    direction.z += 1.f;
  if (directions[LEFT])
    direction.x -= 1.f;
  if (directions[RIGHT])
    direction.x += 1.f;

  if (length2(direction))
    direction = normalize(transform.rotation * direction);

  transform.position += direction * speed * multiplier * float(deltaTime);
}

void SpectatorController::release()
{
  std::memset(directions, 0, sizeof(directions));
  turbo = false;
}

void SpectatorController::inputKeyPress(Key key, bool pressed)
{
  switch (key)
  {
    case 'W':
    case KEY_UP:
    {
      if (pressed)
        directions[FORWARD] = true;
      else
        directions[FORWARD] = false;
      break;
    }

    case 'S':
    case KEY_DOWN:
    {
      if (pressed)
        directions[BACK] = true;
      else
        directions[BACK] = false;
      break;
    }

    case 'A':
    case KEY_LEFT:
    {
      if (pressed)
        directions[LEFT] = true;
      else
        directions[LEFT] = false;
      break;
    }

    case 'D':
    case KEY_RIGHT:
    {
      if (pressed)
        directions[RIGHT] = true;
      else
        directions[RIGHT] = false;
      break;
    }

    case KEY_LCTRL:
    case KEY_RCTRL:
    {
      if (pressed)
        directions[DOWN] = true;
      else
        directions[DOWN] = false;
      break;
    }

    case KEY_LSHIFT:
    case KEY_RSHIFT:
    {
      if (pressed)
        turbo = true;
      else
        turbo = false;
      break;
    }
  }
}

void SpectatorController::inputButtonClick(Button button, bool clicked)
{
  if (button == BUTTON_RIGHT)
  {
    if (clicked)
      directions[UP] = true;
    else
      directions[UP] = false;
  }
}

void SpectatorController::inputCursorOffset(const ivec2& offset)
{
  const float scale = 1.f / 250.f;
  const float limit = half_pi<float>() - 0.01f;

  setRotation(clamp(angleX - offset.y * scale, -limit, limit),
              angleY - offset.x * scale);
}

const Transform3& SpectatorController::getTransform() const
{
  return transform;
}

float SpectatorController::getSpeed() const
{
  return speed;
}

void SpectatorController::setSpeed(float newSpeed)
{
  speed = newSpeed;
}

void SpectatorController::setPosition(const vec3& newPosition)
{
  transform.position = newPosition;
}

void SpectatorController::setRotation(float newAngleX, float newAngleY)
{
  angleX = newAngleX;
  angleY = newAngleY;

  const quat axisX = angleAxis(degrees(angleX), vec3(1.f, 0.f, 0.f));
  const quat axisY = angleAxis(degrees(angleY), vec3(0.f, 1.f, 0.f));
  transform.rotation = axisY * axisX;
}

///////////////////////////////////////////////////////////////////////

TextController::TextController():
  caretPosition(0)
{
}

void TextController::onKeyPressed(Key key, bool pressed)
{
  switch (key)
  {
    case KEY_BACKSPACE:
    {
      if (!pressed)
        break;

      if (!text.empty() && caretPosition > 0)
      {
        text.erase(caretPosition - 1, 1);
        setCaretPosition(caretPosition - 1);
      }

      break;
    }

    case KEY_DELETE:
    {
      if (!pressed)
        break;

      if (!text.empty() && caretPosition < text.length())
        text.erase(caretPosition, 1);

      break;
    }

    case KEY_LEFT:
    {
      if (!pressed)
        break;

      if (caretPosition > 0)
        setCaretPosition(caretPosition - 1);
      break;
    }

    case KEY_RIGHT:
    {
      if (!pressed)
        break;

      setCaretPosition(caretPosition + 1);
      break;
    }

    case KEY_HOME:
    {
      if (!pressed)
        break;

      setCaretPosition(0);
      break;
    }

    case KEY_END:
    {
      if (!pressed)
        break;

      setCaretPosition(text.length());
      break;
    }

    case 'U':
    {
      if (pressed && isCtrlKeyDown())
      {
        text.erase(0, caretPosition);
        setCaretPosition(0);
      }

      break;
    }

    case 'A':
    {
      if (pressed && isCtrlKeyDown())
        setCaretPosition(0);

      break;
    }

    case 'E':
    {
      if (pressed && isCtrlKeyDown())
        setCaretPosition(text.length());

      break;
    }

    case 'W':
    {
      if (pressed && isCtrlKeyDown())
      {
        size_t pos = caretPosition;

        if (pos == text.length())
          pos--;

        while (pos > 0 && text[pos] == ' ')
          pos--;

        if (pos > 0)
        {
          pos = text.rfind(' ', pos);
          if (pos == String::npos)
            pos = 0;
          else
            pos++;
        }

        text.erase(pos, caretPosition - pos);
        setCaretPosition(pos);
      }

      break;
    }
  }
}

void TextController::onCharInput(wchar_t character)
{
  if (isCtrlKeyDown())
    return;

  if (character < 256)
  {
    text.insert(caretPosition, 1, (char) character);
    setCaretPosition(caretPosition + 1);
  }
}

const String& TextController::getText() const
{
  return text;
}

void TextController::setText(const String& newText)
{
  text = newText;
  setCaretPosition(caretPosition);
}

size_t TextController::getCaretPosition() const
{
  return caretPosition;
}

void TextController::setCaretPosition(size_t newPosition)
{
  if (newPosition > text.length())
    caretPosition = text.length();
  else
    caretPosition = newPosition;
}

bool TextController::isCtrlKeyDown() const
{
  Context* context = Context::getSingleton();

  return context->isKeyDown(KEY_LCTRL) || context->isKeyDown(KEY_RCTRL);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace input*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
