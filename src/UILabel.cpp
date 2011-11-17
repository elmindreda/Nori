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
#include <wendy/UILabel.h>

#include <cstdlib>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Label::Label(Layer& layer, const char* initText):
  Widget(layer),
  text(initText),
  textAlignment(LEFT_ALIGNED)
{
  Drawer& drawer = layer.getDrawer();

  const float em = drawer.getCurrentEM();

  float textWidth;

  if (text.empty())
    textWidth = em * 3.f;
  else
    textWidth = drawer.getCurrentFont().getTextMetrics(text.c_str()).size.x;

  setSize(vec2(em * 2.f + textWidth, em * 2.f));
}

const String& Label::getText() const
{
  return text;
}

void Label::setText(const char* newText)
{
  text = newText;
  invalidate();
}

const Alignment& Label::getTextAlignment() const
{
  return textAlignment;
}

void Label::setTextAlignment(const Alignment& newAlignment)
{
  textAlignment = newAlignment;
  invalidate();
}

void Label::draw() const
{
  const Rect& area = getGlobalArea();

  Drawer& drawer = getLayer().getDrawer();
  if (drawer.pushClipArea(area))
  {
    drawer.drawText(area, text.c_str(), textAlignment);

    Widget::draw();

    drawer.popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
