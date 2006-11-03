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
#include <wendy/UIEntry.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Entry::Entry(const String& initText, const String& name):
  Widget(name),
  text(initText)
{
  GL::Font* font = Renderer::get()->getCurrentFont();

  setSize(Vector2(font->getWidth() * 10.f,
                  font->getHeight() * 1.5f));

  getButtonClickSignal().connect(*this, &Entry::onButtonClick);
  getKeyPressSignal().connect(*this, &Entry::onKeyPress);
  getCharInputSignal().connect(*this, &Entry::onCharInput);
}

const String& Entry::getText(void) const
{
  return text;
}

void Entry::setText(const String& newText)
{
  changeTextSignal.emit(*this, newText);
  text = newText;
}

unsigned int Entry::getCaretPosition(void) const
{
  return caretPosition;
}

void Entry::setCaretPosition(unsigned int newPosition)
{
  caretMoveSignal.emit(*this, newPosition);
  caretPosition = newPosition;
}

SignalProxy2<void, Entry&, const String&> Entry::getChangeTextSignal(void)
{
  return changeTextSignal;
}

void Entry::render(void) const
{
  const Rectangle& area = getGlobalArea();

  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawTextFrame(area, getState());

    GL::Font* font = renderer->getCurrentFont();

    Rectangle textArea = area;
    textArea.position.x += font->getWidth() / 2.f;
    textArea.size.x -= font->getWidth();

    renderer->drawText(textArea, text, LEFT_ALIGNED);

    Widget::render();

    renderer->popClipArea();
  }
}

void Entry::onButtonClick(Widget& widget,
		          const Vector2& position,
		          unsigned int button,
		          bool clicked)
{
}

void Entry::onKeyPress(Widget& widget, GL::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case GL::Key::BACKSPACE:
    {
      if (!text.empty())
      {
	String newText = text;
	newText.erase(text.size() - 1, 1);
	setText(newText);
      }

      break;
    }
  }
}

void Entry::onCharInput(Widget& widget, wchar_t character)
{
  String newText = text;
  newText.append(1, (char) character);
  setText(newText);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
