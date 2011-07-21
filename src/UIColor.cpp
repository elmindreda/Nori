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

#include <wendy/Config.h>

#include <wendy/UIDrawer.h>
#include <wendy/UIModule.h>
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

ColorPickerRGB::ColorPickerRGB(Module& module):
  Widget(module)
{
  Layout* sliderLayout = new Layout(module, VERTICAL);
  sliderLayout->setBorderSize(1.f);
  addChild(*sliderLayout);

  for (unsigned int i = 0;  i < 3;  i++)
  {
    sliders[i] = new Slider(module, HORIZONTAL);
    sliders[i]->setValueRange(0.f, 1.f);
    sliders[i]->getValueChangedSignal().connect(*this, &ColorPickerRGB::onValueChanged);
    sliderLayout->addChild(*sliders[i]);
  }

  // TODO: Set size.
}

const vec3& ColorPickerRGB::getValue(void) const
{
  return value;
}

void ColorPickerRGB::setValue(const vec3& newValue)
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

  Drawer& drawer = getModule().getDrawer();
  if (drawer.pushClipArea(area))
  {
    drawer.drawFrame(area, getState());

    Widget::draw();

    drawer.popClipArea();
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
