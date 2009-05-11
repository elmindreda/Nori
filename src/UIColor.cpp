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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>

#include <wendy/RenderMaterial.h>
#include <wendy/RenderFont.h>

#include <wendy/Input.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>
#include <wendy/UILayout.h>
#include <wendy/UISlider.h>
#include <wendy/UIColor.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

ColorPickerRGB::ColorPickerRGB(void)
{
  Layout* sliderLayout = new Layout(VERTICAL);
  sliderLayout->setBorderSize(1.f);
  addChild(*sliderLayout);

  for (unsigned int i = 0;  i < 3;  i++)
  {
    sliders[i] = new Slider(HORIZONTAL);
    sliders[i]->setValueRange(0.f, 1.f);
    sliders[i]->getValueChangedSignal().connect(*this, &ColorPickerRGB::onValueChanged);
    sliderLayout->addChild(*sliders[i]);
  }

  // TODO: Set size.
}

const ColorRGB& ColorPickerRGB::getValue(void) const
{
  return value;
}

void ColorPickerRGB::setValue(const ColorRGB& newValue)
{
  value = newValue;
}

SignalProxy1<void, ColorPickerRGB&> ColorPickerRGB::getValueChangedSignal(void)
{
  return valueChangedSignal;
}

void ColorPickerRGB::draw(void) const
{
  const Rect& area = getGlobalArea();

  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawFrame(area, getState());

    Widget::draw();

    renderer->popClipArea();
  }
}

void ColorPickerRGB::onValueChanged(Slider& slider)
{
  for (unsigned int i = 0;  i < 3;  i++)
  {
    if (&slider == sliders[i])
    {
      value[i] = slider.getValue();
      valueChangedSignal.emit(*this);
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
