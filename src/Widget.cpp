//////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#include <wendy/Drawer.hpp>
#include <wendy/Layer.hpp>
#include <wendy/Widget.hpp>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Widget::Widget(Layer& layer):
  m_layer(layer),
  m_parent(nullptr),
  m_enabled(true),
  m_visible(true),
  m_draggable(false),
  m_focusable(false)
{
  m_layer.m_roots.push_back(this);
  invalidate();
}

Widget::Widget(Widget& parent):
  m_layer(parent.layer()),
  m_parent(&parent),
  m_enabled(true),
  m_visible(true),
  m_draggable(false),
  m_focusable(false)
{
  m_parent->m_children.push_back(this);
  m_parent->onChildAdded(*this);

  invalidate();
}

Widget::~Widget()
{
  invalidate();
  m_destroyedSignal(*this);
  destroyChildren();

  std::vector<Widget*>* siblings;

  if (m_parent)
    siblings = &(m_parent->m_children);
  else
    siblings = &(m_layer.m_roots);

  siblings->erase(std::find(siblings->begin(), siblings->end(), this));
  m_layer.removedWidget(*this);
}

void Widget::destroyChildren()
{
  while (!m_children.empty())
    delete m_children.back();

  invalidate();
}

Widget* Widget::findByPoint(vec2 point)
{
  if (!m_area.contains(point))
    return nullptr;

  const vec2 localPoint = point - m_area.position;

  for (auto c = m_children.rbegin();  c != m_children.rend();  c++)
  {
    if ((*c)->isVisible())
    {
      if (Widget* result = (*c)->findByPoint(localPoint))
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
  std::vector<Widget*>* siblings;

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
  std::vector<Widget*>* siblings;

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
    onAreaChanged();

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

void Widget::setDesiredSize(vec2 newSize)
{
  m_desired = newSize;

  if (m_parent)
    m_parent->onChildDesiredSizeChanged(*this);
}

void Widget::setFocusable(bool focusable)
{
  m_focusable = focusable;
}

void Widget::setDraggable(bool newState)
{
  m_draggable = newState;

  if (!m_draggable)
    cancelDragging();
}

SignalProxy<void, Widget&> Widget::destroyedSignal()
{
  return m_destroyedSignal;
}

SignalProxy<void, Widget&> Widget::areaChangedSignal()
{
  return m_areaChangedSignal;
}

SignalProxy<void, Widget&, bool> Widget::focusChangedSignal()
{
  return m_focusChangedSignal;
}

SignalProxy<void, Widget&, Key, Action, uint> Widget::keySignal()
{
  return m_keySignal;
}

SignalProxy<void, Widget&, uint32, uint> Widget::characterSignal()
{
  return m_characterSignal;
}

SignalProxy<void, Widget&, vec2, MouseButton, Action, uint> Widget::mouseButtonSignal()
{
  return m_mouseButtonSignal;
}

SignalProxy<void, Widget&, vec2> Widget::scrollSignal()
{
  return m_scrollSignal;
}

SignalProxy<void, Widget&, vec2> Widget::cursorPosSignal()
{
  return m_cursorPosSignal;
}

SignalProxy<void, Widget&> Widget::cursorEnteredSignal()
{
  return m_cursorEnteredSignal;
}

SignalProxy<void, Widget&> Widget::cursorLeftSignal()
{
  return m_cursorLeftSignal;
}

SignalProxy<void, Widget&, vec2> Widget::dragBegunSignal()
{
  return m_dragBegunSignal;
}

SignalProxy<void, Widget&, vec2> Widget::dragMovedSignal()
{
  return m_dragMovedSignal;
}

SignalProxy<void, Widget&, vec2> Widget::dragEndedSignal()
{
  return m_dragEndedSignal;
}

void Widget::draw() const
{
  for (const Widget* c : m_children)
  {
    if (c->isVisible())
      c->draw();
  }
}

void Widget::onChildAdded(Widget& child)
{
}

void Widget::onChildDesiredSizeChanged(Widget& child)
{
}

void Widget::onChildRemoved(Widget& child)
{
}

void Widget::onAreaChanged()
{
  m_areaChangedSignal(*this);
}

void Widget::onFocusChanged(bool activated)
{
  m_focusChangedSignal(*this, activated);
}

void Widget::onKey(Key key, Action action, uint mods)
{
  m_keySignal(*this, key, action, mods);
}

void Widget::onCharacter(uint32 character, uint mods)
{
  m_characterSignal(*this, character, mods);
}

void Widget::onMouseButton(vec2 point,
                           MouseButton button,
                           Action action,
                           uint mods)
{
  m_mouseButtonSignal(*this, point, button, action, mods);
}

void Widget::onScroll(vec2 offset)
{
  m_scrollSignal(*this, offset);
}

void Widget::onCursorPos(vec2 point)
{
  m_cursorPosSignal(*this, point);
}

void Widget::onCursorEntered()
{
  m_cursorEnteredSignal(*this);
}

void Widget::onCursorLeft()
{
  m_cursorLeftSignal(*this);
}

void Widget::onDragBegun(vec2 point)
{
  m_dragBegunSignal(*this, point);
}

void Widget::onDragMoved(vec2 point)
{
  m_dragMovedSignal(*this, point);
}

void Widget::onDragEnded(vec2 point)
{
  m_dragEndedSignal(*this, point);
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
