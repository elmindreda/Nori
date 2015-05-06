///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

#include <nori/Config.hpp>

#include <nori/Core.hpp>
#include <nori/Time.hpp>
#include <nori/Profile.hpp>
#include <nori/Window.hpp>
#include <nori/Texture.hpp>
#include <nori/RenderBuffer.hpp>
#include <nori/Program.hpp>
#include <nori/RenderContext.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <utf8.h>

#include <algorithm>
#include <cstring>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/constants.hpp>

namespace nori
{

namespace
{

Window& windowFromHandle(GLFWwindow* handle)
{
  return ((RenderContext*) glfwGetWindowUserPointer(handle))->window();
}

} /*namespace*/

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

void EventHook::onConnected(const Gamepad& gamepad)
{
}

void EventHook::onDisconnected(const Gamepad& gamepad)
{
}

bool EventHook::onKey(Key key, Action action, uint mods)
{
  return false;
}

bool EventHook::onCharacter(uint32 codepoint)
{
  return false;
}

bool EventHook::onMouseButton(MouseButton button, Action action, uint mods)
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

void EventTarget::onConnected(const Gamepad& gamepad)
{
}

void EventTarget::onDisconnected(const Gamepad& gamepad)
{
}

void EventTarget::onKey(Key key, Action action, uint mods)
{
}

void EventTarget::onCharacter(uint32 codepoint)
{
}

void EventTarget::onMouseButton(MouseButton button, Action action, uint mods)
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

Resolution::Resolution():
  width(0),
  height(0)
{
}

Resolution::Resolution(uint width, uint height):
  width(width),
  height(height)
{
}

WindowConfig::WindowConfig():
  title("Nori"),
  width(640),
  height(480),
  mode(WINDOWED),
  resizable(true)
{
}

WindowConfig::WindowConfig(const std::string& title):
  title(title),
  width(640),
  height(480),
  mode(WINDOWED),
  resizable(true)
{
}

WindowConfig::WindowConfig(const std::string& title,
                           uint width,
                           uint height,
                           WindowMode mode,
                           bool resizable):
  title(title),
  width(width),
  height(height),
  mode(mode),
  resizable(resizable)
{
}

bool Gamepad::isButtonDown(uint button) const
{
  assert(button < m_buttons.size());
  return m_buttons[button];
}

uint Gamepad::buttonCount() const
{
  return uint(m_buttons.size());
}

float Gamepad::axis(uint axis) const
{
  assert(axis < m_axes.size());
  return m_axes[axis];
}

uint Gamepad::axisCount() const
{
  return uint(m_axes.size());
}

const char* Gamepad::name() const
{
  return glfwGetJoystickName(GLFW_JOYSTICK_1);
}

Gamepad::Gamepad()
{
  int count;

  glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
  m_buttons.resize(count);

  glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);
  m_axes.resize(count);
}

void Gamepad::update()
{
  int count;

  const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
  assert(count == m_buttons.size());

  for (int i = 0;  i < count;  i++)
    m_buttons[i] = buttons[i] ? true : false;

  const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);
  assert(count == m_axes.size());

  for (int i = 0;  i < count;  i++)
    m_axes[i] = axes[i];
}

bool Gamepad::present()
{
  return glfwJoystickPresent(GLFW_JOYSTICK_1) ? true : false;
}

Window::~Window()
{
}

bool Window::update()
{
  ProfileNodeCall call("Window::update");

  if (Gamepad::present())
  {
    if (!m_gamepad)
    {
      m_gamepad.reset(new Gamepad());

      if (m_hook)
        m_hook->onConnected(*m_gamepad);

      if (m_target)
        m_target->onConnected(*m_gamepad);
    }

    m_gamepad->update();
  }
  else
  {
    if (m_hook)
      m_hook->onDisconnected(*m_gamepad);

    if (m_target)
      m_target->onDisconnected(*m_gamepad);

    m_gamepad = nullptr;
  }

  glfwSwapBuffers(m_handle);
  m_needsRefresh = false;
  m_updated();

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
  return glfwWindowShouldClose(m_handle) ? true : false;
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

std::string Window::clipboardText() const
{
  return glfwGetClipboardString(m_handle);
}

void Window::setClipboardText(const std::string& newText)
{
  glfwSetClipboardString(m_handle, newText.c_str());
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
    if (window.m_hook->onKey(Key(key), Action(action), mods))
      return;
  }

  if (window.m_target)
    window.m_target->onKey(Key(key), Action(action), mods);
}

