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
#include <wendy/Slider.hpp>

namespace wendy
{

Slider::Slider(Layer& layer, Widget* parent, Orientation orientation):
  Widget(layer, parent),
  m_minValue(0.f),
  m_maxValue(1.f),
  m_stepSize(1.f),
  m_value(0.f),
  m_orientation(orientation)
{
  const float em = layer.drawer().theme().em();

  if (m_orientation == HORIZONTAL)
    setDesiredSize(vec2(em * 10.f, em * 1.5f));
  else
    setDesiredSize(vec2(em * 1.5f, em * 10.f));

  setDraggable(true);
  setFocusable(true);
}

void Slider::setValueRange(float newMinValue, float newMaxValue)
{
  m_minValue = newMinValue;
  m_maxValue = newMaxValue;

  if (m_value < m_minValue)
    setValue(m_minValue, true);
  else if (m_value > m_maxValue)
    setValue(m_maxValue, true);
  else
    invalidate();
}

void Slider::setValue(float newValue)
{
  setValue(newValue, false);
}

void Slider::setStepSize(float newSize)
{
  m_stepSize = max(newSize, 0.f);
}

void Slider::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    const float fraction = (m_value - m_minValue) / (m_maxValue - m_minValue);

    Rect wellArea, handleArea;

    if (m_orientation == HORIZONTAL)
    {
      wellArea.set(area.position.x, area.position.y + area.size.y / 4.f,
                   area.size.x, area.size.y / 2.f);

      handleArea.set(area.position.x + fraction * (area.size.x - area.size.y),
                     area.position.y,
                     area.size.y,
                     area.size.y);
    }
    else
    {
      wellArea.set(area.position.x + area.size.x / 4.f, area.position.y,
                   area.size.x / 2.f, area.size.y);

      handleArea.set(area.position.x,
                     area.position.y + fraction * (area.size.y - area.size.x),
                     area.size.x,
                     area.size.x);
    }

    drawer.drawWell(wellArea, state());
    drawer.drawHandle(handleArea, state());

    Widget::draw();

    drawer.popClipArea();
  }
}

void Slider::onMouseButton(vec2 point,
                           MouseButton button,
                           Action action,
                           uint mods)
{
  if (action == PRESSED)
    setValue(transformToLocal(point));

  Widget::onMouseButton(point, button, action, mods);
}

void Slider::onKey(Key key, Action action, uint mods)
{
  if (action == PRESSED)
  {
    switch (key)
    {
      case KEY_UP:
      case KEY_RIGHT:
        setValue(m_value + m_stepSize, true);
        break;
      case KEY_DOWN:
      case KEY_LEFT:
        setValue(m_value - m_stepSize, true);
        break;
      case KEY_HOME:
        setValue(m_minValue, true);
        break;
      case KEY_END:
        setValue(m_maxValue, true);
        break;
      default:
        break;
    }
  }

  Widget::onKey(key, action, mods);
}

void Slider::onScroll(vec2 offset)
{
  if (m_orientation == HORIZONTAL)
    setValue(m_value + float(offset.x) * m_stepSize, true);
  else
    setValue(m_value + float(offset.y) * m_stepSize, true);

  Widget::onScroll(offset);
}

void Slider::onDragBegun(vec2 point, MouseButton button)
{
  if (button == MOUSE_BUTTON_LEFT)
    Widget::onDragBegun(point, button);
  else
    cancelDragging();
}

void Slider::onDragMoved(vec2 point, MouseButton button)
{
  setValue(transformToLocal(point));

  Widget::onDragMoved(point, button);
}

void Slider::setValue(const vec2& position)
{
  float fraction;

  if (m_orientation == HORIZONTAL)
    fraction = clamp((position.x - height() / 2.f) / (width() - height()), 0.f, 1.f);
  else
    fraction = clamp((position.y - width() / 2.f) / (height() - width()), 0.f, 1.f);

  setValue(m_minValue + (m_maxValue - m_minValue) * fraction, true);
}

void Slider::setValue(float newValue, bool notify)
{
  newValue = clamp(newValue, m_minValue, m_maxValue);
  if (newValue == m_value)
    return;

  m_value = newValue;

  if (notify)
    m_valueChanged(*this);

  invalidate();
}

} /*namespace wendy*/

