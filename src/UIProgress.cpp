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
#include <wendy/GLLight.h>
#include <wendy/GLShader.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderFont.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>
#include <wendy/UIProgress.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Progress::Progress(Orientation initOrientation):
  minValue(0.f),
  maxValue(1.f),
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
}

float Progress::getMinValue(void) const
{
  return minValue;
}

float Progress::getMaxValue(void) const
{
  return maxValue;
}

void Progress::setValueRange(float newMinValue, float newMaxValue)
{
  minValue = newMinValue;
  maxValue = newMaxValue;

  if (value < minValue)
    setValue(minValue);
  else if (value > maxValue)
    setValue(maxValue);
}

float Progress::getValue(void) const
{
  return value;
}

void Progress::setValue(float newValue)
{
  value = newValue;
}

Orientation Progress::getOrientation(void) const
{
  return orientation;
}

void Progress::setOrientation(Orientation newOrientation)
{
  orientation = newOrientation;
}

void Progress::render(void) const
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

    renderer->drawHandle(handleArea, getState());

    Widget::render();

    renderer->popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
