///////////////////////////////////////////////////////////////////////
// Wendy user interface library
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

Layer::Layer(Window& initWindow, UI::Drawer& initDrawer):
  window(initWindow),
  drawer(initDrawer),
  dragging(false),
  activeWidget(nullptr),
  draggedWidget(nullptr),
  hoveredWidget(nullptr),
  captureWidget(nullptr),
  stack(nullptr)
{
  assert(&window);
  assert(&drawer);
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
  ProfileNodeCall call("UI::Layer::draw");

  drawer.begin();

  for (auto r : roots)
  {
    if (r->isVisible())
      r->draw();
  }

  drawer.end();
}

void Layer::addRootWidget(Widget& root)
{
  assert(&(root.layer) == this);

  root.removeFromParent();
  roots.push_back(&root);
}

void Layer::destroyRootWidgets()
{
  while (!roots.empty())
    delete roots.back();
}

Widget* Layer::findWidgetByPoint(const vec2& point)
{
  for (auto r = roots.rbegin();  r != roots.rend();  r++)
  {
    if ((*r)->isVisible())
    {
      Widget* widget = (*r)->findByPoint(point);
      if (widget)
        return widget;
    }
  }

  return nullptr;
}

void Layer::captureCursor()
{
  if (!activeWidget)
    return;

  releaseCursor();
  cancelDragging();

  captureWidget = activeWidget;
  hoveredWidget = activeWidget;
  window.captureCursor();
}

void Layer::releaseCursor()
{
  if (captureWidget)
  {
    captureWidget = nullptr;
    window.releaseCursor();
    updateHoveredWidget();
  }
}

void Layer::cancelDragging()
{
  if (dragging && draggedWidget)
  {
    vec2 cursorPosition = vec2(window.cursorPosition());
    cursorPosition.y = window.height() - cursorPosition.y;

    draggedWidget->dragEndedSignal(*draggedWidget, cursorPosition);

    draggedWidget = nullptr;
    dragging = false;
  }
}

void Layer::invalidate()
{
  window.invalidate();
}

bool Layer::isOpaque() const
{
  return true;
}

bool Layer::hasCapturedCursor() const
{
  return captureWidget != nullptr;
}

uint Layer::getWidth() const
{
  return window.width();
}

uint Layer::getHeight() const
{
  return window.height();
}

Drawer& Layer::getDrawer() const
{
  return drawer;
}

Window& Layer::getWindow() const
{
  return window;
}

const WidgetList& Layer::getRootWidgets() const
{
  return roots;
}

Widget* Layer::getActiveWidget()
{
  return activeWidget;
}

Widget* Layer::getDraggedWidget()
{
  return draggedWidget;
}

Widget* Layer::getHoveredWidget()
{
  return hoveredWidget;
}

void Layer::setActiveWidget(Widget* widget)
{
  if (activeWidget == widget)
    return;

  if (widget)
  {
    assert(&(widget->layer) == this);

    if (!widget->isVisible() || !widget->isEnabled())
      return;
  }

  if (captureWidget)
    releaseCursor();

  if (activeWidget)
    activeWidget->focusChangedSignal(*activeWidget, false);

  activeWidget = widget;

  if (activeWidget)
    activeWidget->focusChangedSignal(*activeWidget, true);

  invalidate();
}

LayerStack* Layer::getStack() const
{
  return stack;
}

SignalProxy1<void, Layer&> Layer::getSizeChangedSignal()
{
  return sizeChangedSignal;
}

void Layer::updateHoveredWidget()
{
  if (captureWidget)
    return;

  vec2 cursorPosition = vec2(window.cursorPosition());
  cursorPosition.y = window.height() - cursorPosition.y;

  Widget* newWidget = findWidgetByPoint(cursorPosition);

  if (hoveredWidget == newWidget)
    return;

  Widget* ancestor = hoveredWidget;

  while (ancestor)
  {
    // Find the common ancestor (or NULL) and notify each non-common ancestor

    if (newWidget == ancestor)
      break;

    if (newWidget && newWidget->isChildOf(*ancestor))
      break;

    ancestor->cursorLeftSignal(*ancestor);
    ancestor = ancestor->getParent();
  }

  hoveredWidget = newWidget;

  while (newWidget)
  {
    // Notify each widget up to but not including the common ancestor

    if (newWidget == ancestor)
      break;

    newWidget->cursorEnteredSignal(*newWidget);
    newWidget = newWidget->getParent();
  }
}

