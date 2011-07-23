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
#include <wendy/UIModule.h>
#include <wendy/UIWidget.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Module::Module(input::Context& initContext, UI::Drawer& initDrawer):
  context(initContext),
  drawer(initDrawer),
  dragging(false),
  activeWidget(NULL),
  draggedWidget(NULL),
  hoveredWidget(NULL)
{
}

Module::~Module(void)
{
  destroyRootWidgets();
}

void Module::draw(void)
{
  drawer.begin();

  for (WidgetList::const_iterator i = roots.begin();  i != roots.end();  i++)
  {
    if ((*i)->isVisible())
      (*i)->draw();
  }

  drawer.end();
}

void Module::addRootWidget(Widget& root)
{
  root.removeFromParent();
  roots.push_back(&root);
}

void Module::destroyRootWidgets(void)
{
  while (!roots.empty())
    delete roots.back();
}

Widget* Module::findWidgetByPoint(const vec2& point)
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

void Module::cancelDragging(void)
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

void Module::invalidate(void)
{
  context.getContext().refresh();
}

Drawer& Module::getDrawer(void) const
{
  return drawer;
}

const WidgetList& Module::getRootWidgets(void) const
{
  return roots;
}

Widget* Module::getActiveWidget(void)
{
  return activeWidget;
}

Widget* Module::getDraggedWidget(void)
{
  return draggedWidget;
}

Widget* Module::getHoveredWidget(void)
{
  return hoveredWidget;
}

void Module::setActiveWidget(Widget* widget)
{
  if (activeWidget == widget)
    return;

  if (widget)
  {
    if (&(widget->module) != this)
      throw Exception("Cannot activate widget from other module");

    if (!widget->isVisible() || !widget->isEnabled())
      return;
  }

  if (activeWidget)
    activeWidget->focusChangedSignal.emit(*activeWidget, false);

  activeWidget = widget;

  if (activeWidget)
    activeWidget->focusChangedSignal.emit(*activeWidget, true);
}

void Module::updateHoveredWidget(void)
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

    if (newWidget == ancestor || newWidget->isChildOf(*ancestor))
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

void Module::removedWidget(Widget& widget)
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

void Module::onKeyPressed(input::Key key, bool pressed)
{
  if (activeWidget)
    activeWidget->keyPressedSignal.emit(*activeWidget, key, pressed);
}

void Module::onCharInput(wchar_t character)
{
  if (activeWidget)
    activeWidget->charInputSignal.emit(*activeWidget, character);
}

void Module::onCursorMoved(const ivec2& position)
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

void Module::onButtonClicked(input::Button button, bool clicked)
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

void Module::onWheelTurned(int offset)
{
  if (hoveredWidget)
    hoveredWidget->wheelTurnedSignal.emit(*hoveredWidget, offset);
}

void Module::onFocusChanged(bool activated)
{
  if (!activated)
    cancelDragging();
}

///////////////////////////////////////////////////////////////////////

ModuleStack::ModuleStack(input::Context& initContext):
  context(initContext)
{
}

void ModuleStack::draw(void) const
{
  if (modules.empty())
    return;

  modules.back()->draw();
}

void ModuleStack::push(Module& module)
{
  modules.push_back(&module);
  context.setTarget(&module);
}

void ModuleStack::pop(void)
{
  if (!modules.empty())
    modules.pop_back();

  if (modules.empty())
    context.setTarget(NULL);
  else
    context.setTarget(modules.back());
}

void ModuleStack::empty(void)
{
  while (!modules.empty())
    pop();
}

bool ModuleStack::isEmpty(void) const
{
  return modules.empty();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
