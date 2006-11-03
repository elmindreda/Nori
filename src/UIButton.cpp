//////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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

#include <moira/Moira.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLRender.h>
#include <wendy/GLFont.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>
#include <wendy/UIButton.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Button::Button(const String& initText, const String& name):
  Widget(name),
  text(initText)
{
  GL::Font* font = Renderer::get()->getCurrentFont();

  float textWidth;

  if (text.empty())
    textWidth = font->getWidth() * 3.f;
  else
    textWidth = font->getTextMetrics(text).size.x;

  setSize(Vector2(font->getWidth() * 2.f + textWidth,
                  font->getHeight() * 2.f));

  getButtonClickSignal().connect(*this, &Button::onButtonClick);
  getKeyPressSignal().connect(*this, &Button::onKeyPress);
}

const String& Button::getText(void) const
{
  return text;
}

void Button::setText(const String& newText)
{
  changeTextSignal.emit(*this, newText);
  text = newText;
}

SignalProxy2<void, Button&, const String&> Button::getChangeTextSignal(void)
{
  return changeTextSignal;
}

SignalProxy1<void, Button&> Button::getPushedSignal(void)
{
  return pushedSignal;
}

void Button::render(void) const
{
  const Rectangle& area = getGlobalArea();

  Renderer::get()->drawButton(area, getState(), text);

  Widget::render();
}

void Button::onButtonClick(Widget& widget, const Vector2& position, unsigned int button, bool clicked)
{
  if (button == 0 && clicked == false && getGlobalArea().contains(position))
    pushedSignal.emit(*this);
}

void Button::onKeyPress(Widget& widget, GL::Key key, bool pressed)
{
  if (key == GL::Key::ENTER && pressed == true)
    pushedSignal.emit(*this);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