void Layer::removedWidget(Widget& widget)
{
  if (activeWidget)
  {
    if (activeWidget == &widget || activeWidget->isChildOf(widget))
      setActiveWidget(widget.parent);
  }

  if (hoveredWidget)
  {
    if (hoveredWidget == &widget || hoveredWidget->isChildOf(widget))
      updateHoveredWidget();
  }

  if (captureWidget)
  {
    if (captureWidget == &widget || captureWidget->isChildOf(widget))
      releaseCursor();
  }

  if (dragging)
  {
    if (draggedWidget)
    {
      if (draggedWidget == &widget || draggedWidget->isChildOf(widget))
        cancelDragging();
    }
  }
}

void Layer::onWindowSize(uint width, uint height)
{
  sizeChangedSignal(*this);
}

void Layer::onKey(Key key, Action action, uint mods)
{
  if (activeWidget)
    activeWidget->keyPressedSignal(*activeWidget, key, action);
}

void Layer::onCharacter(uint32 character, uint mods)
{
  if (activeWidget)
    activeWidget->charInputSignal(*activeWidget, character);
}

void Layer::onCursorPos(vec2 position)
{
  updateHoveredWidget();

  position.y = window.height() - position.y;

  if (hoveredWidget)
    hoveredWidget->cursorMovedSignal(*hoveredWidget, position);

  if (draggedWidget)
  {
    if (dragging)
      draggedWidget->dragMovedSignal(*draggedWidget, position);
    else
    {
      // TODO: Add insensitivity radius.

      dragging = true;
      draggedWidget->dragBegunSignal(*draggedWidget, position);
    }
  }
}

void Layer::onMouseButton(MouseButton button, Action action, uint mods)
{
  vec2 cursorPosition = vec2(window.cursorPosition());
  cursorPosition.y = window.height() - cursorPosition.y;

  if (action == PRESSED)
  {
    Widget* clickedWidget = nullptr;

    if (captureWidget)
      clickedWidget = captureWidget;
    else
    {
      for (auto w = roots.rbegin();  w != roots.rend();  w++)
      {
        if ((*w)->isVisible())
        {
          clickedWidget = (*w)->findByPoint(cursorPosition);
          if (clickedWidget)
            break;
        }
      }
    }

    while (clickedWidget && !clickedWidget->isEnabled())
      clickedWidget = clickedWidget->getParent();

    if (clickedWidget)
    {
      clickedWidget->activate();
      clickedWidget->buttonClickedSignal(*clickedWidget,
                                         cursorPosition,
                                         button,
                                         action);

      if (!captureWidget && clickedWidget->isDraggable())
        draggedWidget = clickedWidget;
    }
  }
  else if (action == RELEASED)
  {
    if (draggedWidget)
    {
      if (dragging)
      {
        draggedWidget->dragEndedSignal(*draggedWidget, cursorPosition);
        dragging = false;
      }

      draggedWidget = nullptr;
    }

    if (activeWidget)
    {
      if (captureWidget || activeWidget->getGlobalArea().contains(cursorPosition))
      {
        activeWidget->buttonClickedSignal(*activeWidget,
                                          cursorPosition,
                                          button,
                                          action);
      }
    }
  }
}

void Layer::onScroll(vec2 offset)
{
  if (hoveredWidget)
    hoveredWidget->scrolledSignal(*hoveredWidget, offset);
}

void Layer::onFocus(bool activated)
{
  if (!activated)
  {
    cancelDragging();
    releaseCursor();
  }
}

///////////////////////////////////////////////////////////////////////

LayerStack::LayerStack(Window& initWindow):
  window(initWindow)
{
}

void LayerStack::update() const
{
  for (auto l : layers)
    l->update();
}

void LayerStack::draw() const
{
  size_t count = 0;

  while (count < layers.size())
  {
    count++;

    if (layers[layers.size() - count]->isOpaque())
      break;
  }

  while (count)
  {
    layers[layers.size() - count]->draw();

    count--;
  }
}

void LayerStack::push(Layer& layer)
{
  assert(layer.stack == nullptr);
  assert(&layer.window == &window);

  layers.push_back(&layer);
  layer.stack = this;
  window.setTarget(&layer);
}

void LayerStack::pop()
{
  if (!layers.empty())
  {
    window.setTarget(nullptr);
    layers.back()->stack = nullptr;
    layers.pop_back();
  }

  if (!layers.empty())
    window.setTarget(layers.back());
}

void LayerStack::empty()
{
  while (!layers.empty())
    pop();
}

bool LayerStack::isEmpty() const
{
  return layers.empty();
}

Layer* LayerStack::getTop() const
{
  if (layers.empty())
    return nullptr;

  return layers.back();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
