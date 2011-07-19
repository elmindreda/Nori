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
#include <wendy/UIDesktop.h>
#include <wendy/UIWidget.h>
#include <wendy/UIButton.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Button::Button(Desktop& desktop, const String& initText):
  Widget(desktop),
  text(initText)
{
  Drawer& drawer = getDesktop().getDrawer();

  const float em = drawer.getCurrentEM();

  float textWidth;

  if (text.empty())
    textWidth = em * 3.f;
  else
    textWidth = drawer.getCurrentFont().getTextMetrics(text.c_str()).size.x;

  setSize(vec2(em * 2.f + textWidth, em * 2.f));

  getButtonClickedSignal().connect(*this, &Button::onButtonClicked);
  getKeyPressedSignal().connect(*this, &Button::onKeyPressed);
}

const String& Button::getText(void) const
{
  return text;
}

void Button::setText(const String& newText)
{
  text = newText;
  invalidate();
}

SignalProxy1<void, Button&> Button::getPushedSignal(void)
{
  return pushedSignal;
}

void Button::draw(void) const
{
  const Rect& area = getGlobalArea();

  Drawer& drawer = getDesktop().getDrawer();
  if (drawer.pushClipArea(area))
  {
    drawer.drawButton(area, getState(), text);

    Widget::draw();

    drawer.popClipArea();
  }
}

void Button::onButtonClicked(Widget& widget,
                             const vec2& position,
			     input::Button button,
			     bool clicked)
{
  if (clicked)
    return;

  pushedSignal.emit(*this);
}

void Button::onKeyPressed(Widget& widget, input::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case input::KEY_SPACE:
    case input::KEY_ENTER:
    {
      pushedSignal.emit(*this);
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
