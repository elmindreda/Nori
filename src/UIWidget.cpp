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

#include <wendy/Config.hpp>

#include <wendy/UIDrawer.hpp>
#include <wendy/UILayer.hpp>
#include <wendy/UIWidget.hpp>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Widget::Widget(Layer& layer):
  m_layer(layer),
  m_parent(nullptr),
  m_enabled(true),
  m_visible(true),
  m_draggable(false)
{
  assert(&m_layer);
}

Widget::~Widget()
{
  destroyChildren();
  removeFromParent();

  m_destroyedSignal(*this);
}

void Widget::addChild(Widget& child)
{
  assert(&m_layer == &(child.layer()));
  assert(&child != this);
  assert(!isChildOf(child));

  child.removeFromParent();
  child.m_parent = this;
  m_children.push_back(&child);
  addedChild(child);
  child.addedToParent(*this);

  invalidate();
}

void Widget::destroyChildren()
{
  while (!m_children.empty())
    delete m_children.back();

  invalidate();
}

void Widget::removeFromParent()
{
  WidgetList* siblings;

  if (m_parent)
    siblings = &(m_parent->m_children);
  else
    siblings = &(m_layer.m_roots);

  auto s = std::find(siblings->begin(), siblings->end(), this);
  if (s == siblings->end())
    return;

  siblings->erase(s);
  m_layer.removedWidget(*this);

  if (m_parent)
  {
    Widget* oldParent = m_parent;
    m_parent = nullptr;

    oldParent->removedChild(*this);
    removedFromParent(*oldParent);
  }
}

Widget* Widget::findByPoint(vec2 point)
{
  if (!m_area.contains(point))
    return nullptr;

  const vec2 localPoint = point - m_area.position;

  for (auto c : m_children)
  {
    if (c->isVisible())
    {
      if (Widget* result = c->findByPoint(localPoint))
        return result;
    }
  }

  return this;
}

vec2 Widget::transformToLocal(vec2 globalPoint) const
{
  return globalPoint - globalPos();
}

vec2 Widget::transformToGlobal(vec2 localPoint) const
{
  return localPoint + globalPos();
}

void Widget::show()
{
  if (!m_visible)
  {
    m_visible = true;
    invalidate();
  }
}

void Widget::hide()
{
  if (m_visible)
  {
    m_visible = false;
    invalidate();
  }
}

void Widget::enable()
{
  if (!m_enabled)
  {
    m_enabled = true;
    invalidate();
  }
}

void Widget::disable()
{
  if (m_enabled)
  {
    m_enabled = false;
    // TODO: Handle deactivation
    invalidate();
  }
}

void Widget::invalidate()
{
  m_layer.invalidate();
}

void Widget::activate()
{
  m_layer.setActiveWidget(this);
}

void Widget::bringToFront()
{
  WidgetList* siblings;

  if (m_parent)
    siblings = &(m_parent->m_children);
  else
    siblings = &(m_layer.m_roots);

  siblings->erase(std::find(siblings->begin(), siblings->end(), this));
  siblings->push_back(this);

  invalidate();
}

void Widget::sendToBack()
{
  WidgetList* siblings;

  if (m_parent)
    siblings = &(m_parent->m_children);
  else
    siblings = &(m_layer.m_roots);

  siblings->erase(std::find(siblings->begin(), siblings->end(), this));
  siblings->insert(siblings->begin(), this);

  invalidate();
}

void Widget::cancelDragging()
{
  if (isBeingDragged())
    m_layer.cancelDragging();
}

bool Widget::isEnabled() const
{
  if (!m_enabled)
    return false;

  if (m_parent)
    return m_parent->isEnabled();

  return true;
}

bool Widget::isVisible() const
{
  if (!m_visible)
    return false;

  if (m_parent)
    return m_parent->isVisible();

  return true;
}

bool Widget::isActive() const
{
  return m_layer.activeWidget() == this;
}

bool Widget::isUnderCursor() const
{
  return m_layer.hoveredWidget() == this;
}

bool Widget::isBeingDragged() const
{
  return m_layer.draggedWidget() == this;
}

bool Widget::isChildOf(const Widget& widget) const
{
  assert(&m_layer == &(widget.m_layer));

  for (Widget* ancestor = m_parent;  ancestor;  ancestor = ancestor->m_parent)
  {
    if (ancestor == &widget)
      return true;
  }

  return false;
}

bool Widget::hasCapturedCursor() const
{
  return m_layer.m_captureWidget == this;
}

WidgetState Widget::state() const
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

vec2 Widget::globalPos() const
{
  if (m_parent)
    return m_area.position + m_parent->globalPos();

  return m_area.position;
}

Rect Widget::globalArea() const
{
  if (m_parent)
    return Rect(m_area.position + m_parent->globalPos(), m_area.size);

  return m_area;
}

void Widget::setArea(const Rect& newArea)
{
  if (newArea != m_area)
  {
    m_area = newArea;
    m_areaChangedSignal(*this);

    invalidate();
  }
}

void Widget::setSize(vec2 newSize)
{
  setArea(Rect(m_area.position, newSize));
}

void Widget::setPosition(vec2 newPosition)
{
  setArea(Rect(newPosition, m_area.size));
}

void Widget::setDraggable(bool newState)
{
  m_draggable = newState;

  if (!m_draggable)
    cancelDragging();
}

SignalProxy1<void, Widget&> Widget::destroyedSignal()
{
  return m_destroyedSignal;
}

SignalProxy1<void, Widget&> Widget::areaChangedSignal()
{
  return m_areaChangedSignal;
}

SignalProxy2<void, Widget&, bool> Widget::focusChangedSignal()
{
  return m_focusChangedSignal;
}

SignalProxy4<void, Widget&, Key, Action, uint> Widget::keyPressedSignal()
{
  return m_keyPressedSignal;
}

SignalProxy3<void, Widget&, uint32, uint> Widget::charInputSignal()
{
  return m_charInputSignal;
}

SignalProxy2<void, Widget&, vec2> Widget::cursorMovedSignal()
{
  return m_cursorMovedSignal;
}

SignalProxy5<void, Widget&, vec2, MouseButton, Action, uint> Widget::buttonClickedSignal()
{
  return m_buttonClickedSignal;
}

SignalProxy2<void, Widget&, vec2> Widget::scrolledSignal()
{
  return m_scrolledSignal;
}

SignalProxy1<void, Widget&> Widget::cursorEnteredSignal()
{
  return m_cursorEnteredSignal;
}

SignalProxy1<void, Widget&> Widget::cursorLeftSignal()
{
  return m_cursorLeftSignal;
}

SignalProxy2<void, Widget&, vec2> Widget::dragBegunSignal()
{
  return m_dragBegunSignal;
}

SignalProxy2<void, Widget&, vec2> Widget::dragMovedSignal()
{
  return m_dragMovedSignal;
}

SignalProxy2<void, Widget&, vec2> Widget::dragEndedSignal()
{
  return m_dragEndedSignal;
}

void Widget::draw() const
{
  for (auto c : m_children)
  {
    if (c->isVisible())
      c->draw();
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
