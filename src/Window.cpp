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

#include <wendy/Config.hpp>

#include <wendy/Core.hpp>
#include <wendy/Timer.hpp>
#include <wendy/Profile.hpp>
#include <wendy/Window.hpp>

#include <wendy/GLTexture.hpp>
#include <wendy/GLBuffer.hpp>
#include <wendy/GLProgram.hpp>
#include <wendy/GLContext.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstring>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/constants.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

Window& windowFromHandle(GLFWwindow* handle)
{
  return ((GL::Context*) glfwGetWindowUserPointer(handle))->window();
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

EventHook::~EventHook()
{
}

void EventHook::onWindowSize(uint width, uint height)
{
}

void EventHook::onWindowDamage()
{
}

void EventHook::onWindowCloseRequest()
{
}

bool EventHook::onKey(Key key, Action action)
{
  return false;
}

bool EventHook::onCharacter(uint32 character)
{
  return false;
}

bool EventHook::onMouseButton(MouseButton button, Action action)
{
  return false;
}

bool EventHook::onCursorPos(vec2 position)
{
  return false;
}

bool EventHook::onScroll(vec2 offset)
{
  return false;
}

///////////////////////////////////////////////////////////////////////

EventTarget::~EventTarget()
{
}

void EventTarget::onWindowSize(uint width, uint height)
{
}

void EventTarget::onWindowDamage()
{
}

void EventTarget::onWindowCloseRequest()
{
}

void EventTarget::onKey(Key key, Action action)
{
}

void EventTarget::onCharacter(uint32 character)
{
}

void EventTarget::onMouseButton(MouseButton button, Action action)
{
}

void EventTarget::onCursorPos(vec2 position)
{
}

void EventTarget::onScroll(vec2 offset)
{
}

void EventTarget::onFocus(bool activated)
{
}

///////////////////////////////////////////////////////////////////////

WindowConfig::WindowConfig():
  title("Wendy"),
  width(640),
  height(480),
  mode(WINDOWED),
  resizable(true)
{
}

WindowConfig::WindowConfig(const String& initTitle):
  title(initTitle),
  width(640),
  height(480),
  mode(WINDOWED),
  resizable(true)
{
}

WindowConfig::WindowConfig(const String& initTitle,
                           uint initWidth,
                           uint initHeight,
                           WindowMode initMode,
                           bool initResizable):
  title(initTitle),
  width(initWidth),
  height(initHeight),
  mode(initMode),
  resizable(initResizable)
{
}

///////////////////////////////////////////////////////////////////////

Window::~Window()
{
}

bool Window::update()
{
  ProfileNodeCall call("Window::update");

  glfwSwapBuffers(m_handle);
  m_needsRefresh = false;
  m_frameSignal();

  if (m_refreshMode == MANUAL_REFRESH)
  {
    while (!m_needsRefresh && !glfwWindowShouldClose(m_handle))
      glfwWaitEvents();
  }
  else
    glfwPollEvents();

  return !shouldClose();
}

void Window::invalidate()
{
  m_needsRefresh = true;
}

void Window::captureCursor()
{
  glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::releaseCursor()
{
  glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

bool Window::isKeyDown(Key key) const
{
  return glfwGetKey(m_handle, key) == GLFW_PRESS;
}

bool Window::isButtonDown(MouseButton button) const
{
  return glfwGetMouseButton(m_handle, button + GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
}

bool Window::isCursorCaptured() const
{
  return glfwGetInputMode(m_handle, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
}

bool Window::shouldClose() const
{
  return glfwWindowShouldClose(m_handle);
}

void Window::setShouldClose(bool newValue)
{
  glfwSetWindowShouldClose(m_handle, newValue);
}

WindowMode Window::mode() const
{
  if (glfwGetWindowMonitor(m_handle))
    return FULLSCREEN;
  else
    return WINDOWED;
}

void Window::setTitle(const char* newTitle)
{
  glfwSetWindowTitle(m_handle, newTitle);
}

uint Window::width() const
{
  int width;
  glfwGetWindowSize(m_handle, &width, nullptr);
  return uint(width);
}

uint Window::height() const
{
  int height;
  glfwGetWindowSize(m_handle, nullptr, &height);
  return uint(height);
}

void Window::setRefreshMode(RefreshMode newMode)
{
  m_refreshMode = newMode;
}

vec2 Window::cursorPosition() const
{
  double x, y;
  glfwGetCursorPos(m_handle, &x, &y);
  return vec2(float(x), float(y));
}

void Window::setCursorPosition(vec2 newPosition)
{
  glfwSetCursorPos(m_handle, newPosition.x, newPosition.y);
}

String Window::clipboardText() const
{
  return glfwGetClipboardString(m_handle);
}

void Window::setClipboardText(const String& newText)
{
  glfwSetClipboardString(m_handle, newText.c_str());
}

SignalProxy0<void> Window::frameSignal()
{
  return m_frameSignal;
}

void Window::setHook(EventHook* newHook)
{
  m_hook = newHook;
}

void Window::setTarget(EventTarget* newTarget)
{
  if (m_target == newTarget)
    return;

  if (m_target)
    m_target->onFocus(false);

  m_target = newTarget;

  if (m_target)
    m_target->onFocus(true);
}

Window::Window():
  m_handle(nullptr),
  m_hook(nullptr),
  m_target(nullptr),
  m_needsRefresh(false),
  m_refreshMode(AUTOMATIC_REFRESH)
{
}

void Window::init(GLFWwindow* handle)
{
  m_handle = handle;

  glfwSetWindowSizeCallback(m_handle, sizeCallback);
  glfwSetWindowRefreshCallback(m_handle, damageCallback);
  glfwSetWindowCloseCallback(m_handle, closeCallback);
  glfwSetCursorPosCallback(m_handle, cursorPosCallback);
  glfwSetMouseButtonCallback(m_handle, mouseButtonCallback);
  glfwSetKeyCallback(m_handle, keyCallback);
  glfwSetCharCallback(m_handle, characterCallback);
  glfwSetScrollCallback(m_handle, scrollCallback);

  glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

Window::Window(const Window& source)
{
  panic("Input windows may not be copied");
}

void Window::sizeCallback(GLFWwindow* handle, int width, int height)
{
  Window& window = windowFromHandle(handle);

  if (window.m_hook)
    window.m_hook->onWindowSize(width, height);

  if (window.m_target)
    window.m_target->onWindowSize(width, height);
}

void Window::damageCallback(GLFWwindow* handle)
{
  Window& window = windowFromHandle(handle);

  if (window.m_hook)
    window.m_hook->onWindowDamage();

  if (window.m_target)
    window.m_target->onWindowDamage();

  window.m_needsRefresh = true;
}

void Window::closeCallback(GLFWwindow* handle)
{
  Window& window = windowFromHandle(handle);

  if (window.m_hook)
    window.m_hook->onWindowCloseRequest();

  if (window.m_target)
    window.m_target->onWindowCloseRequest();
}

void Window::keyCallback(GLFWwindow* handle, int key, int scancode, int action, int mods)
{
  Window& window = windowFromHandle(handle);

  if (window.m_hook)
  {
    if (window.m_hook->onKey(Key(key), Action(action)))
      return;
  }

  if (window.m_target)
    window.m_target->onKey(Key(key), Action(action));
}

void Window::characterCallback(GLFWwindow* handle, uint character)
{
  Window& window = windowFromHandle(handle);

  if (window.m_hook)
  {
    if (window.m_hook->onCharacter(character))
      return;
  }

  if (window.m_target)
    window.m_target->onCharacter(character);
}

void Window::cursorPosCallback(GLFWwindow* handle, double x, double y)
{
  Window& window = windowFromHandle(handle);

  if (window.m_hook)
  {
    if (window.m_hook->onCursorPos(vec2(x, y)))
      return;
  }

  if (window.m_target)
    window.m_target->onCursorPos(vec2(x, y));
}

void Window::mouseButtonCallback(GLFWwindow* handle, int button, int action, int mods)
{
  Window& window = windowFromHandle(handle);

  button -= GLFW_MOUSE_BUTTON_1;

  if (window.m_hook)
  {
    if (window.m_hook->onMouseButton(MouseButton(button), Action(action)))
      return;
  }

  if (window.m_target)
    window.m_target->onMouseButton(MouseButton(button), Action(action));
}

void Window::scrollCallback(GLFWwindow* handle, double x, double y)
{
  Window& window = windowFromHandle(handle);

  if (window.m_hook)
  {
    if (window.m_hook->onScroll(vec2(x, y)))
      return;
  }

  if (window.m_target)
    window.m_target->onScroll(vec2(x, y));
}

Window& Window::operator = (const Window& source)
{
  panic("Input windows may not be assigned");
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

void SpectatorController::inputKey(Key key, Action action)
{
  switch (key)
  {
    case KEY_W:
    case KEY_UP:
    {
      if (action == PRESSED)
        directions[FORWARD] = true;
      else if (action == RELEASED)
        directions[FORWARD] = false;
      break;
    }

    case KEY_S:
    case KEY_DOWN:
    {
      if (action == PRESSED)
        directions[BACK] = true;
      else if (action == RELEASED)
        directions[BACK] = false;
      break;
    }

    case KEY_A:
    case KEY_LEFT:
    {
      if (action == PRESSED)
        directions[LEFT] = true;
      else if (action == RELEASED)
        directions[LEFT] = false;
      break;
    }

    case KEY_D:
    case KEY_RIGHT:
    {
      if (action == PRESSED)
        directions[RIGHT] = true;
      else if (action == RELEASED)
        directions[RIGHT] = false;
      break;
    }

    case KEY_LEFT_CONTROL:
    case KEY_RIGHT_CONTROL:
    {
      if (action == PRESSED)
        directions[DOWN] = true;
      else if (action == RELEASED)
        directions[DOWN] = false;
      break;
    }

    case KEY_LEFT_SHIFT:
    case KEY_RIGHT_SHIFT:
    {
      if (action == PRESSED)
        turbo = true;
      else if (action == RELEASED)
        turbo = false;
      break;
    }

    default:
      break;
  }
}

void SpectatorController::inputMouseButton(MouseButton button, Action action)
{
  if (button == MOUSE_BUTTON_RIGHT)
  {
    if (action == PRESSED)
      directions[UP] = true;
    else if (action == RELEASED)
      directions[UP] = false;
  }
}

void SpectatorController::inputCursorOffset(vec2 offset)
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

void SpectatorController::setPosition(vec3 newPosition)
{
  transform.position = newPosition;
}

float SpectatorController::getAngleX() const
{
  return angleX;
}

float SpectatorController::getAngleY() const
{
  return angleY;
}

void SpectatorController::setRotation(float newAngleX, float newAngleY)
{
  angleX = newAngleX;
  angleY = newAngleY;

  const quat axisX = angleAxis(degrees(angleX), 1.f, 0.f, 0.f);
  const quat axisY = angleAxis(degrees(angleY), 0.f, 1.f, 0.f);
  transform.rotation = axisY * axisX;
}

///////////////////////////////////////////////////////////////////////

TextController::TextController():
  caretPosition(0)
{
}

void TextController::inputKey(Key key, Action action)
{
  switch (key)
  {
    case KEY_BACKSPACE:
    {
      if (action == RELEASED)
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
      if (action == RELEASED)
        break;

      if (!text.empty() && caretPosition < text.length())
        text.erase(caretPosition, 1);

      break;
    }

    case KEY_LEFT:
    {
      if (action == RELEASED)
        break;

      if (caretPosition > 0)
        setCaretPosition(caretPosition - 1);
      break;
    }

    case KEY_RIGHT:
    {
      if (action == RELEASED)
        break;

      setCaretPosition(caretPosition + 1);
      break;
    }

    case KEY_HOME:
    {
      if (action == RELEASED)
        break;

      setCaretPosition(0);
      break;
    }

    case KEY_END:
    {
      if (action == RELEASED)
        break;

      setCaretPosition(text.length());
      break;
    }

    case KEY_U:
    {
      if (action != RELEASED && isCtrlKeyDown())
      {
        text.erase(0, caretPosition);
        setCaretPosition(0);
      }

      break;
    }

    case KEY_A:
    {
      if (action != RELEASED && isCtrlKeyDown())
        setCaretPosition(0);

      break;
    }

    case KEY_E:
    {
      if (action != RELEASED && isCtrlKeyDown())
        setCaretPosition(text.length());

      break;
    }

    case KEY_W:
    {
      if (action != RELEASED && isCtrlKeyDown())
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

    default:
      break;
  }
}

void TextController::inputCharacter(uint32 character)
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
  caretPosition = min(text.length(), newPosition);
}

bool TextController::isCtrlKeyDown() const
{
  return false;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
