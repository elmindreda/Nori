//////////////////////////////////////////////////////////////////////
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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLRender.h>
#include <wendy/GLFont.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Widget::Widget(const String& name):
  Managed<Widget>(name),
  parent(NULL),
  enabled(true),
  visible(true),
  underCursor(false)
{
  static bool initialized = false;

  if (!initialized)
  {
    GL::Context* context = GL::Context::get();
    if (!context)
      throw Exception("Cannot create UI widgets without an OpenGL context");

    context->getKeyPressSignal().connect(&Widget::onKeyPress);
    context->getCharInputSignal().connect(&Widget::onCharInput);
    context->getButtonClickSignal().connect(&Widget::onButtonClick);
    context->getCursorMoveSignal().connect(&Widget::onCursorMove);

    initialized = true;
  }

  area.set(0.f, 0.f, 1.f, 1.f);

  roots.push_back(this);
}

Widget::~Widget(void)
{
  if (parent)
    parent->removeChild(*this);
  else
    roots.remove(this);

  if (activeWidget == this)
  {
    if (Widget* parent = getParent())
      parent->activate();
    else if (!roots.empty())
      roots.back()->activate();
    else
    {
      changeFocusSignal.emit(*this, false);
      activeWidget = NULL;
    }
  }

  while (!children.empty())
    delete children.front();

  destroySignal.emit(*this);
}

void Widget::addChild(Widget& child)
{
  for (Widget* widget = this;  widget;  widget = widget->parent)
  {
    if (widget == &child)
      return;
  }

  if (child.parent)
    child.parent->removeChild(child);
  else
    roots.remove(&child);

  children.push_front(&child);
  child.parent = this;

  addedChild(child);
  child.addedToParent(*this);
}

void Widget::removeChild(Widget& child)
{
  List::iterator i = std::find(children.begin(), children.end(), &child);
  if (i != children.end())
  {
    children.erase(i);
    child.parent = NULL;
    roots.push_back(&child);

    removedChild(child);
    child.removedFromParent(*this);
  }
}

Widget* Widget::findByPoint(const Vector2& point)
{
  if (!area.contains(point))
    return NULL;

  const Vector2 localPoint = point - area.position;

  for (List::const_iterator i = children.begin();  i != children.end();  i++)
  {
    if ((*i)->isVisible())
      if (Widget* result = (*i)->findByPoint(localPoint))
	return result;
  }

  return this;
}

Vector2 Widget::transformToLocal(const Vector2& globalPoint) const
{
  return globalPoint - getGlobalArea().position;
}

Vector2 Widget::transformToGlobal(const Vector2& localPoint) const
{
  return localPoint + getGlobalArea().position;
}

void Widget::enable(void)
{
  enabled = true;
}

void Widget::disable(void)
{
  enabled = false;
}

void Widget::show(void)
{
  visible = true;
}

void Widget::hide(void)
{
  visible = false;
}

void Widget::activate(void)
{
  if (!isEnabled())
    return;

  if (activeWidget == this)
    return;

  if (activeWidget)
    activeWidget->changeFocusSignal.emit(*activeWidget, false);

  changeFocusSignal.emit(*this, true);
  activeWidget = this;
}

bool Widget::isEnabled(void) const
{
  return enabled;
}

bool Widget::isVisible(void) const
{
  return visible;
}

bool Widget::isActive(void) const
{
  return activeWidget == this;
}

bool Widget::isUnderCursor(void) const
{
  return underCursor;
}

bool Widget::isBeingDragged(void) const
{
  return draggedWidget == this;
}

Widget* Widget::getParent(void) const
{
  return parent;
}

const Widget::List& Widget::getChildren(void) const
{
  return children;
}

