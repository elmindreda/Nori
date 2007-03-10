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
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderFont.h>

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

Slider::Slider(Orientation initOrientation):
  minValue(0.f),
  maxValue(1.f),
  stepSize(1.f),
  value(0.f),
  orientation(initOrientation)
{
  render::Font* font = Renderer::get()->getCurrentFont();

  if (orientation == HORIZONTAL)
    setSize(Vector2(font->getWidth() * 10.f,
                    font->getHeight() * 1.5f));
  else
    setSize(Vector2(font->getHeight() * 1.5f,
                    font->getWidth() * 10.f));

  getKeyPressedSignal().connect(*this, &Slider::onKeyPressed);
  getButtonClickedSignal().connect(*this, &Slider::onButtonClicked);
  getWheelTurnedSignal().connect(*this, &Slider::onWheelTurned);
  getDragMovedSignal().connect(*this, &Slider::onDragMoved);

  setDraggable(true);
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
    setValue(minValue, true);
  else if (value > maxValue)
    setValue(maxValue, true);
}

float Slider::getValue(void) const
{
  return value;
}

void Slider::setValue(float newValue)
{
  setValue(newValue, false);
}

float Slider::getStepSize(void) const
{
  return stepSize;
}

void Slider::setStepSize(float newSize)
{
  stepSize = std::max(newSize, 0.f);
}

Orientation Slider::getOrientation(void) const
{
  return orientation;
}

void Slider::setOrientation(Orientation newOrientation)
{
  orientation = newOrientation;
}

SignalProxy2<void, Slider&, float> Slider::getValueChangedSignal(void)
{
  return valueChangedSignal;
}

void Slider::render(void) const
{
  const Rectangle& area = getGlobalArea();

  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawFrame(area, getState());

    const float position = (value - minValue) / (maxValue - minValue);

    render::Font* font = Renderer::get()->getCurrentFont();

    Rectangle handleArea;

    if (orientation == HORIZONTAL)
    {
      handleArea.set(area.position.x + position * (area.size.x - font->getWidth()),
		     area.position.y,
		     font->getWidth(),
		     area.size.y);
    }
    else
    {
      handleArea.set(area.position.x,
		     area.position.y + position * (area.size.y - font->getHeight()),
		     area.size.x,
		     font->getHeight());
    }

    renderer->drawHandle(handleArea, getState());

    Widget::render();

    renderer->popClipArea();
  }
}

void Slider::onButtonClicked(Widget& widget,
			     const Vector2& position,
			     unsigned int button,
			     bool clicked)
{
  if (clicked)
    setValue(transformToLocal(position));
}

void Slider::onKeyPressed(Widget& widget, GL::Key key, bool pressed)
{
  if (pressed)
  {
    switch (key)
    {
      case GL::Key::UP:
      case GL::Key::RIGHT:
	setValue(value + stepSize, true);
	break;
      case GL::Key::DOWN:
      case GL::Key::LEFT:
	setValue(value - stepSize, true);
	break;
      case GL::Key::HOME:
	setValue(minValue, true);
	break;
      case GL::Key::END:
	setValue(maxValue, true);
	break;
    }
  }
}

void Slider::onWheelTurned(Widget& widget, int offset)
{
  setValue(value + offset * stepSize, true);
}

void Slider::onDragMoved(Widget& widget, const Vector2& position)
{
  setValue(transformToLocal(position));
}

void Slider::setValue(const Vector2& position)
{
  render::Font* font = Renderer::get()->getCurrentFont();

  float scale;

  if (orientation == HORIZONTAL)
    scale = (position.x - font->getWidth() / 2.f) / (getArea().size.x - font->getWidth());
  else
    scale = (position.y - font->getHeight() / 2.f) / (getArea().size.y - font->getHeight());

  setValue(minValue + (maxValue - minValue) * scale, true);
}

void Slider::setValue(float newValue, bool notify)
{
  if (newValue < minValue)
    newValue = minValue;
  else if (newValue > maxValue)
    newValue = maxValue;

  if (newValue == value)
    return;

  if (notify)
    valueChangedSignal.emit(*this, newValue);

  value = newValue;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
