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

#include <wendy/Drawer.hpp>
#include <wendy/Layer.hpp>
#include <wendy/Widget.hpp>
#include <wendy/Button.hpp>

namespace wendy
{

void Button::setChecked(bool checked)
{
  m_checked = checked;
}

void Button::setText(const char* newText)
{
  m_text = newText;
  invalidate();
}

Button::Button(Layer& layer, Widget* parent, ButtonType type, const char* text):
  Widget(layer, parent),
  m_type(type),
  m_text(text),
  m_selected(false),
  m_checked(false)
{
  Font& font = layer.drawer().theme().font();
  const float em = font.height();
  const float textWidth = font.boundsOf(m_text.c_str()).size.x;

  setDesiredSize(vec2(em * 2.f + textWidth, em * 2.f));
  setDraggable(true);
  setFocusable(true);
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

    drawer.setCurrentFont(nullptr);
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
      m_pushed(*this);
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

void Button::onDragBegun(vec2 point, MouseButton button)
{
  if (button == MOUSE_BUTTON_LEFT)
    Widget::onDragBegun(point, button);
  else
    cancelDragging();
}

void Button::onDragEnded(vec2 point, MouseButton button)
{
  m_selected = false;
  invalidate();

  Widget::onDragEnded(point, button);
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
        m_pushed(*this);
        invalidate();
      }

      break;
    }

    default:
      break;
  }

  Widget::onKey(key, action, mods);
}

} /*namespace wendy*/

