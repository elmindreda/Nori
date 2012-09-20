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
#include <wendy/UILayer.h>
#include <wendy/UIWidget.h>
#include <wendy/UIButton.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Button::Button(Layer& layer, const char* initText):
  Widget(layer),
  selected(false),
  text(initText)
{
  Drawer& drawer = getLayer().getDrawer();

  const float em = drawer.getCurrentEM();

  float textWidth;

  if (text.empty())
    textWidth = em * 3.f;
  else
    textWidth = drawer.getCurrentFont().getTextMetrics(text.c_str()).size.x;

  setSize(vec2(em * 2.f + textWidth, em * 2.f));
  setDraggable(true);

  getDragEndedSignal().connect(*this, &Button::onDragEnded);
  getButtonClickedSignal().connect(*this, &Button::onButtonClicked);
  getKeyPressedSignal().connect(*this, &Button::onKeyPressed);
}

const String& Button::getText() const
{
  return text;
}

void Button::setText(const char* newText)
{
  text = newText;
  invalidate();
}

SignalProxy1<void, Button&> Button::getPushedSignal()
{
  return pushedSignal;
}

void Button::draw() const
{
  const Rect& area = getGlobalArea();

  Drawer& drawer = getLayer().getDrawer();
  if (drawer.pushClipArea(area))
  {
    WidgetState state;

    if (isUnderCursor() && selected)
      state = STATE_SELECTED;
    else
      state = getState();

    drawer.drawButton(area, state, text.c_str());

    Widget::draw();

    drawer.popClipArea();
  }
}

void Button::onButtonClicked(Widget& widget,
                             const vec2& position,
                             input::Button button,
                             bool clicked)
{
  if (button == input::BUTTON_LEFT)
  {
    if (clicked)
      selected = true;
    else
    {
      selected = false;
      pushedSignal(*this);
    }

    invalidate();
  }
}

void Button::onDragEnded(Widget& widget, const vec2& position)
{
  selected = false;
  invalidate();
}

void Button::onKeyPressed(Widget& widget, input::Key key, bool pressed)
{
  switch (key)
  {
    case input::KEY_SPACE:
    case input::KEY_ENTER:
    {
      if (pressed)
        selected = true;
      else
      {
        selected = false;
        pushedSignal(*this);
        invalidate();
      }

      break;
    }

    default:
      break;
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
