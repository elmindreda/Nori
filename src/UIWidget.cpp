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

#include <wendy/GLContext.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLShader.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>

#include <wendy/RenderMaterial.h>
#include <wendy/RenderFont.h>

#include <wendy/Input.h>

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

Widget::Widget(void):
  parent(NULL),
  enabled(true),
  visible(true),
  draggable(false)
{
  static bool initialized = false;

  if (!initialized)
  {
    input::Context* context = input::Context::get();
    if (!context)
      throw Exception("Cannot create UI widgets without an input context");

    context->getKeyPressedSignal().connect(&Widget::onKeyPressed);
    context->getCharInputSignal().connect(&Widget::onCharInput);
    context->getButtonClickedSignal().connect(&Widget::onButtonClicked);
    context->getCursorMovedSignal().connect(&Widget::onCursorMoved);
    context->getWheelTurnedSignal().connect(&Widget::onWheelTurned);

    initialized = true;
  }

  const float em = Renderer::get()->getDefaultEM();

  area.set(0.f, 0.f, em, em);

  roots.push_back(this);
}

Widget::~Widget(void)
{
  destroyChildren();

  if (parent)
  {
    parent->children.remove(this);
    parent->removedChild(*this);
  }
  else
    roots.remove(this);

  if (activeWidget == this)
  {
    activeWidget = NULL;
    focusChangedSignal.emit(*this, false);
  }

  if (draggedWidget == this)
    draggedWidget = NULL;

  if (hoveredWidget == this)
    hoveredWidget = NULL;

  destroyedSignal.emit(*this);
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

  children.push_back(&child);
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

void Widget::destroyChildren(void)
{
  while (!children.empty())
    delete children.front();
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

void Widget::activate(void)
{
  if (!isVisible() || !isEnabled())
    return;

  if (activeWidget == this)
    return;

  if (activeWidget)
    activeWidget->focusChangedSignal.emit(*activeWidget, false);

  activeWidget = this;
  focusChangedSignal.emit(*this, true);
}

void Widget::bringToFront(void)
{
  List* siblings;

  if (parent)
    siblings = &(parent->children);
  else
    siblings = &roots;

  List::iterator i = std::find(siblings->begin(), siblings->end(), this);
  siblings->erase(i);
  siblings->push_back(this);
}

void Widget::sendToBack(void)
{
  List* siblings;

  if (parent)
    siblings = &(parent->children);
  else
    siblings = &roots;

  List::iterator i = std::find(siblings->begin(), siblings->end(), this);
  siblings->erase(i);
  siblings->push_front(this);
}

void Widget::cancelDragging(void)
{
  if (dragging && draggedWidget == this)
  {
    input::Context* context = input::Context::get();

    draggedWidget->dragEndedSignal.emit(*draggedWidget, context->getCursorPosition());

    draggedWidget = NULL;
    dragging = false;
  }
}

bool Widget::isEnabled(void) const
{
  if (!enabled)
    return false;

  if (parent)
    return parent->isEnabled();

  return true;
}

bool Widget::isVisible(void) const
{
  if (!visible)
    return false;

  if (parent)
    return parent->isVisible();

  return true;
}

bool Widget::isActive(void) const
{
  return activeWidget == this;
}

bool Widget::isUnderCursor(void) const
{
  return hoveredWidget == this;
}

bool Widget::isDraggable(void) const
{
  return draggable;
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

  if (parent)
    globalArea.position += parent->getGlobalArea().position;

  return globalArea;
}

void Widget::setArea(const Rectangle& newArea)
{
  area = newArea;
  areaChangedSignal.emit(*this);
}

void Widget::setSize(const Vector2& newSize)
{
  setArea(Rectangle(area.position, newSize));
}

void Widget::setPosition(const Vector2& newPosition)
{
  setArea(Rectangle(newPosition, area.size));
}

void Widget::setVisible(bool newState)
{
  visible = newState;
}

void Widget::setDraggable(bool newState)
{
  draggable = newState;
  if (draggedWidget == this)
    cancelDragging();
}

SignalProxy1<void, Widget&> Widget::getDestroyedSignal(void)
{
  return destroyedSignal;
}

SignalProxy1<void, Widget&> Widget::getAreaChangedSignal(void)
{
  return areaChangedSignal;
}

SignalProxy2<void, Widget&, bool> Widget::getFocusChangedSignal(void)
{
  return focusChangedSignal;
}

SignalProxy3<void, Widget&, input::Key, bool> Widget::getKeyPressedSignal(void)
{
  return keyPressedSignal;
}

SignalProxy2<void, Widget&, wchar_t> Widget::getCharInputSignal(void)
{
  return charInputSignal;
}

SignalProxy2<void, Widget&, const Vector2&> Widget::getCursorMovedSignal(void)
{
  return cursorMovedSignal;
}

SignalProxy4<void, Widget&, const Vector2&, unsigned int, bool> Widget::getButtonClickedSignal(void)
{
  return buttonClickedSignal;
}

SignalProxy2<void, Widget&, int> Widget::getWheelTurnedSignal(void)
{
  return wheelTurnedSignal;
}

SignalProxy1<void, Widget&> Widget::getCursorEnteredSignal(void)
{
  return cursorEnteredSignal;
}

SignalProxy1<void, Widget&> Widget::getCursorLeftSignal(void)
{
  return cursorLeftSignal;
}

SignalProxy2<void, Widget&, const Vector2&> Widget::getDragBegunSignal(void)
{
  return dragBegunSignal;
}

SignalProxy2<void, Widget&, const Vector2&> Widget::getDragMovedSignal(void)
{
  return dragMovedSignal;
}

SignalProxy2<void, Widget&, const Vector2&> Widget::getDragEndedSignal(void)
{
  return dragEndedSignal;
}

Widget* Widget::getActive(void)
{
  return activeWidget;
}

void Widget::drawRoots(void)
{
  if (!Renderer::get())
  {
    Log::writeError("Cannot draw widgets without a widget renderer");
    return;
  }

  GL::Canvas* canvas = GL::Canvas::getCurrent();
  if (!canvas)
  {
    Log::writeError("Cannot draw widgets without a current canvas");
    return;
  }

  GL::Renderer* renderer = GL::Renderer::get();
  if (!renderer)
  {
    Log::writeError("Cannot draw widgets without a renderer");
    return;
  }

  renderer->begin2D((float) canvas->getPhysicalWidth(),
                    (float) canvas->getPhysicalHeight());

  for (List::iterator i = roots.begin();  i != roots.end();  i++)
  {
    if ((*i)->isVisible())
      (*i)->draw();
  }

  renderer->end();
}

void Widget::draw(void) const
{
  for (List::const_iterator i = children.begin();  i != children.end();  i++)
  {
    if ((*i)->isVisible())
      (*i)->draw();
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

void Widget::onKeyPressed(input::Key key, bool pressed)
{
  switch (key)
  {
    default:
    {
      if (activeWidget)
        activeWidget->keyPressedSignal.emit(*activeWidget, key, pressed);

      break;
    }
  }
}

void Widget::onCharInput(wchar_t character)
{
  if (activeWidget)
    activeWidget->charInputSignal.emit(*activeWidget, character);
}

void Widget::onCursorMoved(const Vector2& position)
{
  GL::Context* context = GL::Context::get();

  Vector2 cursorPosition = position;
  cursorPosition.y = context->getHeight() - cursorPosition.y;

  Widget* newWidget = NULL;

  for (List::reverse_iterator i = roots.rbegin();  i != roots.rend();  i++)
  {
    if ((*i)->isVisible())
      if (newWidget = (*i)->findByPoint(cursorPosition))
	break;
  }

  if (newWidget != hoveredWidget)
  {
    if (hoveredWidget)
      hoveredWidget->cursorLeftSignal.emit(*hoveredWidget);

    hoveredWidget = newWidget;

    if (newWidget)
      newWidget->cursorEnteredSignal.emit(*newWidget);
  }

  if (hoveredWidget)
    hoveredWidget->cursorMovedSignal.emit(*hoveredWidget, cursorPosition);

  if (draggedWidget)
  {
    if (dragging)
      draggedWidget->dragMovedSignal.emit(*draggedWidget, cursorPosition);
    else
    {
      // TODO: Add insensitivity.

      dragging = true;
      draggedWidget->dragBegunSignal.emit(*draggedWidget, cursorPosition);
    }
  }
}

void Widget::onButtonClicked(unsigned int button, bool clicked)
{
  input::Context* context = input::Context::get();

  Vector2 cursorPosition = context->getCursorPosition();
  cursorPosition.y = context->getHeight() - cursorPosition.y;

  if (clicked)
  {
    Widget* clickedWidget = NULL;

    for (List::reverse_iterator i = roots.rbegin();  i != roots.rend();  i++)
    {
      if ((*i)->isVisible())
	if (clickedWidget = (*i)->findByPoint(cursorPosition))
	  break;
    }

    while (clickedWidget && !clickedWidget->isEnabled())
      clickedWidget = clickedWidget->getParent();

    if (clickedWidget)
    {
      clickedWidget->activate();
      clickedWidget->buttonClickedSignal.emit(*clickedWidget,
                                              cursorPosition,
					      button,
					      clicked);

      if (button == 0 && clickedWidget->isDraggable())
	draggedWidget = clickedWidget;
    }
  }
  else
  {
    if (draggedWidget)
    {
      if (dragging)
      {
	draggedWidget->dragEndedSignal.emit(*draggedWidget, cursorPosition);
	dragging = false;
      }

      draggedWidget = NULL;
    }

    if (activeWidget && activeWidget->getGlobalArea().contains(cursorPosition))
      activeWidget->buttonClickedSignal.emit(*activeWidget,
					     cursorPosition,
					     button,
					     clicked);
  }
}

void Widget::onWheelTurned(int offset)
{
  if (hoveredWidget)
    hoveredWidget->wheelTurnedSignal.emit(*hoveredWidget, offset);
}

bool Widget::dragging = false;

Widget::List Widget::roots;

Widget* Widget::activeWidget = NULL;
Widget* Widget::draggedWidget = NULL;
Widget* Widget::hoveredWidget = NULL;

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
