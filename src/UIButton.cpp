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

#include <wendy/GLContext.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLShader.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>

#include <wendy/RenderMaterial.h>
#include <wendy/RenderFont.h>

#include <wendy/Input.h>

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

Button::Button(const String& initText):
  text(initText)
{
  Renderer* renderer = Renderer::get();

  const float em = renderer->getDefaultEM();

  float textWidth;

  if (text.empty())
    textWidth = em * 3.f;
  else
    textWidth = renderer->getDefaultFont()->getTextMetrics(text).size.x;

  setSize(Vector2(em * 2.f + textWidth, em * 2.f));

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
}

SignalProxy1<void, Button&> Button::getPushedSignal(void)
{
  return pushedSignal;
}

void Button::draw(void) const
{
  const Rectangle& area = getGlobalArea();

  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    Renderer::get()->drawButton(area, getState(), text);

    Widget::draw();

    renderer->popClipArea();
  }
}

void Button::onButtonClicked(Widget& widget,
                             const Vector2& position,
			     unsigned int button,
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
    case input::Key::SPACE:
    case input::Key::ENTER:
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
