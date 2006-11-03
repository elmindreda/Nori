//////////////////////////////////////////////////////////////////////
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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLRender.h>
#include <wendy/GLFont.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>
#include <wendy/UISlider.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Slider::Slider(Orientation initOrientation, const String& name):
  Widget(name),
  minValue(0.f),
  maxValue(1.f),
  value(0.f),
  orientation(initOrientation)
{
  GL::Font* font = Renderer::get()->getCurrentFont();

  if (orientation == HORIZONTAL)
    setSize(Vector2(font->getWidth() * 10.f,
                    font->getHeight() * 1.5f));
  else
    setSize(Vector2(font->getHeight() * 1.5f,
                    font->getWidth() * 10.f));

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

  if (value < minValue)
    setValue(minValue);
  else if (value > maxValue)
    setValue(maxValue);
}

float Slider::getValue(void) const
{
  return value;
}

void Slider::setValue(float newValue, bool notify)
{
  if (notify)
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

  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawFrame(area, getState());

    const float position = (value - minValue) / (maxValue - minValue);

    Rectangle handleArea;

    if (orientation == HORIZONTAL)
    {
      handleArea.set(area.position.x + position * area.size.x - 5.f,
		     area.position.y,
		     10.f,
		     area.size.y);
    }
    else
    {
      handleArea.set(area.position.x,
		     area.position.y + position * area.size.y - 5.f,
		     area.size.x,
		     10.f);
    }

    renderer->drawHandle(handleArea, getState());

    Widget::render();

    renderer->popClipArea();
  }
}

void Slider::onButtonClick(Widget& widget,
	                   const Vector2& position,
	                   unsigned int button,
		           bool clicked)
{
  if (clicked)
  {
    Vector2 localPosition = transformToLocal(position);

    if (orientation == HORIZONTAL)
      setValue(minValue + (maxValue - minValue) * (localPosition.x / getArea().size.x));
    else
      setValue(minValue + (maxValue - minValue) * (localPosition.y / getArea().size.y));
  }
}

void Slider::onKeyPress(Widget& widget, GL::Key key, bool pressed)
{
  if (pressed)
  {
    switch (key)
    {
      case GL::Key::UP:
	setValue(value + 1.f);
	break;
      case GL::Key::DOWN:
	setValue(value - 1.f);
	break;
    }
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
