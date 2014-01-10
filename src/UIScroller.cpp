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

Scroller::Scroller(Layer& layer, Orientation orientation):
  Widget(layer),
  m_minValue(0.f),
  m_maxValue(1.f),
  m_value(0.f),
  m_percentage(0.5f),
  m_reference(0.f),
  m_orientation(orientation)
{
  const float em = layer.drawer().currentEM();

  if (m_orientation == HORIZONTAL)
    setSize(vec2(em * 10.f, em * 1.5f));
  else
    setSize(vec2(em * 1.5f, em * 10.f));

  setDraggable(true);
}

void Scroller::setValueRange(float newMinValue, float newMaxValue)
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

void Scroller::setValue(float newValue)
{
  setValue(newValue, false);
}

void Scroller::setPercentage(float newPercentage)
{
  m_percentage = clamp(newPercentage, 0.f, 1.f);
  invalidate();
}

SignalProxy1<void, Scroller&> Scroller::valueChangedSignal()
{
  return m_valueChangedSignal;
}

void Scroller::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    drawer.drawWell(area, state());

    if (m_minValue != m_maxValue)
    {
      const float size = handleSize();
      const float offset = handleOffset();

      Rect handleArea;

      if (m_orientation == HORIZONTAL)
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

      drawer.drawHandle(handleArea, state());
    }

    Widget::draw();

    drawer.popClipArea();
  }
}

void Scroller::onMouseButton(vec2 point,
                             MouseButton button,
                             Action action,
                             uint mods)
{
  if (action == PRESSED)
  {
    const vec2 local = transformToLocal(point);
    const float size = handleSize();
    const float offset = handleOffset();

    if (m_orientation == HORIZONTAL)
    {
      if (local.x < offset)
        setValue(m_value - valueStep(), true);
      else if (local.x >= offset + size)
        setValue(m_value + valueStep(), true);
    }
    else
    {
      if (local.y > height() - offset)
        setValue(m_value - valueStep(), true);
      else if (local.y <= height() - offset - size)
        setValue(m_value + valueStep(), true);
    }
  }

  Widget::onMouseButton(point, button, action, mods);
}

void Scroller::onKey(Key key, Action action, uint mods)
{
  if (action != RELEASED)
  {
    switch (key)
    {
      case KEY_DOWN:
      case KEY_RIGHT:
      {
        setValue(m_value + valueStep(), true);
        break;
      }

      case KEY_UP:
      case KEY_LEFT:
      {
        setValue(m_value - valueStep(), true);
        break;
      }

      case KEY_HOME:
      {
        setValue(m_minValue, true);
        break;
      }

      case KEY_END:
      {
        setValue(m_maxValue, true);
        break;
      }

      default:
        break;
    }
  }

  Widget::onKey(key, action, mods);
}

void Scroller::onScroll(vec2 offset)
{
  if (m_orientation == HORIZONTAL)
    setValue(m_value + float(offset.x) * valueStep(), true);
  else
    setValue(m_value + float(offset.y) * valueStep(), true);

  Widget::onScroll(offset);
}

void Scroller::onDragBegun(vec2 point)
{
  const vec2 local = transformToLocal(point);
  const float size = handleSize();
  const float offset = handleOffset();

  if (m_orientation == HORIZONTAL)
  {
    if (local.x >= offset && local.x < offset + size)
      m_reference = local.x - offset;
    else
      cancelDragging();
  }
  else
  {
    if (local.y <= height() - offset && local.y > height() - offset - size)
      m_reference = height() - local.y - offset;
    else
      cancelDragging();
  }

  Widget::onDragBegun(point);
}

void Scroller::onDragMoved(vec2 point)
{
  const vec2 local = transformToLocal(point);
  const float size = handleSize();

  float scale;

  if (m_orientation == HORIZONTAL)
    scale = (local.x - m_reference) / (width() - size);
  else
    scale = (height() - local.y - m_reference) / (height() - size);

  setValue(m_minValue + (m_maxValue - m_minValue) * scale, true);

  Widget::onDragMoved(point);
}

void Scroller::setValue(float newValue, bool notify)
{
  newValue = clamp(newValue, m_minValue, m_maxValue);
  if (newValue == m_value)
    return;

  m_value = newValue;

  if (notify)
    m_valueChangedSignal(*this);

  invalidate();
}

float Scroller::handleSize() const
{
  const float em = layer().drawer().currentEM();

  if (m_orientation == HORIZONTAL)
    return max(width() * m_percentage, em);
  else
    return max(height() * m_percentage, em);
}

float Scroller::handleOffset() const
{
  const float scale = (m_value - m_minValue) / (m_maxValue - m_minValue);

  if (m_orientation == HORIZONTAL)
    return (width() - handleSize()) * scale;
  else
    return (height() - handleSize()) * scale;
}

float Scroller::valueStep() const
{
  return (m_maxValue - m_minValue) * m_percentage;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
