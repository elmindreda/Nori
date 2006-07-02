///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <moira/Config.h>
#include <moira/Core.h>
#include <moira/Signal.h>
#include <moira/Node.h>
#include <moira/Vector.h>
#include <moira/Rectangle.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLWidget.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Widget::Widget(const String& name):
  Managed<Widget>(name),
  enabled(true),
  visible(true)
{
  area.set(Vector2(0.f, 0.f), Vector2(1.f, 1.f));
}

Widget::~Widget(void)
{
  destroySignal.emit(*this);
}

Widget* Widget::findByPoint(const Vector2& point)
{
  if (!area.contains(point))
    return NULL;

  for (Widget* child = getFirstChild();  child;  child = child->getNextSibling())
  {
    const Vector2 localPoint = point - area.position;

    if (Widget* result = child->findByPoint(localPoint))
      return result;
  }

  return this;
}

bool Widget::isEnabled(void) const
{
  return enabled;
}

bool Widget::isVisible(void) const
{
  return visible;
}

void Widget::setEnabled(bool newState)
{
  enabled = newState;
}

void Widget::setVisible(bool newState)
{
  visible = newState;
}

const Rectangle& Widget::getArea(void) const
{
  return area;
}

const Rectangle& Widget::getGlobalArea(void) const
{
  globalArea = area;

  if (const Widget* parent = getParent())
    globalArea.position += parent->getGlobalArea().position;

  return globalArea;
}

void Widget::setArea(const Rectangle& newArea)
{
  changeAreaSignal.emit(*this, newArea);
  area = newArea;
}

void Widget::setSize(const Vector2& newSize)
{
  Rectangle newArea(area.position, newSize);
  changeAreaSignal.emit(*this, newArea);
  area.size = newSize;
}

void Widget::setPosition(const Vector2& newPosition)
{
  Rectangle newArea(newPosition, area.size);
  changeAreaSignal.emit(*this, newArea);
  area.position = newPosition;
}

SignalProxy1<void, Widget&> Widget::getDestroySignal(void)
{
  return destroySignal;
}

SignalProxy2<void, Widget&, const Rectangle&> Widget::getChangeAreaSignal(void)
{
  return changeAreaSignal;
}

SignalProxy2<void, Widget&, bool> Widget::getChangeFocusSignal(void)
{
  return changeFocusSignal;
}

SignalProxy3<void, Widget&, Key, bool> Widget::getKeyPressSignal(void)
{
  return keyPressSignal;
}

SignalProxy2<void, Widget&, const Vector2&> Widget::getCursorMoveSignal(void)
{
  return cursorMoveSignal;
}

SignalProxy4<void, Widget&, const Vector2&, unsigned int, bool> Widget::getButtonClickSignal(void)
{
  return buttonClickSignal;
}

SignalProxy1<void, Widget&> Widget::getCursorEnterSignal(void)
{
  return cursorEnterSignal;
}

SignalProxy1<void, Widget&> Widget::getCursorLeaveSignal(void)
{
  return cursorLeaveSignal;
}

void Widget::render(void) const
{
  for (const Widget* child = getFirstChild();  child;  child = child->getNextSibling())
    child->render();
}

///////////////////////////////////////////////////////////////////////

Button::Button(const String& name, const String& initTitle):
  Widget(name),
  title(initTitle)
{
}

const String& Button::getTitle(void) const
{
  return title;
}

void Button::setTitle(const String& newTitle)
{
  title = newTitle;
}

SignalProxy2<void, Button&, const String&> Button::getChangeTitleSignal(void)
{
  return changeTitleSignal;
}

SignalProxy1<void, Button&> Button::getPushedSignal(void)
{
  return pushedSignal;
}

void Button::render(void) const
{
  Widget::render();
}

///////////////////////////////////////////////////////////////////////

Window::Window(const String& name, const String& initTitle):
  Widget(name),
  title(initTitle)
{
  Context::get()->getKeyPressSignal().connect(*this, &Window::onKeyPress);
}

void Window::render(void) const
{
  Widget::render();
}

Widget& Window::getActiveWidget(void)
{
  if (!activeWidget.isValid())
    activeWidget = this;

  return *activeWidget;
}

void Window::setActiveWidget(const Widget& child)
{
  activeWidget = child.getName();
}

void Window::onKeyPress(Key key, bool pressed)
{
  switch (key)
  {
  }
}

void Window::onCursorMove(const Vector2& position)
{
}

void Window::onButtonClick(unsigned int button, bool clicked)
{
  Vector2 cursorPosition = Context::get()->getCursorPosition();

  Widget* clickedWidget = findByPoint(cursorPosition);
  if (!clickedWidget)
    return;

  clickedWidget->buttonClickSignal.emit(*clickedWidget, cursorPosition, button, clicked);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
