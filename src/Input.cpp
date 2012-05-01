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

#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#include <wendy/Input.h>

#define GLFW_NO_GLU
#include <GL/glfw3.h>

#include <map>
#include <algorithm>
#include <cstring>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/constants.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace input
  {

///////////////////////////////////////////////////////////////////////

namespace
{

Key externalMap[GLFW_KEY_LAST + 1];
int internalMap[GLFW_KEY_LAST + 1];

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Hook::~Hook()
{
}

bool Hook::onKeyPressed(Key key, bool pressed)
{
  return false;
}

bool Hook::onCharInput(uint32 character)
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

bool Hook::onScrolled(double x, double y)
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

void Target::onCharInput(uint32 character)
{
}

void Target::onButtonClicked(Button button, bool clicked)
{
}

void Target::onCursorMoved(const ivec2& position)
{
}

void Target::onScrolled(double x, double y)
{
}

void Target::onFocusChanged(bool activated)
{
}

///////////////////////////////////////////////////////////////////////

Window::~Window()
{
  glfwSetMousePosCallback(NULL);
  glfwSetMouseButtonCallback(NULL);
  glfwSetKeyCallback(NULL);
  glfwSetCharCallback(NULL);
  glfwSetScrollCallback(NULL);

  instance = NULL;
}

void Window::captureCursor()
{
  glfwSetInputMode(handle, GLFW_CURSOR_MODE, GLFW_CURSOR_CAPTURED);
}

void Window::releaseCursor()
{
  glfwSetInputMode(handle, GLFW_CURSOR_MODE, GLFW_CURSOR_NORMAL);
}

bool Window::isKeyDown(Key key) const
{
  return glfwGetKey(handle, internalMap[key]) == GLFW_PRESS;
}

bool Window::isButtonDown(Button button) const
{
  return glfwGetMouseButton(handle, button + GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
}

bool Window::isCursorCaptured() const
{
  return glfwGetInputMode(handle, GLFW_CURSOR_MODE) == GLFW_CURSOR_CAPTURED;
}

unsigned int Window::getWidth() const
{
  return context.getDefaultFramebuffer().getWidth();
}

unsigned int Window::getHeight() const
{
  return context.getDefaultFramebuffer().getHeight();
}

ivec2 Window::getCursorPosition() const
{
  ivec2 position;
  glfwGetMousePos(handle, &position.x, &position.y);
  return position;
}

void Window::setCursorPosition(const ivec2& newPosition)
{
  glfwSetMousePos(handle, newPosition.x, newPosition.y);
}

Hook* Window::getHook() const
{
  return currentHook;
}

void Window::setHook(Hook* newHook)
{
  currentHook = newHook;
}

Target* Window::getTarget() const
{
  return currentTarget;
}

void Window::setTarget(Target* newTarget)
{
  if (currentTarget == newTarget)
    return;

  if (currentTarget)
    currentTarget->onFocusChanged(false);

  currentTarget = newTarget;

  if (currentTarget)
    currentTarget->onFocusChanged(true);
}

GL::Context& Window::getContext() const
{
  return context;
}

bool Window::createSingleton(GL::Context& context)
{
  set(new Window(context));
  return true;
}

Window::Window(GL::Context& initContext):
  context(initContext),
  handle(NULL),
  currentHook(NULL),
  currentTarget(NULL)
{
  // TODO: Remove this upon the arrival of GLFW_USER_DATA.
  instance = this;

  context.getResizedSignal().connect(*this, &Window::onWindowResized);

  std::memset(externalMap, 0, sizeof(externalMap));
  externalMap[GLFW_KEY_SPACE] = KEY_SPACE;
  externalMap[GLFW_KEY_APOSTROPHE] = KEY_APOSTROPHE;
  externalMap[GLFW_KEY_COMMA] = KEY_COMMA;
  externalMap[GLFW_KEY_MINUS] = KEY_MINUS;
  externalMap[GLFW_KEY_PERIOD] = KEY_PERIOD;
  externalMap[GLFW_KEY_SLASH] = KEY_SLASH;
  externalMap[GLFW_KEY_0] = KEY_0;
  externalMap[GLFW_KEY_1] = KEY_1;
  externalMap[GLFW_KEY_2] = KEY_2;
  externalMap[GLFW_KEY_3] = KEY_3;
  externalMap[GLFW_KEY_4] = KEY_4;
  externalMap[GLFW_KEY_5] = KEY_5;
  externalMap[GLFW_KEY_6] = KEY_6;
  externalMap[GLFW_KEY_7] = KEY_7;
  externalMap[GLFW_KEY_8] = KEY_8;
  externalMap[GLFW_KEY_9] = KEY_9;
  externalMap[GLFW_KEY_SEMICOLON] = KEY_SEMICOLON;
  externalMap[GLFW_KEY_EQUAL] = KEY_EQUAL;
  externalMap[GLFW_KEY_A] = KEY_A;
  externalMap[GLFW_KEY_B] = KEY_B;
  externalMap[GLFW_KEY_C] = KEY_C;
  externalMap[GLFW_KEY_D] = KEY_D;
  externalMap[GLFW_KEY_E] = KEY_E;
  externalMap[GLFW_KEY_F] = KEY_F;
  externalMap[GLFW_KEY_G] = KEY_G;
  externalMap[GLFW_KEY_H] = KEY_H;
  externalMap[GLFW_KEY_I] = KEY_I;
  externalMap[GLFW_KEY_J] = KEY_J;
  externalMap[GLFW_KEY_K] = KEY_K;
  externalMap[GLFW_KEY_L] = KEY_L;
  externalMap[GLFW_KEY_M] = KEY_M;
  externalMap[GLFW_KEY_N] = KEY_N;
  externalMap[GLFW_KEY_O] = KEY_O;
  externalMap[GLFW_KEY_P] = KEY_P;
  externalMap[GLFW_KEY_Q] = KEY_Q;
  externalMap[GLFW_KEY_R] = KEY_R;
  externalMap[GLFW_KEY_S] = KEY_S;
  externalMap[GLFW_KEY_T] = KEY_T;
  externalMap[GLFW_KEY_U] = KEY_U;
  externalMap[GLFW_KEY_V] = KEY_V;
  externalMap[GLFW_KEY_W] = KEY_W;
  externalMap[GLFW_KEY_X] = KEY_X;
  externalMap[GLFW_KEY_Y] = KEY_Y;
  externalMap[GLFW_KEY_Z] = KEY_Z;
  externalMap[GLFW_KEY_LEFT_BRACKET] = KEY_LEFT_BRACKET;
  externalMap[GLFW_KEY_BACKSLASH] = KEY_BACKSLASH;
  externalMap[GLFW_KEY_RIGHT_BRACKET] = KEY_RIGHT_BRACKET;
  externalMap[GLFW_KEY_GRAVE_ACCENT] = KEY_GRAVE_ACCENT;
  externalMap[GLFW_KEY_WORLD_1] = KEY_WORLD_1;
  externalMap[GLFW_KEY_WORLD_2] = KEY_WORLD_2;
  externalMap[GLFW_KEY_ESCAPE] = KEY_ESCAPE;
  externalMap[GLFW_KEY_ENTER] = KEY_ENTER;
  externalMap[GLFW_KEY_TAB] = KEY_TAB;
  externalMap[GLFW_KEY_BACKSPACE] = KEY_BACKSPACE;
  externalMap[GLFW_KEY_INSERT] = KEY_INSERT;
  externalMap[GLFW_KEY_DELETE] = KEY_DELETE;
  externalMap[GLFW_KEY_RIGHT] = KEY_RIGHT;
  externalMap[GLFW_KEY_LEFT] = KEY_LEFT;
  externalMap[GLFW_KEY_DOWN] = KEY_DOWN;
  externalMap[GLFW_KEY_UP] = KEY_UP;
  externalMap[GLFW_KEY_PAGE_UP] = KEY_PAGE_UP;
  externalMap[GLFW_KEY_PAGE_DOWN] = KEY_PAGE_DOWN;
  externalMap[GLFW_KEY_HOME] = KEY_HOME;
  externalMap[GLFW_KEY_END] = KEY_END;
  externalMap[GLFW_KEY_CAPS_LOCK] = KEY_CAPS_LOCK;
  externalMap[GLFW_KEY_SCROLL_LOCK] = KEY_SCROLL_LOCK;
  externalMap[GLFW_KEY_NUM_LOCK] = KEY_NUM_LOCK;
  externalMap[GLFW_KEY_PRINT_SCREEN] = KEY_PRINT_SCREEN;
  externalMap[GLFW_KEY_PAUSE] = KEY_PAUSE;
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
  externalMap[GLFW_KEY_F13] = KEY_F13;
  externalMap[GLFW_KEY_F14] = KEY_F14;
  externalMap[GLFW_KEY_F15] = KEY_F15;
  externalMap[GLFW_KEY_F16] = KEY_F16;
  externalMap[GLFW_KEY_F17] = KEY_F17;
  externalMap[GLFW_KEY_F18] = KEY_F18;
  externalMap[GLFW_KEY_F19] = KEY_F19;
  externalMap[GLFW_KEY_F20] = KEY_F20;
  externalMap[GLFW_KEY_F21] = KEY_F21;
  externalMap[GLFW_KEY_F22] = KEY_F22;
  externalMap[GLFW_KEY_F23] = KEY_F23;
  externalMap[GLFW_KEY_F24] = KEY_F24;
  externalMap[GLFW_KEY_F25] = KEY_F25;
  externalMap[GLFW_KEY_KP_0] = KEY_KP_0;
  externalMap[GLFW_KEY_KP_1] = KEY_KP_1;
  externalMap[GLFW_KEY_KP_2] = KEY_KP_2;
  externalMap[GLFW_KEY_KP_3] = KEY_KP_3;
  externalMap[GLFW_KEY_KP_4] = KEY_KP_4;
  externalMap[GLFW_KEY_KP_5] = KEY_KP_5;
  externalMap[GLFW_KEY_KP_6] = KEY_KP_6;
  externalMap[GLFW_KEY_KP_7] = KEY_KP_7;
  externalMap[GLFW_KEY_KP_8] = KEY_KP_8;
  externalMap[GLFW_KEY_KP_9] = KEY_KP_9;
  externalMap[GLFW_KEY_KP_DECIMAL] = KEY_KP_DECIMAL;
  externalMap[GLFW_KEY_KP_DIVIDE] = KEY_KP_DIVIDE;
  externalMap[GLFW_KEY_KP_MULTIPLY] = KEY_KP_MULTIPLY;
  externalMap[GLFW_KEY_KP_SUBTRACT] = KEY_KP_SUBTRACT;
  externalMap[GLFW_KEY_KP_ADD] = KEY_KP_ADD;
  externalMap[GLFW_KEY_KP_ENTER] = KEY_KP_ENTER;
  externalMap[GLFW_KEY_KP_EQUAL] = KEY_KP_EQUAL;
  externalMap[GLFW_KEY_LEFT_SHIFT] = KEY_LEFT_SHIFT;
  externalMap[GLFW_KEY_LEFT_CONTROL] = KEY_LEFT_CONTROL;
  externalMap[GLFW_KEY_LEFT_ALT] = KEY_LEFT_ALT;
  externalMap[GLFW_KEY_LEFT_SUPER] = KEY_LEFT_SUPER;
  externalMap[GLFW_KEY_RIGHT_SHIFT] = KEY_RIGHT_SHIFT;
  externalMap[GLFW_KEY_RIGHT_CONTROL] = KEY_RIGHT_CONTROL;
  externalMap[GLFW_KEY_RIGHT_ALT] = KEY_RIGHT_ALT;
  externalMap[GLFW_KEY_RIGHT_SUPER] = KEY_RIGHT_SUPER;
  externalMap[GLFW_KEY_MENU] = KEY_MENU;

  std::memset(internalMap, 0, sizeof(internalMap));
  internalMap[KEY_SPACE] = GLFW_KEY_SPACE;
  internalMap[KEY_APOSTROPHE] = GLFW_KEY_APOSTROPHE;
  internalMap[KEY_COMMA] = GLFW_KEY_COMMA;
  internalMap[KEY_MINUS] = GLFW_KEY_MINUS;
  internalMap[KEY_PERIOD] = GLFW_KEY_PERIOD;
  internalMap[KEY_SLASH] = GLFW_KEY_SLASH;
  internalMap[KEY_0] = GLFW_KEY_0;
  internalMap[KEY_1] = GLFW_KEY_1;
  internalMap[KEY_2] = GLFW_KEY_2;
  internalMap[KEY_3] = GLFW_KEY_3;
  internalMap[KEY_4] = GLFW_KEY_4;
  internalMap[KEY_5] = GLFW_KEY_5;
  internalMap[KEY_6] = GLFW_KEY_6;
  internalMap[KEY_7] = GLFW_KEY_7;
  internalMap[KEY_8] = GLFW_KEY_8;
  internalMap[KEY_9] = GLFW_KEY_9;
  internalMap[KEY_SEMICOLON] = GLFW_KEY_SEMICOLON;
  internalMap[KEY_EQUAL] = GLFW_KEY_EQUAL;
  internalMap[KEY_A] = GLFW_KEY_A;
  internalMap[KEY_B] = GLFW_KEY_B;
  internalMap[KEY_C] = GLFW_KEY_C;
  internalMap[KEY_D] = GLFW_KEY_D;
  internalMap[KEY_E] = GLFW_KEY_E;
  internalMap[KEY_F] = GLFW_KEY_F;
  internalMap[KEY_G] = GLFW_KEY_G;
  internalMap[KEY_H] = GLFW_KEY_H;
  internalMap[KEY_I] = GLFW_KEY_I;
  internalMap[KEY_J] = GLFW_KEY_J;
  internalMap[KEY_K] = GLFW_KEY_K;
  internalMap[KEY_L] = GLFW_KEY_L;
  internalMap[KEY_M] = GLFW_KEY_M;
  internalMap[KEY_N] = GLFW_KEY_N;
  internalMap[KEY_O] = GLFW_KEY_O;
  internalMap[KEY_P] = GLFW_KEY_P;
  internalMap[KEY_Q] = GLFW_KEY_Q;
  internalMap[KEY_R] = GLFW_KEY_R;
  internalMap[KEY_S] = GLFW_KEY_S;
  internalMap[KEY_T] = GLFW_KEY_T;
  internalMap[KEY_U] = GLFW_KEY_U;
  internalMap[KEY_V] = GLFW_KEY_V;
  internalMap[KEY_W] = GLFW_KEY_W;
  internalMap[KEY_X] = GLFW_KEY_X;
  internalMap[KEY_Y] = GLFW_KEY_Y;
  internalMap[KEY_Z] = GLFW_KEY_Z;
  internalMap[KEY_LEFT_BRACKET] = GLFW_KEY_LEFT_BRACKET;
  internalMap[KEY_BACKSLASH] = GLFW_KEY_BACKSLASH;
  internalMap[KEY_RIGHT_BRACKET] = GLFW_KEY_RIGHT_BRACKET;
  internalMap[KEY_GRAVE_ACCENT] = GLFW_KEY_GRAVE_ACCENT;
  internalMap[KEY_WORLD_1] = GLFW_KEY_WORLD_1;
  internalMap[KEY_WORLD_2] = GLFW_KEY_WORLD_2;
  internalMap[KEY_ESCAPE] = GLFW_KEY_ESCAPE;
  internalMap[KEY_ENTER] = GLFW_KEY_ENTER;
  internalMap[KEY_TAB] = GLFW_KEY_TAB;
  internalMap[KEY_BACKSPACE] = GLFW_KEY_BACKSPACE;
  internalMap[KEY_INSERT] = GLFW_KEY_INSERT;
  internalMap[KEY_DELETE] = GLFW_KEY_DELETE;
  internalMap[KEY_RIGHT] = GLFW_KEY_RIGHT;
  internalMap[KEY_LEFT] = GLFW_KEY_LEFT;
  internalMap[KEY_DOWN] = GLFW_KEY_DOWN;
  internalMap[KEY_UP] = GLFW_KEY_UP;
  internalMap[KEY_PAGE_UP] = GLFW_KEY_PAGE_UP;
  internalMap[KEY_PAGE_DOWN] = GLFW_KEY_PAGE_DOWN;
  internalMap[KEY_HOME] = GLFW_KEY_HOME;
  internalMap[KEY_END] = GLFW_KEY_END;
  internalMap[KEY_CAPS_LOCK] = GLFW_KEY_CAPS_LOCK;
  internalMap[KEY_SCROLL_LOCK] = GLFW_KEY_SCROLL_LOCK;
  internalMap[KEY_NUM_LOCK] = GLFW_KEY_NUM_LOCK;
  internalMap[KEY_PRINT_SCREEN] = GLFW_KEY_PRINT_SCREEN;
  internalMap[KEY_PAUSE] = GLFW_KEY_PAUSE;
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
  internalMap[KEY_F13] = GLFW_KEY_F13;
  internalMap[KEY_F14] = GLFW_KEY_F14;
  internalMap[KEY_F15] = GLFW_KEY_F15;
  internalMap[KEY_F16] = GLFW_KEY_F16;
  internalMap[KEY_F17] = GLFW_KEY_F17;
  internalMap[KEY_F18] = GLFW_KEY_F18;
  internalMap[KEY_F19] = GLFW_KEY_F19;
  internalMap[KEY_F20] = GLFW_KEY_F20;
  internalMap[KEY_F21] = GLFW_KEY_F21;
  internalMap[KEY_F22] = GLFW_KEY_F22;
  internalMap[KEY_F23] = GLFW_KEY_F23;
  internalMap[KEY_F24] = GLFW_KEY_F24;
  internalMap[KEY_F25] = GLFW_KEY_F25;
  internalMap[KEY_KP_0] = GLFW_KEY_KP_0;
  internalMap[KEY_KP_1] = GLFW_KEY_KP_1;
  internalMap[KEY_KP_2] = GLFW_KEY_KP_2;
  internalMap[KEY_KP_3] = GLFW_KEY_KP_3;
  internalMap[KEY_KP_4] = GLFW_KEY_KP_4;
  internalMap[KEY_KP_5] = GLFW_KEY_KP_5;
  internalMap[KEY_KP_6] = GLFW_KEY_KP_6;
  internalMap[KEY_KP_7] = GLFW_KEY_KP_7;
  internalMap[KEY_KP_8] = GLFW_KEY_KP_8;
  internalMap[KEY_KP_9] = GLFW_KEY_KP_9;
  internalMap[KEY_KP_DECIMAL] = GLFW_KEY_KP_DECIMAL;
  internalMap[KEY_KP_DIVIDE] = GLFW_KEY_KP_DIVIDE;
  internalMap[KEY_KP_MULTIPLY] = GLFW_KEY_KP_MULTIPLY;
  internalMap[KEY_KP_SUBTRACT] = GLFW_KEY_KP_SUBTRACT;
  internalMap[KEY_KP_ADD] = GLFW_KEY_KP_ADD;
  internalMap[KEY_KP_ENTER] = GLFW_KEY_KP_ENTER;
  internalMap[KEY_KP_EQUAL] = GLFW_KEY_KP_EQUAL;
  internalMap[KEY_LEFT_SHIFT] = GLFW_KEY_LEFT_SHIFT;
  internalMap[KEY_LEFT_CONTROL] = GLFW_KEY_LEFT_CONTROL;
  internalMap[KEY_LEFT_ALT] = GLFW_KEY_LEFT_ALT;
  internalMap[KEY_LEFT_SUPER] = GLFW_KEY_LEFT_SUPER;
  internalMap[KEY_RIGHT_SHIFT] = GLFW_KEY_RIGHT_SHIFT;
  internalMap[KEY_RIGHT_CONTROL] = GLFW_KEY_RIGHT_CONTROL;
  internalMap[KEY_RIGHT_ALT] = GLFW_KEY_RIGHT_ALT;
  internalMap[KEY_RIGHT_SUPER] = GLFW_KEY_RIGHT_SUPER;
  internalMap[KEY_MENU] = GLFW_KEY_MENU;

  handle = glfwGetCurrentContext();

  glfwSetMousePosCallback(mousePosCallback);
  glfwSetMouseButtonCallback(mouseButtonCallback);
  glfwSetKeyCallback(keyboardCallback);
  glfwSetCharCallback(characterCallback);
  glfwSetScrollCallback(scrollCallback);

  glfwSetInputMode(handle, GLFW_CURSOR_MODE, GLFW_CURSOR_NORMAL);
}

Window::Window(const Window& source):
  context(source.context)
{
  panic("Input windows may not be copied");
}

void Window::onWindowResized(unsigned int width, unsigned int height)
{
  if (currentTarget)
    currentTarget->onWindowResized(width, height);
}

void Window::keyboardCallback(void* handle, int key, int action)
{
  const bool pressed = (action == GLFW_PRESS) ? true : false;

  if (instance->currentHook)
  {
    if (instance->currentHook->onKeyPressed(externalMap[key], pressed))
      return;
  }

  if (instance->currentTarget)
    instance->currentTarget->onKeyPressed(externalMap[key], pressed);
}

void Window::characterCallback(void* handle, int character)
{
  if (instance->currentHook)
  {
    if (instance->currentHook->onCharInput((uint32) character))
      return;
  }

  if (instance->currentTarget)
    instance->currentTarget->onCharInput((uint32) character);
}

void Window::mousePosCallback(void* handle, int x, int y)
{
  if (instance->currentHook)
  {
    if (instance->currentHook->onCursorMoved(ivec2(x, y)))
      return;
  }

  if (instance->currentTarget)
    instance->currentTarget->onCursorMoved(ivec2(x, y));
}

void Window::mouseButtonCallback(void* handle, int button, int action)
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

void Window::scrollCallback(void* handle, double x, double y)
{
  if (instance->currentHook)
  {
    if (instance->currentHook->onScrolled(x, y))
      return;
  }

  if (instance->currentTarget)
    instance->currentTarget->onScrolled(x, y);
}

Window& Window::operator = (const Window& source)
{
  panic("Input windows may not be assigned");
}

Window* Window::instance = NULL;

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
  Window* window = Window::getSingleton();

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
      window->captureCursor();
  }
  else
  {
    mode = NONE;
    window->releaseCursor();
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

void MayaCamera::onScrolled(double x, double y)
{
  distance += float(y);
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
    case KEY_W:
    case KEY_UP:
    {
      if (pressed)
        directions[FORWARD] = true;
      else
        directions[FORWARD] = false;
      break;
    }

    case KEY_S:
    case KEY_DOWN:
    {
      if (pressed)
        directions[BACK] = true;
      else
        directions[BACK] = false;
      break;
    }

    case KEY_A:
    case KEY_LEFT:
    {
      if (pressed)
        directions[LEFT] = true;
      else
        directions[LEFT] = false;
      break;
    }

    case KEY_D:
    case KEY_RIGHT:
    {
      if (pressed)
        directions[RIGHT] = true;
      else
        directions[RIGHT] = false;
      break;
    }

    case KEY_LEFT_CONTROL:
    case KEY_RIGHT_CONTROL:
    {
      if (pressed)
        directions[DOWN] = true;
      else
        directions[DOWN] = false;
      break;
    }

    case KEY_LEFT_SHIFT:
    case KEY_RIGHT_SHIFT:
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

void TextController::onCharInput(uint32 character)
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
  Window* window = Window::getSingleton();

  return window->isKeyDown(KEY_LEFT_CONTROL) ||
         window->isKeyDown(KEY_RIGHT_CONTROL);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace input*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
