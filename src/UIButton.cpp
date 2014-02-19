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
#include <wendy/UIButton.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

void Button::setChecked(bool checked)
{
  m_checked = checked;
}

const String& Button::text() const
{
  return m_text;
}

void Button::setText(const char* newText)
{
  m_text = newText;
  invalidate();
}

SignalProxy<void, Button&> Button::pushedSignal()
{
  return m_pushedSignal;
}

Button::Button(Layer& layer, ButtonType type, const char* text):
  Widget(layer),
  m_type(type),
  m_text(text),
  m_selected(false),
  m_checked(false)
{
  init();
}

Button::Button(Widget& parent, ButtonType type, const char* text):
  Widget(parent),
  m_type(type),
  m_text(text),
  m_selected(false),
  m_checked(false)
{
  init();
}

void Button::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    WidgetState hoverState;

    if (isUnderCursor() && m_selected)
      hoverState = STATE_SELECTED;
    else
      hoverState = state();

    if (m_type == PUSH_BUTTON)
      drawer.drawButton(area, hoverState, m_text.c_str());
    else if (m_type == CHECK_BUTTON)
      drawer.drawCheck(area, hoverState, m_checked, m_text.c_str());

    Widget::draw();

    drawer.popClipArea();
  }
}

void Button::onMouseButton(vec2 point,
                           MouseButton button,
                           Action action,
                           uint mods)
{
  if (button == MOUSE_BUTTON_LEFT)
  {
    if (action == PRESSED)
      m_selected = true;
    else if (action == RELEASED)
    {
      m_checked = !m_checked;
      m_selected = false;
      m_pushedSignal(*this);
    }

    invalidate();
  }

  Widget::onMouseButton(point, button, action, mods);
}

void Button::onCursorEntered()
{
  if (m_selected)
    invalidate();
}

void Button::onCursorLeft()
{
  if (m_selected)
    invalidate();
}

void Button::onDragEnded(vec2 point)
{
  m_selected = false;
  invalidate();

  Widget::onDragEnded(point);
}

void Button::onKey(Key key, Action action, uint mods)
{
  switch (key)
  {
    case KEY_SPACE:
    case KEY_ENTER:
    {
      if (action == PRESSED)
      {
        m_checked = !m_checked;
        m_selected = false;
        m_pushedSignal(*this);
        invalidate();
      }

      break;
    }

    default:
      break;
  }

  Widget::onKey(key, action, mods);
}

void Button::init()
{
  Drawer& drawer = layer().drawer();
  drawer.setCurrentFont(nullptr);
  const float em = drawer.currentEM();

  float textWidth;
  if (m_text.empty())
    textWidth = em * 3.f;
  else
    textWidth = drawer.currentFont().boundsOf(m_text.c_str()).size.x;

  setDesiredSize(vec2(em * 2.f + textWidth, em * 2.f));
  setDraggable(true);
  setFocusable(true);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
