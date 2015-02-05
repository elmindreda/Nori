///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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
#include <wendy/Time.hpp>
#include <wendy/Profile.hpp>

#include <wendy/Drawer.hpp>
#include <wendy/Layer.hpp>
#include <wendy/Widget.hpp>

namespace wendy
{

Layer::Layer(Drawer& drawer):
  m_window(drawer.context().window()),
  m_drawer(drawer),
  m_dragging(false),
  m_activeWidget(nullptr),
  m_draggedWidget(nullptr),
  m_hoveredWidget(nullptr),
  m_captureWidget(nullptr),
  m_stack(nullptr)
{
  assert(&m_window);
  assert(&m_drawer);
}

Layer::~Layer()
{
  destroyRootWidgets();
}

void Layer::update()
{
}

void Layer::draw()
{
  ProfileNodeCall call("Layer::draw");

  m_drawer.begin();

  for (Widget* r : m_roots)
  {
    if (r->isVisible())
      r->draw();
  }

  m_drawer.end();
}

void Layer::destroyRootWidgets()
{
  while (!m_roots.empty())
    delete m_roots.back();
}

Widget* Layer::findWidgetByPoint(vec2 point)
{
  for (auto r = m_roots.rbegin();  r != m_roots.rend();  r++)
  {
    if ((*r)->isVisible())
    {
      if (Widget* widget = (*r)->findByPoint(point))
        return widget;
    }
  }

  return nullptr;
}

void Layer::captureCursor()
{
  if (!m_activeWidget)
    return;

  releaseCursor();
  cancelDragging();

  m_captureWidget = m_activeWidget;
  m_hoveredWidget = m_activeWidget;
  m_window.captureCursor();
}

void Layer::releaseCursor()
{
  if (m_captureWidget)
  {
    m_captureWidget = nullptr;
    m_window.releaseCursor();
    updateHoveredWidget();
  }
}

void Layer::cancelDragging()
{
  if (m_dragging)
  {
    m_draggedWidget->onDragEnded(cursorPoint(), m_dragButton);
    m_dragging = false;
  }

  m_draggedWidget = nullptr;
}

void Layer::activatePrevWidget()
{
  activateWidget(-1);
}

void Layer::activateNextWidget()
{
  activateWidget(1);
}

void Layer::invalidate()
{
  m_window.invalidate();
}

bool Layer::isOpaque() const
{
  return true;
}

bool Layer::hasCapturedCursor() const
{
  return m_captureWidget != nullptr;
}

vec2 Layer::cursorPoint() const
{
  const vec2 cursorPosition(m_window.cursorPosition());
  return vec2(cursorPosition.x, m_window.height() - cursorPosition.y);
}

void Layer::setActiveWidget(Widget* widget)
{
  if (widget)
  {
    assert(&(widget->m_layer) == this);

    while (widget)
    {
      if (widget->isVisible() && widget->isEnabled())
        break;

      widget = widget->parent();
    }
  }

  if (m_activeWidget == widget)
    return;

  if (m_captureWidget)
    releaseCursor();

  if (m_activeWidget)
    m_activeWidget->onFocusChanged(false);

  m_activeWidget = widget;

  if (m_activeWidget)
    m_activeWidget->onFocusChanged(true);

  invalidate();
}

void Layer::onWindowSize(uint width, uint height)
{
  m_sizeChanged(*this);
}

void Layer::onKey(Key key, Action action, uint mods)
{
  if (key == KEY_TAB && action == PRESSED)
  {
    if (mods & MOD_SHIFT)
      activatePrevWidget();
    else
      activateNextWidget();
  }
  else
  {
    if (m_activeWidget)
      m_activeWidget->onKey(key, action, mods);
  }
}

void Layer::onCharacter(uint32 codepoint)
{
  if (m_activeWidget)
    m_activeWidget->onCharacter(codepoint);
}

void Layer::onCursorPos(vec2 point)
{
  updateHoveredWidget();

  point.y = m_window.height() - point.y;

  if (m_hoveredWidget)
    m_hoveredWidget->onCursorPos(point);

  if (m_draggedWidget)
  {
    if (m_dragging)
      m_draggedWidget->onDragMoved(point, m_dragButton);
    else
    {
      // TODO: Add insensitivity radius.

      m_dragging = true;
      m_draggedWidget->onDragBegun(point, m_dragButton);
    }
  }
}

void Layer::onMouseButton(MouseButton button, Action action, uint mods)
{
  const vec2 point(cursorPoint());

  if (action == PRESSED)
  {
    Widget* clickedWidget = nullptr;

    if (m_captureWidget)
      clickedWidget = m_captureWidget;
    else
      clickedWidget = findWidgetByPoint(point);

    if (clickedWidget)
    {
      if (!clickedWidget->isEnabled())
        return;

      if (clickedWidget->isFocusable())
      {
        clickedWidget->activate();

        if (!m_captureWidget && clickedWidget->isDraggable())
        {
          if (m_draggedWidget != clickedWidget)
            m_dragButton = button;

          m_draggedWidget = clickedWidget;
        }
      }

      clickedWidget->onMouseButton(point, button, action, mods);
    }
  }
  else if (action == RELEASED)
  {
    if (m_draggedWidget)
    {
      if (m_dragging)
      {
        m_draggedWidget->onDragEnded(point, m_dragButton);
        m_dragging = false;
      }

      m_draggedWidget = nullptr;
    }

    if (m_activeWidget)
    {
      if (m_captureWidget || m_activeWidget->globalArea().contains(point))
        m_activeWidget->onMouseButton(point, button, action, mods);
    }
  }
}

void Layer::onScroll(vec2 offset)
{
  if (m_hoveredWidget)
    m_hoveredWidget->onScroll(offset);
}

void Layer::onFocus(bool activated)
{
  if (!activated)
  {
    cancelDragging();
    releaseCursor();
  }
}

void Layer::updateHoveredWidget()
{
  if (m_captureWidget)
    return;

  Widget* newWidget = findWidgetByPoint(cursorPoint());
  if (newWidget == m_hoveredWidget)
    return;

  Widget* ancestor = m_hoveredWidget;
  while (ancestor)
  {
    // Find the common ancestor (or NULL) and notify each non-common ancestor

    if (newWidget == ancestor)
      break;

    if (newWidget && newWidget->isChildOf(*ancestor))
      break;

    ancestor->onCursorLeft();
    ancestor = ancestor->parent();
  }

  m_hoveredWidget = newWidget;

  while (newWidget)
  {
    // Notify each widget up to but not including the common ancestor

    if (newWidget == ancestor)
      break;

    newWidget->onCursorEntered();
    newWidget = newWidget->parent();
  }
}

void Layer::activateWidget(int offset)
{
  std::vector<Widget*> focusable;
  focusableWidgets(focusable, m_roots);

  if (!focusable.empty())
  {
    auto i = std::find(focusable.begin(), focusable.end(), m_activeWidget);
    const size_t count = focusable.size();
    const size_t index = i - focusable.begin();

    focusable[(index + count + offset) % count]->activate();
  }
}

void Layer::removeWidget(Widget& widget)
{
  if (m_activeWidget)
  {
    if (m_activeWidget == &widget || m_activeWidget->isChildOf(widget))
      setActiveWidget(widget.parent());
  }

  if (m_hoveredWidget)
  {
    if (m_hoveredWidget == &widget || m_hoveredWidget->isChildOf(widget))
      m_hoveredWidget = nullptr;
  }

  if (m_captureWidget)
  {
    if (m_captureWidget == &widget || m_captureWidget->isChildOf(widget))
      releaseCursor();
  }

  if (m_draggedWidget)
  {
    if (m_draggedWidget == &widget || m_draggedWidget->isChildOf(widget))
      cancelDragging();
  }
}

void Layer::focusableWidgets(std::vector<Widget*>& target,
                             const std::vector<Widget*>& source) const
{
  for (Widget* widget : source)
  {
    if (widget->isVisible() && widget->isEnabled())
    {
      if (widget->isFocusable())
        target.push_back(widget);

      focusableWidgets(target, widget->m_children);
    }
  }
}

LayerStack::LayerStack(Window& window):
  m_window(window)
{
}

void LayerStack::update() const
{
  for (Layer* l : m_layers)
    l->update();
}

void LayerStack::draw() const
{
  size_t count = 0;

  while (count < m_layers.size())
  {
    count++;

    if (m_layers[m_layers.size() - count]->isOpaque())
      break;
  }

  while (count)
  {
    m_layers[m_layers.size() - count]->draw();
    count--;
  }
}

void LayerStack::push(Layer& layer)
{
  assert(layer.m_stack == nullptr);
  assert(&layer.m_window == &m_window);

  m_layers.push_back(&layer);
  layer.m_stack = this;
  m_window.setTarget(&layer);
}

void LayerStack::pop()
{
  if (!m_layers.empty())
  {
    m_window.setTarget(nullptr);
    m_layers.back()->m_stack = nullptr;
    m_layers.pop_back();
  }

  if (!m_layers.empty())
    m_window.setTarget(m_layers.back());
}

void LayerStack::empty()
{
  while (!m_layers.empty())
    pop();
}

bool LayerStack::isEmpty() const
{
  return m_layers.empty();
}

Layer* LayerStack::top() const
{
  if (m_layers.empty())
    return nullptr;

  return m_layers.back();
}

} /*namespace wendy*/

