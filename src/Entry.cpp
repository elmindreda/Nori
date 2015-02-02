//////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

#include <nori/Config.hpp>

#include <nori/Theme.hpp>
#include <nori/Layer.hpp>
#include <nori/Widget.hpp>
#include <nori/Entry.hpp>

namespace nori
{

Entry::Entry(Layer& layer, Widget* parent, const std::string& text):
  Widget(layer, parent),
  m_controller(text)
{
  Theme& theme = layer.theme();
  const float em = theme.em();
  const float textWidth = theme.context().textBounds(vec2(0.f), text.c_str()).size.x;

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
  Theme& theme = layer().theme();

  const Rect area = globalArea();
  if (theme.pushClipArea(area))
  {
    theme.drawWell(area, state());

    const float em = theme.em();
    const Rect textArea(area.position + vec2(em / 2.f, 0.f),
                        area.size + vec2(em, 0.f));
    const std::string& text = m_controller.text();

    theme.drawText(textArea, state(), ALIGN_LEFT | ALIGN_MIDDLE, text.c_str());

    if (isActive() && floor(mod(m_timer.time(), 2.0)) == 0.0)
    {
      VectorContext& vc = theme.context();
      const Rect bounds(vc.textBounds(vec2(0.f),
                                      text.c_str(),
                                      text.c_str() + m_controller.caretPosition()));
      const float position = bounds.size.x;
      const vec2 start(textArea.position.x + position,
                       textArea.position.y);
      const vec2 end(textArea.position.x + position,
                     textArea.position.y + textArea.size.y);

      vc.beginPath();
      vc.moveTo(start);
      vc.lineTo(end);
      vc.strokeColor(vec4(1.f));
      vc.strokeWidth(1.f);
      vc.stroke();
    }

    Widget::draw();
    theme.popClipArea();
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
    Theme& theme = layer().theme();
    VectorContext& vc = theme.context();
    const float em = theme.em();
    const float position = transformToLocal(point).x - em / 2.f;
    const auto positions = vc.textGlyphPositions(vec2(0.f), m_controller.text().c_str());

    size_t caretPosition = positions.size();

    for (size_t i = 0;  i < positions.size();  i++)
    {
      if (position < (positions[i].minx + positions[i].maxx) / 2.f)
      {
        if (i == 0 || position >= (positions[i - 1].minx +
                                   positions[i - 1].maxx) / 2.f)
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

} /*namespace nori*/

