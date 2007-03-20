//////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLLight.h>
#include <wendy/GLShader.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderFont.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>
#include <wendy/UIScroller.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Scroller::Scroller(Orientation initOrientation):
  minValue(0.f),
  maxValue(1.f),
  value(0.f),
  percentage(0.5f),
  orientation(initOrientation)
{
  render::Font* font = Renderer::get()->getCurrentFont();

  if (orientation == HORIZONTAL)
    setSize(Vector2(font->getWidth() * 10.f,
                    font->getHeight() * 1.5f));
  else
    setSize(Vector2(font->getHeight() * 1.5f,
                    font->getWidth() * 10.f));

  getKeyPressedSignal().connect(*this, &Scroller::onKeyPressed);
  getButtonClickedSignal().connect(*this, &Scroller::onButtonClicked);
  getWheelTurnedSignal().connect(*this, &Scroller::onWheelTurned);
  getDragBegunSignal().connect(*this, &Scroller::onDragBegun);
  getDragMovedSignal().connect(*this, &Scroller::onDragMoved);

  setDraggable(true);
}

float Scroller::getMinValue(void) const
{
  return minValue;
}

float Scroller::getMaxValue(void) const
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
}

float Scroller::getValue(void) const
{
  return value;
}

void Scroller::setValue(float newValue)
{
  setValue(newValue, false);
}

float Scroller::getPercentage(void) const
{
  return percentage;
}

void Scroller::setPercentage(float newPercentage)
{
  percentage = std::max(std::min(newPercentage, 1.f), 0.f);
}

SignalProxy1<void, Scroller&> Scroller::getValueChangedSignal(void)
{
  return valueChangedSignal;
}

void Scroller::draw(void) const
{
  const Rectangle& area = getGlobalArea();

  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawFrame(area, getState());

    const float size = getHandleSize();
    const float scale = (value - minValue) / (maxValue - minValue);

    Rectangle handleArea;

    if (orientation == HORIZONTAL)
    {
      handleArea.set(area.position.x + scale * (area.size.x - size),
		     area.position.y,
		     size,
		     area.size.y);
    }
    else
    {
      handleArea.set(area.position.x,
		     area.position.y + scale * (area.size.y - size),
		     area.size.x,
		     size);
    }

    renderer->drawHandle(handleArea, getState());

    Widget::draw();

    renderer->popClipArea();
  }
}

void Scroller::onButtonClicked(Widget& widget,
			       const Vector2& point,
			       unsigned int button,
			       bool clicked)
{
  if (!clicked)
    return;

  Vector2 localPoint = transformToLocal(point);

  const Rectangle& area = getArea();

  const float size = getHandleSize();
  const float scale = (value - minValue) / (maxValue - minValue);
  const float stepScale = percentage / (1.f - percentage);

  if (orientation == HORIZONTAL)
  {
    const float offset = (area.size.x - size) * scale;

    if (localPoint.x < offset)
      setValue(value - (maxValue - minValue) * stepScale, true);
    else if (localPoint.x >= offset + size)
      setValue(value + (maxValue - minValue) * stepScale, true);
  }
  else
  {
    const float offset = (area.size.y - size) * scale;

    if (localPoint.y < offset)
      setValue(value - (maxValue - minValue) * stepScale, true);
    else if (localPoint.y >= offset + size)
      setValue(value + (maxValue - minValue) * stepScale, true);
  }
}

void Scroller::onKeyPressed(Widget& widget, GL::Key key, bool pressed)
{
  if (pressed)
  {
    switch (key)
    {
      case GL::Key::UP:
      case GL::Key::RIGHT:
      {
	const float stepScale = percentage / (1.f - percentage);
	setValue(value + (maxValue - minValue) * stepScale, true);
	break;
      }

      case GL::Key::DOWN:
      case GL::Key::LEFT:
      {
	const float stepScale = percentage / (1.f - percentage);
	setValue(value - (maxValue - minValue) * stepScale, true);
	break;
      }

      case GL::Key::HOME:
      {
	setValue(minValue, true);
	break;
      }

      case GL::Key::END:
      {
	setValue(maxValue, true);
	break;
      }
    }
  }
}

void Scroller::onWheelTurned(Widget& widget, int offset)
{
  const float stepScale = percentage / (1.f - percentage);
  setValue(value + offset * (maxValue - minValue) * stepScale, true);
}

void Scroller::onDragBegun(Widget& widget, const Vector2& point)
{
  Vector2 localPoint = transformToLocal(point);

  const Rectangle& area = getArea();

  const float size = getHandleSize();
  const float scale = (value - minValue) / (maxValue - minValue);

  if (orientation == HORIZONTAL)
  {
    const float offset = (area.size.x - size) * scale;

    if (localPoint.x >= offset && localPoint.x < offset + size)
      reference = localPoint.x - offset;
    else
      cancelDragging();
  }
  else
  {
    const float offset = (area.size.y - size) * scale;

    if (localPoint.y >= offset && localPoint.y < offset + size)
      reference = localPoint.y - offset;
    else
      cancelDragging();
  }
}

void Scroller::onDragMoved(Widget& widget, const Vector2& point)
{
  Vector2 localPoint = transformToLocal(point);

  const Rectangle& area = getArea();

  const float size = getHandleSize();

  float scale;

  if (orientation == HORIZONTAL)
    scale = (localPoint.x - reference) / (area.size.x - size);
  else
    scale = (localPoint.y - reference) / (area.size.y - size);

  setValue(minValue + (maxValue - minValue) * scale, true);
}

void Scroller::setValue(float newValue, bool notify)
{
  if (newValue < minValue)
    newValue = minValue;
  else if (newValue > maxValue)
    newValue = maxValue;

  if (newValue == value)
    return;

  value = newValue;

  if (notify)
    valueChangedSignal.emit(*this);
}

float Scroller::getHandleSize(void) const
{
  render::Font* font = Renderer::get()->getCurrentFont();

  if (orientation == HORIZONTAL)
    return std::max(getArea().size.x * percentage, font->getWidth());
  else
    return std::max(getArea().size.y * percentage, font->getHeight());
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