void Window::characterCallback(GLFWwindow* handle, uint codepoint)
{
  Window& window = windowFromHandle(handle);

  if (window.m_hook)
  {
    if (window.m_hook->onCharacter(codepoint))
      return;
  }

  if (window.m_target)
    window.m_target->onCharacter(codepoint);
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
    if (window.m_hook->onMouseButton(MouseButton(button), Action(action), mods))
      return;
  }

  if (window.m_target)
    window.m_target->onMouseButton(MouseButton(button), Action(action), mods);
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

std::vector<Resolution> Window::resolutions()
{
  std::vector<Resolution> resolutions;

  int count;
  const GLFWvidmode* modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);

  for (int i = 0;  i < count;  i++)
  {
    if (modes[i].redBits != 8 ||
        modes[i].greenBits != 8 ||
        modes[i].blueBits != 8)
    {
      continue;
    }

    resolutions.push_back(Resolution(modes[i].width, modes[i].height));
  }

  return resolutions;
}

SpectatorController::SpectatorController():
  m_angleX(0.f),
  m_angleY(0.f),
  m_speed(3.f),
  m_turbo(false)
{
  std::memset(m_directions, 0, sizeof(m_directions));
}

void SpectatorController::update(Time deltaTime)
{
  float multiplier;

  if (m_turbo)
    multiplier = 3.f;
  else
    multiplier = 1.f;

  vec3 direction;

  if (m_directions[UP])
    direction.y += 1.f;
  if (m_directions[DOWN])
    direction.y -= 1.f;
  if (m_directions[FORWARD])
    direction.z -= 1.f;
  if (m_directions[BACK])
    direction.z += 1.f;
  if (m_directions[LEFT])
    direction.x -= 1.f;
  if (m_directions[RIGHT])
    direction.x += 1.f;

  if (length2(direction))
    direction = normalize(m_transform.rotation * direction);

  m_transform.position += direction * m_speed * multiplier * float(deltaTime);
}

void SpectatorController::release()
{
  std::memset(m_directions, 0, sizeof(m_directions));
  m_turbo = false;
}

void SpectatorController::inputKey(Key key, Action action, uint mods)
{
  switch (key)
  {
    case KEY_W:
    case KEY_UP:
    {
      if (action == PRESSED)
        m_directions[FORWARD] = true;
      else if (action == RELEASED)
        m_directions[FORWARD] = false;
      break;
    }

    case KEY_S:
    case KEY_DOWN:
    {
      if (action == PRESSED)
        m_directions[BACK] = true;
      else if (action == RELEASED)
        m_directions[BACK] = false;
      break;
    }

    case KEY_A:
    case KEY_LEFT:
    {
      if (action == PRESSED)
        m_directions[LEFT] = true;
      else if (action == RELEASED)
        m_directions[LEFT] = false;
      break;
    }

    case KEY_D:
    case KEY_RIGHT:
    {
      if (action == PRESSED)
        m_directions[RIGHT] = true;
      else if (action == RELEASED)
        m_directions[RIGHT] = false;
      break;
    }

    case KEY_LEFT_CONTROL:
    case KEY_RIGHT_CONTROL:
    {
      if (action == PRESSED)
        m_directions[DOWN] = true;
      else if (action == RELEASED)
        m_directions[DOWN] = false;
      break;
    }

    case KEY_LEFT_SHIFT:
    case KEY_RIGHT_SHIFT:
    {
      if (action == PRESSED)
        m_turbo = true;
      else if (action == RELEASED)
        m_turbo = false;
      break;
    }

    default:
      break;
  }
}

void SpectatorController::inputMouseButton(MouseButton button,
                                           Action action,
                                           uint mods)
{
  if (button == MOUSE_BUTTON_RIGHT)
  {
    if (action == PRESSED)
      m_directions[UP] = true;
    else if (action == RELEASED)
      m_directions[UP] = false;
  }
}

void SpectatorController::inputCursorOffset(vec2 offset)
{
  const float scale = 1.f / 250.f;
  const float limit = half_pi<float>() - 0.01f;

  setRotation(clamp(m_angleX - offset.y * scale, -limit, limit),
              m_angleY - offset.x * scale);
}

void SpectatorController::setSpeed(float newSpeed)
{
  m_speed = newSpeed;
}

void SpectatorController::setPosition(vec3 newPosition)
{
  m_transform.position = newPosition;
}

