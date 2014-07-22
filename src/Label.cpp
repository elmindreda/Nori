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
#include <wendy/Label.hpp>

#include <cstdlib>

namespace wendy
{

Label::Label(Layer& layer, Widget* parent, const char* text, Alignment alignment):
  Widget(layer, parent),
  m_text(text),
  m_textAlignment(alignment)
{
  Font& font = layer.drawer().theme().font();
  const float em = font.height();
  const float textWidth = font.boundsOf(m_text.c_str()).size.x;

  setDesiredSize(vec2(em * 2.f + textWidth, em * 2.f));
}

const String& Label::text() const
{
  return m_text;
}

void Label::setText(const char* newText)
{
  m_text = newText;
  invalidate();
}

const Alignment& Label::textAlignment() const
{
  return m_textAlignment;
}

void Label::setTextAlignment(const Alignment& newAlignment)
{
  m_textAlignment = newAlignment;
  invalidate();
}

void Label::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    drawer.setCurrentFont(nullptr);
    drawer.drawText(area, m_text.c_str(), m_textAlignment, state());

    Widget::draw();

    drawer.popClipArea();
  }
}

} /*namespace wendy*/