WidgetState Widget::getState(void) const
{
  if (isEnabled())
  {
    if (isActive())
      return STATE_ACTIVE;
    else
      return STATE_NORMAL;
  }
  else
    return STATE_DISABLED;
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

SignalProxy3<void, Widget&, GL::Key, bool> Widget::getKeyPressSignal(void)
{
  return keyPressSignal;
}

SignalProxy2<void, Widget&, wchar_t> Widget::getCharInputSignal(void)
{
  return charInputSignal;
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

SignalProxy2<void, Widget&, const Vector2&> Widget::getDragBeginSignal(void)
{
  return dragBeginSignal;
}

SignalProxy2<void, Widget&, const Vector2&> Widget::getDragMoveSignal(void)
{
  return dragMoveSignal;
}

SignalProxy2<void, Widget&, const Vector2&> Widget::getDragEndSignal(void)
{
  return dragEndSignal;
}

Widget* Widget::getActive(void)
{
  return activeWidget;
}

void Widget::renderRoots(void)
{
  if (!Renderer::get())
  {
    Log::writeError("Cannot render widgets without a widget renderer");
    return;
  }

  GL::Canvas* canvas = GL::Canvas::getCurrent();
  if (!canvas)
  {
    Log::writeError("Cannot render widgets without a current canvas");
    return;
  }

  GL::Renderer* renderer = GL::Renderer::get();
  if (!renderer)
  {
    Log::writeError("Cannot render widgets without a renderer");
    return;
  }

  renderer->begin2D(Vector2((float) canvas->getPhysicalWidth(),
                            (float) canvas->getPhysicalHeight()));

  for (List::iterator i = roots.begin();  i != roots.end();  i++)
  {
    if ((*i)->isVisible())
      (*i)->render();
  }

  renderer->end();
}

void Widget::render(void) const
{
  for (List::const_iterator i = children.begin();  i != children.end();  i++)
  {
    if ((*i)->isVisible())
      (*i)->render();
  }
}

void Widget::addedChild(Widget& child)
{
}

void Widget::removedChild(Widget& child)
{
}

void Widget::addedToParent(Widget& parent)
{
}

void Widget::removedFromParent(Widget& parent)
{
}

void Widget::onKeyPress(GL::Key key, bool pressed)
{
  switch (key)
  {
    default:
    {
      if (activeWidget)
        activeWidget->keyPressSignal.emit(*activeWidget, key, pressed);

      break;
    }
  }
}

void Widget::onCharInput(wchar_t character)
{
  if (activeWidget)
    activeWidget->charInputSignal.emit(*activeWidget, character);
}

void Widget::onCursorMove(const Vector2& position)
{
  if (draggedWidget)
  {
    if (dragging)
      draggedWidget->dragMoveSignal.emit(*draggedWidget, position);
    else
      draggedWidget->dragBeginSignal.emit(*draggedWidget, position);
  }
}

void Widget::onButtonClick(unsigned int button, bool clicked)
{
  GL::Context* context = GL::Context::get();

  Vector2 cursorPosition = context->getCursorPosition();
  cursorPosition.y = context->getHeight() - cursorPosition.y;

  if (clicked)
  {
    Widget* clickedWidget = NULL;

    for (List::iterator i = roots.begin();  i != roots.end();  i++)
    {
      if (clickedWidget = (*i)->findByPoint(cursorPosition))
	break;
    }

    while (clickedWidget && !clickedWidget->isEnabled())
      clickedWidget = clickedWidget->getParent();

    if (clickedWidget)
    {
      clickedWidget->activate();
      clickedWidget->buttonClickSignal.emit(*clickedWidget, cursorPosition, button, clicked);
    }
  }
  else
  {
    if (draggedWidget)
    {
      if (dragging)
      {
	draggedWidget->dragEndSignal.emit(*draggedWidget, cursorPosition);
	dragging = false;
      }

      draggedWidget = NULL;
    }

    if (activeWidget)
      activeWidget->buttonClickSignal.emit(*activeWidget, cursorPosition, button, clicked);
  }
}

bool Widget::dragging = false;

Widget::List Widget::roots;

Widget* Widget::activeWidget = NULL;
Widget* Widget::draggedWidget = NULL;

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
