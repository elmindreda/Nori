//////////////////////////////////////////////////////////////////////
// Wendy user interface library
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

#include <wendy/Config.h>

#include <wendy/UIDrawer.h>
#include <wendy/UILayer.h>
#include <wendy/UIWidget.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Widget::Widget(Layer& initlayer):
  layer(initlayer),
  parent(NULL),
  enabled(true),
  visible(true),
  draggable(false)
{
  assert(&layer);
}

Widget::~Widget()
{
  destroyChildren();
  removeFromParent();

  destroyedSignal(*this);
}

void Widget::addChild(Widget& child)
{
  assert(&layer == &(child.getLayer()));
  assert(&child != this);
  assert(!isChildOf(child));

  child.removeFromParent();
  child.parent = this;
  children.push_back(&child);
  addedChild(child);
  child.addedToParent(*this);

  invalidate();
}

void Widget::destroyChildren()
{
  while (!children.empty())
    delete children.back();

  invalidate();
}

void Widget::removeFromParent()
{
  WidgetList* siblings;

  if (parent)
    siblings = &(parent->children);
  else
    siblings = &(layer.roots);

  auto s = std::find(siblings->begin(), siblings->end(), this);
  if (s == siblings->end())
    return;

  siblings->erase(s);
  layer.removedWidget(*this);

  if (parent)
  {
    Widget* oldParent = parent;
    parent = NULL;

    oldParent->removedChild(*this);
    removedFromParent(*oldParent);
  }
}

Widget* Widget::findByPoint(const vec2& point)
{
  if (!area.contains(point))
    return NULL;

  const vec2 localPoint = point - area.position;

  for (auto c = children.begin();  c != children.end();  c++)
  {
    if ((*c)->isVisible())
      if (Widget* result = (*c)->findByPoint(localPoint))
        return result;
  }

  return this;
}

vec2 Widget::transformToLocal(const vec2& globalPoint) const
{
  return globalPoint - getGlobalArea().position;
}

vec2 Widget::transformToGlobal(const vec2& localPoint) const
{
  return localPoint + getGlobalArea().position;
}

void Widget::show()
{
  if (!visible)
  {
    visible = true;
    invalidate();
  }
}

void Widget::hide()
{
  if (visible)
  {
    visible = false;
    invalidate();
  }
}

void Widget::enable()
{
  if (!enabled)
  {
    enabled = true;
    invalidate();
  }
}

void Widget::disable()
{
  if (enabled)
  {
    enabled = false;
    // TODO: Handle deactivation
    invalidate();
  }
}

void Widget::invalidate()
{
  layer.invalidate();
}

void Widget::activate()
{
  layer.setActiveWidget(this);
}

void Widget::bringToFront()
{
  WidgetList* siblings;

  if (parent)
    siblings = &(parent->children);
  else
    siblings = &(layer.roots);

  siblings->erase(std::find(siblings->begin(), siblings->end(), this));
  siblings->push_back(this);

  invalidate();
}

void Widget::sendToBack()
{
  WidgetList* siblings;

  if (parent)
    siblings = &(parent->children);
  else
    siblings = &(layer.roots);

  siblings->erase(std::find(siblings->begin(), siblings->end(), this));
  siblings->insert(siblings->begin(), this);

  invalidate();
}

void Widget::cancelDragging()
{
  if (isBeingDragged())
    layer.cancelDragging();
}

bool Widget::isEnabled() const
{
  if (!enabled)
    return false;

  if (parent)
    return parent->isEnabled();

  return true;
}

bool Widget::isVisible() const
{
  if (!visible)
    return false;

  if (parent)
    return parent->isVisible();

  return true;
}

bool Widget::isActive() const
{
  return layer.getActiveWidget() == this;
}

bool Widget::isUnderCursor() const
{
  return layer.getHoveredWidget() == this;
}

bool Widget::isDraggable() const
{
  return draggable;
}

bool Widget::isBeingDragged() const
{
  return layer.getDraggedWidget() == this;
}

bool Widget::isChildOf(const Widget& widget) const
{
  assert(&layer == &(widget.layer));

  for (Widget* ancestor = parent;  ancestor;  ancestor = ancestor->parent)
  {
    if (ancestor == &widget)
      return true;
  }

  return false;
}

bool Widget::hasCapturedCursor() const
{
  return layer.captureWidget == this;
}

Layer& Widget::getLayer() const
{
  return layer;
}

Widget* Widget::getParent() const
{
  return parent;
}

const WidgetList& Widget::getChildren() const
{
  return children;
}

WidgetState Widget::getState() const
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

float Widget::getWidth() const
{
  return area.size.x;
}

float Widget::getHeight() const
{
  return area.size.y;
}

const Rect& Widget::getArea() const
{
  return area;
}

const Rect& Widget::getGlobalArea() const
{
  globalArea = area;

  if (parent)
    globalArea.position += parent->getGlobalArea().position;

  return globalArea;
}

void Widget::setArea(const Rect& newArea)
{
  if (newArea != area)
  {
    area = newArea;
    areaChangedSignal(*this);

    invalidate();
  }
}

const vec2& Widget::getSize() const
{
  return area.size;
}

void Widget::setSize(const vec2& newSize)
{
  setArea(Rect(area.position, newSize));
}

void Widget::setPosition(const vec2& newPosition)
{
  setArea(Rect(newPosition, area.size));
}

void Widget::setDraggable(bool newState)
{
  draggable = newState;

  if (!draggable)
    cancelDragging();
}

SignalProxy1<void, Widget&> Widget::getDestroyedSignal()
{
  return destroyedSignal;
}

SignalProxy1<void, Widget&> Widget::getAreaChangedSignal()
{
  return areaChangedSignal;
}

SignalProxy2<void, Widget&, bool> Widget::getFocusChangedSignal()
{
  return focusChangedSignal;
}

SignalProxy3<void, Widget&, input::Key, bool> Widget::getKeyPressedSignal()
{
  return keyPressedSignal;
}

SignalProxy2<void, Widget&, uint32> Widget::getCharInputSignal()
{
  return charInputSignal;
}

SignalProxy2<void, Widget&, const vec2&> Widget::getCursorMovedSignal()
{
  return cursorMovedSignal;
}

SignalProxy4<void, Widget&, const vec2&, input::Button, bool> Widget::getButtonClickedSignal()
{
  return buttonClickedSignal;
}

SignalProxy3<void, Widget&, double, double> Widget::getScrolledSignal()
{
  return scrolledSignal;
}

SignalProxy1<void, Widget&> Widget::getCursorEnteredSignal()
{
  return cursorEnteredSignal;
}

SignalProxy1<void, Widget&> Widget::getCursorLeftSignal()
{
  return cursorLeftSignal;
}

SignalProxy2<void, Widget&, const vec2&> Widget::getDragBegunSignal()
{
  return dragBegunSignal;
}

SignalProxy2<void, Widget&, const vec2&> Widget::getDragMovedSignal()
{
  return dragMovedSignal;
}

SignalProxy2<void, Widget&, const vec2&> Widget::getDragEndedSignal()
{
  return dragEndedSignal;
}

void Widget::draw() const
{
  for (auto c = children.begin();  c != children.end();  c++)
  {
    if ((*c)->isVisible())
      (*c)->draw();
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

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
