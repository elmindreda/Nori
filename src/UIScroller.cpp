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

#include <wendy/Config.hpp>

#include <wendy/UIDrawer.hpp>
#include <wendy/UILayer.hpp>
#include <wendy/UIWidget.hpp>
#include <wendy/UIScroller.hpp>

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

  getKeyPressedSignal().connect(*this, &Scroller::onKey);
  getButtonClickedSignal().connect(*this, &Scroller::onMouseButton);
  getScrolledSignal().connect(*this, &Scroller::onScroll);
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

void Scroller::onMouseButton(Widget& widget,
                             vec2 point,
                             MouseButton button,
                             Action action)
{
  if (action != PRESSED)
    return;

  const vec2 local = transformToLocal(point);
  const float size = getHandleSize();
  const float offset = getHandleOffset();

  if (orientation == HORIZONTAL)
  {
    if (local.x < offset)
      setValue(value - getValueStep(), true);
    else if (local.x >= offset + size)
      setValue(value + getValueStep(), true);
  }
  else
  {
    if (local.y > getHeight() - offset)
      setValue(value - getValueStep(), true);
    else if (local.y <= getHeight() - offset - size)
      setValue(value + getValueStep(), true);
  }
}

void Scroller::onKey(Widget& widget, Key key, Action action)
{
  if (action != PRESSED)
  {
    switch (key)
    {
      case KEY_DOWN:
      case KEY_RIGHT:
      {
        setValue(value + getValueStep(), true);
        break;
      }

      case KEY_UP:
      case KEY_LEFT:
      {
        setValue(value - getValueStep(), true);
        break;
      }

      case KEY_HOME:
      {
        setValue(minValue, true);
        break;
      }

      case KEY_END:
      {
        setValue(maxValue, true);
        break;
      }

      default:
        break;
    }
  }
}

void Scroller::onScroll(Widget& widget, vec2 offset)
{
  if (orientation == HORIZONTAL)
    setValue(value + float(offset.x) * getValueStep(), true);
  else
    setValue(value + float(offset.y) * getValueStep(), true);
}

void Scroller::onDragBegun(Widget& widget, vec2 point)
{
  const vec2 local = transformToLocal(point);
  const float size = getHandleSize();
  const float offset = getHandleOffset();

  if (orientation == HORIZONTAL)
  {
    if (local.x >= offset && local.x < offset + size)
      reference = local.x - offset;
    else
      cancelDragging();
  }
  else
  {
    if (local.y <= getHeight() - offset && local.y > getHeight() - offset - size)
      reference = getHeight() - local.y - offset;
    else
      cancelDragging();
  }
}

void Scroller::onDragMoved(Widget& widget, vec2 point)
{
  const vec2 local = transformToLocal(point);
  const float size = getHandleSize();

  float scale;

  if (orientation == HORIZONTAL)
    scale = (local.x - reference) / (getWidth() - size);
  else
    scale = (getHeight() - local.y - reference) / (getHeight() - size);

  setValue(minValue + (maxValue - minValue) * scale, true);
}

void Scroller::setValue(float newValue, bool notify)
{
  newValue = clamp(newValue, minValue, maxValue);
  if (newValue == value)
    return;

  value = newValue;

  if (notify)
    valueChangedSignal(*this);

  invalidate();
}

float Scroller::getHandleSize() const
{
  const float em = getLayer().getDrawer().getCurrentEM();

  if (orientation == HORIZONTAL)
    return max(getWidth() * percentage, em);
  else
    return max(getHeight() * percentage, em);
}

float Scroller::getHandleOffset() const
{
  const float scale = (value - minValue) / (maxValue - minValue);

  if (orientation == HORIZONTAL)
    return (getWidth() - getHandleSize()) * scale;
  else
    return (getHeight() - getHandleSize()) * scale;
}

float Scroller::getValueStep() const
{
  return (maxValue - minValue) * percentage;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
