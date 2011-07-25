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

Layer::Layer(input::Context& initContext, UI::Drawer& initDrawer):
  context(initContext),
  drawer(initDrawer),
  dragging(false),
  activeWidget(NULL),
  draggedWidget(NULL),
  hoveredWidget(NULL)
{
}

Layer::~Layer(void)
{
  destroyRootWidgets();
}

void Layer::update(void)
{
}

void Layer::draw(void)
{
  drawer.begin();

  for (WidgetList::const_iterator i = roots.begin();  i != roots.end();  i++)
  {
    if ((*i)->isVisible())
      (*i)->draw();
  }

  drawer.end();
}

void Layer::addRootWidget(Widget& root)
{
  root.removeFromParent();
  roots.push_back(&root);
}

void Layer::destroyRootWidgets(void)
{
  while (!roots.empty())
    delete roots.back();
}

Widget* Layer::findWidgetByPoint(const vec2& point)
{
  for (WidgetList::reverse_iterator r = roots.rbegin();  r != roots.rend();  r++)
  {
    if ((*r)->isVisible())
    {
      Widget* widget = (*r)->findByPoint(point);
      if (widget)
	return widget;
    }
  }

  return NULL;
}

void Layer::cancelDragging(void)
{
  if (dragging && draggedWidget)
  {
    ivec2 cursorPosition = context.getCursorPosition();
    cursorPosition.y = context.getHeight() - cursorPosition.y;

    const vec2 scaledPosition((float) cursorPosition.x,
                              (float) cursorPosition.y);

    draggedWidget->dragEndedSignal.emit(*draggedWidget, scaledPosition);

    draggedWidget = NULL;
    dragging = false;
  }
}

void Layer::invalidate(void)
{
  context.getContext().refresh();
}

Drawer& Layer::getDrawer(void) const
{
  return drawer;
}

const WidgetList& Layer::getRootWidgets(void) const
{
  return roots;
}

Widget* Layer::getActiveWidget(void)
{
  return activeWidget;
}

Widget* Layer::getDraggedWidget(void)
{
  return draggedWidget;
}

Widget* Layer::getHoveredWidget(void)
{
  return hoveredWidget;
}

void Layer::setActiveWidget(Widget* widget)
{
  if (activeWidget == widget)
    return;

  if (widget)
  {
    if (&(widget->layer) != this)
      throw Exception("Cannot activate widget from other layer");

    if (!widget->isVisible() || !widget->isEnabled())
      return;
  }

  if (activeWidget)
    activeWidget->focusChangedSignal.emit(*activeWidget, false);

  activeWidget = widget;

  if (activeWidget)
    activeWidget->focusChangedSignal.emit(*activeWidget, true);
}

void Layer::updateHoveredWidget(void)
{
  ivec2 cursorPosition = context.getCursorPosition();
  cursorPosition.y = context.getHeight() - cursorPosition.y;

  const vec2 scaledPosition((float) cursorPosition.x,
                            (float) cursorPosition.y);

  Widget* newWidget = findWidgetByPoint(scaledPosition);

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

    ancestor->cursorLeftSignal.emit(*ancestor);
    ancestor = ancestor->getParent();
  }

  hoveredWidget = newWidget;

  while (newWidget)
  {
    // Notify each widget up to but not including the common ancestor

    if (newWidget == ancestor)
      break;

    newWidget->cursorEnteredSignal.emit(*newWidget);
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

  if (dragging)
  {
    if (draggedWidget)
    {
      if (draggedWidget == &widget || draggedWidget->isChildOf(widget))
	cancelDragging();
    }
  }
}

void Layer::onKeyPressed(input::Key key, bool pressed)
{
  if (activeWidget)
    activeWidget->keyPressedSignal.emit(*activeWidget, key, pressed);
}

void Layer::onCharInput(wchar_t character)
{
  if (activeWidget)
    activeWidget->charInputSignal.emit(*activeWidget, character);
}

void Layer::onCursorMoved(const ivec2& position)
{
  updateHoveredWidget();

  ivec2 cursorPosition = context.getCursorPosition();
  cursorPosition.y = context.getHeight() - cursorPosition.y;

  const vec2 scaledPosition((float) cursorPosition.x,
                            (float) cursorPosition.y);

  if (hoveredWidget)
    hoveredWidget->cursorMovedSignal.emit(*hoveredWidget, scaledPosition);

  if (draggedWidget)
  {
    if (dragging)
      draggedWidget->dragMovedSignal.emit(*draggedWidget, scaledPosition);
    else
    {
      // TODO: Add insensitivity radius.

      dragging = true;
      draggedWidget->dragBegunSignal.emit(*draggedWidget, scaledPosition);
    }
  }
}

void Layer::onButtonClicked(input::Button button, bool clicked)
{
  ivec2 cursorPosition = context.getCursorPosition();
  cursorPosition.y = context.getHeight() - cursorPosition.y;

  const vec2 scaledPosition((float) cursorPosition.x,
                            (float) cursorPosition.y);

  if (clicked)
  {
    Widget* clickedWidget = NULL;

    for (WidgetList::reverse_iterator r = roots.rbegin();  r != roots.rend();  r++)
    {
      if ((*r)->isVisible())
      {
	clickedWidget = (*r)->findByPoint(scaledPosition);
	if (clickedWidget)
	  break;
      }
    }

    while (clickedWidget && !clickedWidget->isEnabled())
      clickedWidget = clickedWidget->getParent();

    if (clickedWidget)
    {
      clickedWidget->activate();
      clickedWidget->buttonClickedSignal.emit(*clickedWidget,
                                              scaledPosition,
					      button,
					      clicked);

      // TODO: Allow dragging with any button.

      if (button == input::BUTTON_LEFT && clickedWidget->isDraggable())
	draggedWidget = clickedWidget;
    }
  }
  else
  {
    if (draggedWidget)
    {
      if (dragging)
      {
	draggedWidget->dragEndedSignal.emit(*draggedWidget, scaledPosition);
	dragging = false;
      }

      draggedWidget = NULL;
    }

    if (activeWidget && activeWidget->getGlobalArea().contains(scaledPosition))
    {
      activeWidget->buttonClickedSignal.emit(*activeWidget,
					     scaledPosition,
					     button,
					     clicked);
    }
  }
}

void Layer::onWheelTurned(int offset)
{
  if (hoveredWidget)
    hoveredWidget->wheelTurnedSignal.emit(*hoveredWidget, offset);
}

void Layer::onFocusChanged(bool activated)
{
  if (!activated)
    cancelDragging();
}

///////////////////////////////////////////////////////////////////////

LayerStack::LayerStack(input::Context& initContext):
  context(initContext)
{
}

void LayerStack::update(void) const
{
  for (LayerList::const_iterator l = layers.begin();  l != layers.end();  l++)
    (*l)->update();
}

void LayerStack::draw(void) const
{
  if (layers.empty())
    return;

  layers.back()->draw();
}

void LayerStack::push(Layer& layer)
{
  layers.push_back(&layer);
  context.setTarget(&layer);
}

void LayerStack::pop(void)
{
  if (!layers.empty())
    layers.pop_back();

  if (layers.empty())
    context.setTarget(NULL);
  else
    context.setTarget(layers.back());
}

void LayerStack::empty(void)
{
  while (!layers.empty())
    pop();
}

bool LayerStack::isEmpty(void) const
{
  return layers.empty();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
