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

#include <wendy/UIDrawer.hpp>
#include <wendy/UILayer.hpp>
#include <wendy/UIWidget.hpp>
#include <wendy/UIProgress.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Progress::Progress(Layer& layer, Orientation orientation):
  Widget(layer),
  m_minValue(0.f),
  m_maxValue(1.f),
  m_value(0.f),
  m_orientation(orientation)
{
  init();
}

Progress::Progress(Widget& parent, Orientation orientation):
  Widget(parent),
  m_minValue(0.f),
  m_maxValue(1.f),
  m_value(0.f),
  m_orientation(orientation)
{
  init();
}

void Progress::setValueRange(float newMinValue, float newMaxValue)
{
  m_minValue = newMinValue;
  m_maxValue = newMaxValue;

  if (m_value < m_minValue)
    setValue(m_minValue);
  else if (m_value > m_maxValue)
    setValue(m_maxValue);
  else
    invalidate();
}

void Progress::setValue(float newValue)
{
  m_value = newValue;
  invalidate();
}

void Progress::setOrientation(Orientation newOrientation)
{
  m_orientation = newOrientation;
  invalidate();
}

void Progress::init()
{
  const float em = layer().drawer().currentEM();

  if (m_orientation == HORIZONTAL)
    setDesiredSize(vec2(em * 10.f, em * 1.5f));
  else
    setDesiredSize(vec2(em * 1.5f, em * 10.f));
}

void Progress::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    drawer.drawWell(area, state());

    const float position = (m_value - m_minValue) / (m_maxValue - m_minValue);
    Rect handleArea;

    if (m_orientation == HORIZONTAL)
    {
      handleArea.set(area.position.x + position * (area.size.x - 10.f) + 5.f,
                     area.position.y,
                     10.f,
                     area.size.y);
    }
    else
    {
      handleArea.set(area.position.x,
                     area.position.y + position * (area.size.y - 10.f) + 5.f,
                     area.size.x,
                     10.f);
    }

    drawer.drawHandle(handleArea, state());

    Widget::draw();

    drawer.popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
