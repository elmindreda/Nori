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
#include <wendy/Entry.hpp>

namespace wendy
{

Entry::Entry(Layer& layer, Widget* parent, const std::string& text):
  Widget(layer, parent),
  m_controller(text)
{
  Font& font = layer.drawer().theme().font();
  const float em = font.height();

  float textWidth;
  if (m_controller.text().empty())
    textWidth = em * 3.f;
  else
    textWidth = font.boundsOf(m_controller.text().c_str()).size.x;

  setDesiredSize(vec2(em * 2.f + textWidth, em * 2.f));

  m_controller.textChanged().connect(*this, &Entry::onTextChanged);
  m_controller.caretMoved().connect(*this, &Entry::onCaretMoved);

  m_timer.start();
  setFocusable(true);
}

void Entry::setText(const std::string& text)
{
  m_controller.setText(text);
  invalidate();
}

void Entry::setCaretPosition(size_t newPosition)
{
  m_controller.setCaretPosition(newPosition);
}

void Entry::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    drawer.drawWell(area, state());

    Font& font = drawer.theme().font();
    const float em = font.height();
    const Rect textArea(area.position + vec2(em / 2.f, 0.f),
                        area.size + vec2(em, 0.f));
    const std::string& text = m_controller.text();

    drawer.setFont(nullptr);
    drawer.drawText(textArea, text.c_str(), LEFT_ALIGNED, state());

    if (isActive() && floor(fmod(m_timer.time(), 2.0)) == 0.0)
    {
      const Rect bounds = font.boundsOf(text.c_str(), 0, m_controller.caretPosition());
      const float position = bounds.size.x;

      const vec2 start = vec2(textArea.position.x + position,
                              textArea.position.y);
      const vec2 end = vec2(textArea.position.x + position,
                            textArea.position.y + textArea.size.y);

      drawer.drawLine(start, end, vec4(drawer.theme().caretColor(state()), 1.f));
    }

    Widget::draw();
    drawer.popClipArea();
  }
}

void Entry::onFocusChanged(bool activated)
{
  if (activated)
    m_timer.start();

  Widget::onFocusChanged(activated);
}

void Entry::onMouseButton(vec2 point,
                          MouseButton button,
                          Action action,
                          uint mods)
{
  if (action == PRESSED)
  {
    Font& font = layer().drawer().theme().font();
    const float em = font.height();
    const float position = transformToLocal(point).x - em / 2.f;
    const auto layout = font.layoutOf(m_controller.text().c_str());

    size_t caretPosition = layout.size();

    for (size_t i = 0;  i < layout.size();  i++)
    {
      if (position < layout[i].position.x + layout[i].size.x / 2.f)
      {
        if (i == 0 || position >= layout[i - 1].position.x +
                                  layout[i - 1].size.x / 2.f)
        {
          caretPosition = i;
          break;
        }
      }
    }

    m_controller.setCaretPosition(caretPosition);
    m_caretMoved(*this);
    m_timer.start();
  }

  Widget::onMouseButton(point, button, action, mods);
}

void Entry::onKey(Key key, Action action, uint mods)
{
  m_controller.inputKey(key, action, mods);
  Widget::onKey(key, action, mods);
}

void Entry::onCharacter(uint32 codepoint)
{
  m_controller.inputCharacter(codepoint);
  Widget::onCharacter(codepoint);
}

void Entry::onTextChanged()
{
  m_timer.start();
  m_textChanged(*this);
  invalidate();
}

void Entry::onCaretMoved()
{
  m_timer.start();
  m_caretMoved(*this);
  invalidate();
}

} /*namespace wendy*/

