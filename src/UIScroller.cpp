//////////////////////////////////////////////////////////////////////
// Wendy user interface library
// Copyright (c) 2007 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/UIScroller.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Scroller::Scroller(Layer& layer, Orientation initOrientation):
  Widget(layer),
  minValue(0.f),
  maxValue(1.f),
  value(0.f),
  percentage(0.5f),
  reference(0.f),
  orientation(initOrientation)
{
  const float em = layer.getDrawer().getCurrentEM();

  if (orientation == HORIZONTAL)
    setSize(vec2(em * 10.f, em * 1.5f));
  else
    setSize(vec2(em * 1.5f, em * 10.f));

  getKeyPressedSignal().connect(*this, &Scroller::onKeyPressed);
  getButtonClickedSignal().connect(*this, &Scroller::onButtonClicked);
  getWheelTurnedSignal().connect(*this, &Scroller::onWheelTurned);
  getDragBegunSignal().connect(*this, &Scroller::onDragBegun);
  getDragMovedSignal().connect(*this, &Scroller::onDragMoved);

  setDraggable(true);
}

float Scroller::getMinValue() const
{
  return minValue;
}

float Scroller::getMaxValue() const
{
  return maxValue;
}

void Scroller::setValueRange(float newMinValue, float newMaxValue)
{
  minValue = newMinValue;
  maxValue = newMaxValue;

  if (value < minValue)
    setValue(minValue, true);
  else if (value > maxValue)
    setValue(maxValue, true);
  else
    invalidate();
}

float Scroller::getValue() const
{
  return value;
}

void Scroller::setValue(float newValue)
{
  setValue(newValue, false);
}

float Scroller::getPercentage() const
{
  return percentage;
}

void Scroller::setPercentage(float newPercentage)
{
  percentage = clamp(newPercentage, 0.f, 1.f);
  invalidate();
}

SignalProxy1<void, Scroller&> Scroller::getValueChangedSignal()
{
  return valueChangedSignal;
}

void Scroller::draw() const
{
  const Rect& area = getGlobalArea();

  Drawer& drawer = getLayer().getDrawer();
  if (drawer.pushClipArea(area))
  {
    drawer.drawWell(area, getState());

    if (minValue != maxValue)
    {
      const float size = getHandleSize();
      const float offset = getHandleOffset();

      Rect handleArea;

      if (orientation == HORIZONTAL)
      {
        handleArea.set(area.position.x + offset,
                       area.position.y,
                       size,
                       area.size.y);
      }
      else
      {
        handleArea.set(area.position.x,
                       area.position.y + area.size.y - offset - size,
                       area.size.x,
                       size);
      }

      drawer.drawHandle(handleArea, getState());
    }

    Widget::draw();

    drawer.popClipArea();
  }
}

void Scroller::onButtonClicked(Widget& widget,
                               const vec2& point,
                               input::Button button,
                               bool clicked)
{
  if (!clicked)
    return;

  if (minValue == maxValue)
    return;

  vec2 localPoint = transformToLocal(point);

  const float size = getHandleSize();
  const float offset = getHandleOffset();

  if (orientation == HORIZONTAL)
  {
    if (localPoint.x < offset)
      setValue(value - getValueStep(), true);
    else if (localPoint.x >= offset + size)
      setValue(value + getValueStep(), true);
  }
  else
  {
    if (localPoint.y > getArea().size.y - offset)
      setValue(value - getValueStep(), true);
    else if (localPoint.y <= getArea().size.y - offset - size)
      setValue(value + getValueStep(), true);
  }
}

void Scroller::onKeyPressed(Widget& widget, input::Key key, bool pressed)
{
  if (minValue == maxValue)
    return;

  if (pressed)
  {
    switch (key)
    {
      case input::KEY_DOWN:
      case input::KEY_RIGHT:
      {
        setValue(value + getValueStep(), true);
        break;
      }

      case input::KEY_UP:
      case input::KEY_LEFT:
      {
        setValue(value - getValueStep(), true);
        break;
      }

      case input::KEY_HOME:
      {
        setValue(minValue, true);
        break;
      }

      case input::KEY_END:
      {
        setValue(maxValue, true);
        break;
      }
    }
  }
}

void Scroller::onWheelTurned(Widget& widget, int offset)
{
  if (minValue == maxValue)
    return;

  setValue(value + offset * getValueStep(), true);
}

void Scroller::onDragBegun(Widget& widget, const vec2& point)
{
  if (minValue == maxValue)
    return;

  vec2 localPoint = transformToLocal(point);

  const Rect& area = getArea();

  const float size = getHandleSize();
  const float offset = getHandleOffset();

  if (orientation == HORIZONTAL)
  {
    if (localPoint.x >= offset && localPoint.x < offset + size)
      reference = localPoint.x - offset;
    else
      cancelDragging();
  }
  else
  {
    if (localPoint.y <= area.size.y - offset &&
        localPoint.y > area.size.y - offset - size)
      reference = area.size.y - localPoint.y - offset;
    else
      cancelDragging();
  }
}

void Scroller::onDragMoved(Widget& widget, const vec2& point)
{
  if (minValue == maxValue)
    return;

  vec2 localPoint = transformToLocal(point);

  const Rect& area = getArea();

  const float size = getHandleSize();

  float scale;

  if (orientation == HORIZONTAL)
    scale = (localPoint.x - reference) / (area.size.x - size);
  else
    scale = (area.size.y - localPoint.y - reference) / (area.size.y - size);

  setValue(minValue + (maxValue - minValue) * scale, true);
}

void Scroller::setValue(float newValue, bool notify)
{
  newValue = clamp(newValue, minValue, maxValue);
  if (newValue == value)
    return;

  value = newValue;

  if (notify)
    valueChangedSignal.emit(*this);

  invalidate();
}

float Scroller::getHandleSize() const
{
  const float em = getLayer().getDrawer().getCurrentEM();

  if (orientation == HORIZONTAL)
    return max(getArea().size.x * percentage, em);
  else
    return max(getArea().size.y * percentage, em);
}

float Scroller::getHandleOffset() const
{
  const float scale = (value - minValue) / (maxValue - minValue);

  if (orientation == HORIZONTAL)
    return (getArea().size.x - getHandleSize()) * scale;
  else
    return (getArea().size.y - getHandleSize()) * scale;
}

float Scroller::getValueStep() const
{
  return (maxValue - minValue) * percentage / (1.f - percentage);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
