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
#include <moira/Log.h>
#include <moira/Signal.h>
#include <moira/Node.h>
#include <moira/Color.h>
#include <moira/Vector.h>
#include <moira/Rectangle.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLShader.h>
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
  visible(true),
  underCursor(false)
{
  static bool initialized = false;

  if (!initialized)
  {
    Context::get()->getKeyPressSignal().connect(&Widget::onKeyPress);
    Context::get()->getButtonClickSignal().connect(&Widget::onButtonClick);
    Context::get()->getCursorMoveSignal().connect(&Widget::onCursorMove);

    initialized = true;
  }

  area.set(0.f, 0.f, 1.f, 1.f);

  roots.push_front(this);
}

Widget::~Widget(void)
{
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

  destroySignal.emit(*this);
}

Widget* Widget::findByPoint(const Vector2& point)
{
  if (!area.contains(point))
    return NULL;

  const Vector2 localPoint = point - area.position;

  for (Widget* child = getFirstChild();  child;  child = child->getNextSibling())
  {
    if (child->isVisible())
      if (Widget* result = child->findByPoint(localPoint))
	return result;
  }

  return this;
}

void Widget::removeFromParent(void)
{
  Node<Widget>::removeFromParent();

  roots.push_front(this);
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
  if (activeWidget)
    activeWidget->changeFocusSignal.emit(*activeWidget, false);

  changeFocusSignal.emit(*this, true);
  activeWidget = this;
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

Widget* Widget::getActive(void)
{
  return activeWidget;
}

void Widget::renderRoots(void)
{
  for (WidgetList::iterator i = roots.begin();  i != roots.end();  i++)
  {
    if ((*i)->isVisible())
      (*i)->render();
  }
}

void Widget::render(void) const
{
  for (const Widget* child = getFirstChild();  child;  child = child->getNextSibling())
  {
    if (child->isVisible())
      child->render();
  }
}

void Widget::addedToParent(Widget& parent)
{
  roots.remove(this);
}

void Widget::onKeyPress(Key key, bool pressed)
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

void Widget::onCursorMove(const Vector2& position)
{
}

void Widget::onButtonClick(unsigned int button, bool clicked)
{
  Context* context = Context::get();

  Vector2 cursorPosition = context->getCursorPosition();
  cursorPosition.y = context->getHeight() - cursorPosition.y;

  if (clicked)
  {
    for (WidgetList::iterator i = roots.begin();  i != roots.end();  i++)
    {
      Widget* clickedWidget = (*i)->findByPoint(cursorPosition);

      while (clickedWidget && !clickedWidget->isEnabled())
	clickedWidget = clickedWidget->getParent();

      if (clickedWidget)
      {
	cursorPosition -= clickedWidget->getGlobalArea().position;

	clickedWidget->activate();
	clickedWidget->buttonClickSignal.emit(*clickedWidget, cursorPosition, button, clicked);
      }
    }
  }
  else
  {
    if (activeWidget)
    {
      cursorPosition -= activeWidget->getGlobalArea().position;
      activeWidget->buttonClickSignal.emit(*activeWidget, cursorPosition, button, clicked);
    }
  }
}

Widget::WidgetList Widget::roots;

Widget* Widget::activeWidget = NULL;

///////////////////////////////////////////////////////////////////////

Button::Button(const String& name, const String& initTitle):
  Widget(name),
  title(initTitle)
{
  getButtonClickSignal().connect(*this, &Button::onButtonClick);
  getKeyPressSignal().connect(*this, &Button::onKeyPress);
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
  const Rectangle& area = getGlobalArea();

  ShaderPass pass;
  pass.setDepthTesting(false);

  if (isActive())
    pass.setDefaultColor(ColorRGBA(0.7f, 0.7f, 0.7f, 1.f));
  else
    pass.setDefaultColor(ColorRGBA(0.5f, 0.5f, 0.5f, 1.f));
  pass.apply();

  glRectf(area.position.x, area.position.y, area.position.x + area.size.x, area.position.y + area.size.y);

  pass.setDefaultColor(ColorRGBA::BLACK);
  pass.setPolygonMode(GL_LINE);
  pass.apply();

  glRectf(area.position.x, area.position.y, area.position.x + area.size.x, area.position.y + area.size.y);

  Widget::render();
}

void Button::onButtonClick(Widget& widget, const Vector2& position, unsigned int button, bool clicked)
{
  if (button == 0 && clicked == false && getArea().contains(position))
    pushedSignal.emit(*this);
}

void Button::onKeyPress(Widget& widget, Key key, bool pressed)
{
  if (key == Key::ENTER && pressed == true)
    pushedSignal.emit(*this);
}

///////////////////////////////////////////////////////////////////////

Slider::Slider(const String& name):
  Widget(name),
  minValue(0.f),
  maxValue(1.f),
  value(0.f),
  orientation(VERTICAL)
{
  getKeyPressSignal().connect(*this, &Slider::onKeyPress);
  getButtonClickSignal().connect(*this, &Slider::onButtonClick);
}

float Slider::getMinValue(void) const
{
  return minValue;
}

float Slider::getMaxValue(void) const
{
  return maxValue;
}

void Slider::setValueRange(float newMinValue, float newMaxValue)
{
  minValue = newMinValue;
  maxValue = newMaxValue;
}

float Slider::getValue(void) const
{
  return value;
}

void Slider::setValue(float newValue)
{
  changeValueSignal.emit(*this, newValue);
  value = newValue;
}

Slider::Orientation Slider::getOrientation(void) const
{
  return orientation;
}

void Slider::setOrientation(Orientation newOrientation)
{
  orientation = newOrientation;
}

SignalProxy2<void, Slider&, float> Slider::getChangeValueSignal(void)
{
  return changeValueSignal;
}

void Slider::render(void) const
{
  const Rectangle& area = getGlobalArea();

  ShaderPass pass;
  pass.setCullMode(CULL_NONE);
  pass.setDepthTesting(false);

  if (isActive())
    pass.setDefaultColor(ColorRGBA(0.7f, 0.7f, 0.7f, 1.f));
  else
    pass.setDefaultColor(ColorRGBA(0.5f, 0.5f, 0.5f, 1.f));
  pass.apply();

  glRectf(area.position.x, area.position.y, area.position.x + area.size.x, area.position.y + area.size.y);

  pass.setDefaultColor(ColorRGBA::BLACK);
  pass.setPolygonMode(GL_LINE);
  pass.apply();

  glRectf(area.position.x, area.position.y, area.position.x + area.size.x, area.position.y + area.size.y);

  const float position = (value - minValue) / (maxValue - minValue);

  if (orientation == HORIZONTAL)
  {
    glRectf(area.position.x + position * area.size.x - 5.f,
            area.position.y,
	    area.position.x + position * area.size.x + 5.f,
	    area.position.y + area.size.y);
  }
  else
  {
    glRectf(area.position.x,
            area.position.y + position * area.size.y - 5.f,
	    area.position.x + area.size.x,
	    area.position.y + position * area.size.y + 5.f);
  }

  Widget::render();
}

void Slider::onButtonClick(Widget& widget,
	                   const Vector2& position,
	                   unsigned int button,
		           bool clicked)
{
  if (clicked)
  {
    if (orientation == HORIZONTAL)
      setValue(minValue + (maxValue - minValue) * (position.x / getArea().size.x));
    else
      setValue(minValue + (maxValue - minValue) * (position.y / getArea().size.y));
  }
}

void Slider::onKeyPress(Widget& widget, Key key, bool pressed)
{
  if (pressed)
  {
    switch (key)
    {
      case Key::UP:
	setValue(value + 1.f);
	break;
      case Key::DOWN:
	setValue(value - 1.f);
	break;
    }
  }
}

///////////////////////////////////////////////////////////////////////

Window::Window(const String& name, const String& initTitle):
  Widget(name),
  title(initTitle)
{
}

void Window::render(void) const
{
  Context* context = Context::get();
  Canvas::getCurrent()->begin2D(Vector2(context->getWidth(), context->getHeight()));
  ShaderPass pass;
  pass.setDepthTesting(false);
  pass.setDefaultColor(ColorRGBA::WHITE);
  pass.apply();
  const Rectangle& area = getGlobalArea();
  glRectf(area.position.x, area.position.y, area.position.x + area.size.x, area.position.y + area.size.y);
  Widget::render();
  Canvas::getCurrent()->end();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
