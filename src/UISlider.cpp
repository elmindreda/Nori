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
#include <wendy/UISlider.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Slider::Slider(Layer& layer, Orientation initOrientation):
  Widget(layer),
  minValue(0.f),
  maxValue(1.f),
  stepSize(1.f),
  value(0.f),
  orientation(initOrientation)
{
  const float em = layer.getDrawer().getCurrentEM();

  if (orientation == HORIZONTAL)
    setSize(vec2(em * 10.f, em * 1.5f));
  else
    setSize(vec2(em * 1.5f, em * 10.f));

  getKeyPressedSignal().connect(*this, &Slider::onKeyPressed);
  getButtonClickedSignal().connect(*this, &Slider::onButtonClicked);
  getWheelTurnedSignal().connect(*this, &Slider::onWheelTurned);
  getDragMovedSignal().connect(*this, &Slider::onDragMoved);

  setDraggable(true);
}

float Slider::getMinValue() const
{
  return minValue;
}

float Slider::getMaxValue() const
{
  return maxValue;
}

void Slider::setValueRange(float newMinValue, float newMaxValue)
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

float Slider::getValue() const
{
  return value;
}

void Slider::setValue(float newValue)
{
  setValue(newValue, false);
}

float Slider::getStepSize() const
{
  return stepSize;
}

void Slider::setStepSize(float newSize)
{
  stepSize = max(newSize, 0.f);
}

SignalProxy1<void, Slider&> Slider::getValueChangedSignal()
{
  return valueChangedSignal;
}

void Slider::draw() const
{
  const Rect& area = getGlobalArea();

  Drawer& drawer = getLayer().getDrawer();
  if (drawer.pushClipArea(area))
  {
    drawer.drawWell(area, getState());

    const float position = (value - minValue) / (maxValue - minValue);

    const float em = drawer.getCurrentEM();

    Rect handleArea;

    if (orientation == HORIZONTAL)
    {
      handleArea.set(area.position.x + position * (area.size.x - em),
                     area.position.y,
                     em,
                     area.size.y);
    }
    else
    {
      handleArea.set(area.position.x,
                     area.position.y + position * (area.size.y - em),
                     area.size.x,
                     em);
    }

    drawer.drawHandle(handleArea, getState());

    Widget::draw();

    drawer.popClipArea();
  }
}

void Slider::onButtonClicked(Widget& widget,
                             const vec2& position,
                             input::Button button,
                             bool clicked)
{
  if (clicked)
    setValue(transformToLocal(position));
}

void Slider::onKeyPressed(Widget& widget, input::Key key, bool pressed)
{
  if (pressed)
  {
    switch (key)
    {
      case input::KEY_UP:
      case input::KEY_RIGHT:
        setValue(value + stepSize, true);
        break;
      case input::KEY_DOWN:
      case input::KEY_LEFT:
        setValue(value - stepSize, true);
        break;
      case input::KEY_HOME:
        setValue(minValue, true);
        break;
      case input::KEY_END:
        setValue(maxValue, true);
       break;
    }
  }
}

void Slider::onWheelTurned(Widget& widget, int offset)
{
  setValue(value + offset * stepSize, true);
}

void Slider::onDragMoved(Widget& widget, const vec2& position)
{
  setValue(transformToLocal(position));
}

void Slider::setValue(const vec2& position)
{
  const float em = getLayer().getDrawer().getCurrentEM();

  float scale;

  if (orientation == HORIZONTAL)
    scale = (position.x - em / 2.f) / (getArea().size.x - em);
  else
    scale = (position.y - em / 2.f) / (getArea().size.y - em);

  setValue(minValue + (maxValue - minValue) * scale, true);
}

void Slider::setValue(float newValue, bool notify)
{
  newValue = clamp(newValue, minValue, maxValue);
  if (newValue == value)
    return;

  value = newValue;

  if (notify)
    valueChangedSignal.emit(*this);

  invalidate();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
