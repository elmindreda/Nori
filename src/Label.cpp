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

#include <nori/Drawer.hpp>
#include <nori/Layer.hpp>
#include <nori/Widget.hpp>
#include <nori/Label.hpp>

#include <cstdlib>

namespace nori
{

Label::Label(Layer& layer, Widget* parent, const std::string& text, Alignment alignment):
  Widget(layer, parent),
  m_text(text),
  m_alignment(alignment)
{
  Font& font = layer.drawer().theme().font();
  const float em = font.height();
  const float textWidth = font.boundsOf(m_text.c_str()).size.x;

  setDesiredSize(vec2(em * 2.f + textWidth, em * 2.f));
}

void Label::setText(const std::string& text)
{
  m_text = text;
  invalidate();
}

void Label::setTextAlignment(const Alignment& alignment)
{
  m_alignment = alignment;
  invalidate();
}

void Label::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    drawer.setFont(nullptr);
    drawer.drawText(area, m_text.c_str(), m_alignment, state());

    Widget::draw();

    drawer.popClipArea();
  }
}

} /*namespace nori*/