void SpectatorController::setRotation(float newAngleX, float newAngleY)
{
  m_angleX = newAngleX;
  m_angleY = newAngleY;

  const quat axisX = angleAxis(m_angleX, vec3(1.f, 0.f, 0.f));
  const quat axisY = angleAxis(m_angleY, vec3(0.f, 1.f, 0.f));
  m_transform.rotation = axisY * axisX;
}

TextController::TextController():
  m_caretPosition(0)
{
}

TextController::TextController(const std::string& text):
  m_text(text),
  m_caretPosition(0)
{
}

void TextController::inputKey(Key key, Action action, uint mods)
{
  switch (key)
  {
    case KEY_BACKSPACE:
    {
      if ((action == PRESSED || action == REPEATED) && mods == 0)
      {
        if (caretPosition() > 0)
        {
          std::string::iterator s = m_text.begin();
          utf8::advance(s, caretPosition() - 1, m_text.end());
          std::string::iterator e = s;
          utf8::next(e, m_text.end());
          m_text.erase(s, e);
          m_textChanged();
          setCaretPosition(caretPosition() - 1);
        }
      }

      break;
    }

    case KEY_DELETE:
    {
      if ((action == PRESSED || action == REPEATED) && mods == 0)
      {
        if (caretPosition() < length())
        {
          std::string::iterator s = m_text.begin();
          utf8::advance(s, caretPosition(), m_text.end());
          std::string::iterator e = s;
          utf8::next(e, m_text.end());
          m_text.erase(s, e);
          m_textChanged();
        }
      }

      break;
    }

    case KEY_LEFT:
    {
      if ((action == PRESSED || action == REPEATED) && mods == 0)
      {
        if (caretPosition() > 0)
          setCaretPosition(caretPosition() - 1);
      }

      break;
    }

    case KEY_RIGHT:
    {
      if ((action == PRESSED || action == REPEATED) && mods == 0)
        setCaretPosition(caretPosition() + 1);

      break;
    }

    case KEY_HOME:
    {
      if ((action == PRESSED || action == REPEATED) && mods == 0)
        setCaretPosition(0);

      break;
    }

    case KEY_END:
    {
      if ((action == PRESSED || action == REPEATED) && mods == 0)
        setCaretPosition(length());

      break;
    }

    case KEY_U:
    {
      if ((action == PRESSED || action == REPEATED) && mods == MOD_CONTROL)
      {
        std::string::iterator e = m_text.begin();
        utf8::advance(e, caretPosition(), m_text.end());
        m_text.erase(m_text.begin(), e);
        setCaretPosition(0);
        m_textChanged();
      }

      break;
    }

    case KEY_A:
    {
      if ((action == PRESSED || action == REPEATED) && mods == MOD_CONTROL)
        setCaretPosition(0);

      break;
    }

    case KEY_E:
    {
      if ((action == PRESSED || action == REPEATED) && mods == MOD_CONTROL)
        setCaretPosition(length());

      break;
    }

    case KEY_W:
    {
      if ((action == PRESSED || action == REPEATED) && mods == MOD_CONTROL)
      {
        std::string::iterator e = m_text.begin();
        utf8::advance(e, caretPosition(), m_text.end());
        std::string::iterator s = e;

        while (s != m_text.begin())
        {
          if (utf8::prior(s, m_text.begin()) != ' ')
            break;
        }

        while (s != m_text.begin())
        {
          if (utf8::prior(s, m_text.begin()) == ' ')
          {
            s++;
            break;
          }
        }

        setCaretPosition(s - m_text.begin());
        m_text.erase(s, e);
        m_textChanged();
      }

      break;
    }

    default:
      break;
  }
}

void TextController::inputCharacter(uint32 codepoint)
{
  std::string::iterator i = m_text.begin();
  utf8::advance(i, caretPosition(), m_text.end());
  utf8::append(codepoint, std::inserter(m_text, i));
  setCaretPosition(caretPosition() + 1);
  m_textChanged();
}

size_t TextController::length() const
{
  return utf8::distance(m_text.begin(), m_text.end());
}

void TextController::setText(const std::string& newText)
{
  m_text = newText;
  setCaretPosition(caretPosition());
  m_textChanged();
}

void TextController::setCaretPosition(size_t newPosition)
{
  m_caretPosition = min(length(), newPosition);
  m_caretMoved();
}

} /*namespace nori*/

