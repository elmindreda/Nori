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
#include <wendy/UIEntry.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Entry::Entry(Layer& layer, const char* initText):
  Widget(layer),
  text(initText),
  startPosition(0),
  caretPosition(0)
{
  const float em = getLayer().getDrawer().getCurrentEM();

  setSize(vec2(em * 10.f, em * 1.5f));

  getButtonClickedSignal().connect(*this, &Entry::onButtonClicked);
  getKeyPressedSignal().connect(*this, &Entry::onKeyPressed);
  getCharInputSignal().connect(*this, &Entry::onCharInput);
}

const String& Entry::getText() const
{
  return text;
}

void Entry::setText(const char* newText)
{
  text = newText;
  invalidate();
}

unsigned int Entry::getCaretPosition() const
{
  return caretPosition;
}

void Entry::setCaretPosition(unsigned int newPosition)
{
  setCaretPosition(newPosition, false);
}

SignalProxy1<void, Entry&> Entry::getTextChangedSignal()
{
  return textChangedSignal;
}

SignalProxy1<void, Entry&> Entry::getCaretMovedSignal()
{
  return caretMovedSignal;
}

void Entry::draw() const
{
  const Rect& area = getGlobalArea();

  Drawer& drawer = getLayer().getDrawer();
  if (drawer.pushClipArea(area))
  {
    drawer.drawWell(area, getState());

    const float em = drawer.getCurrentEM();

    Rect textArea = area;
    textArea.position.x += em / 2.f;
    textArea.size.x -= em;

    drawer.drawText(textArea, text.c_str(), LEFT_ALIGNED, getState());

    if (isActive() && ((unsigned int) (Timer::getCurrentTime() * 2.f) & 1))
    {
      float position = 0.f;

      if (caretPosition > startPosition)
      {
        const render::Font& font = drawer.getCurrentFont();
        Rect metrics = font.getTextMetrics(text.substr(startPosition, caretPosition).c_str());
        position = metrics.size.x;
      }

      Segment2 segment;
      segment.start = vec2(textArea.position.x + position,
                           textArea.position.y);
      segment.end = vec2(textArea.position.x + position,
                         textArea.position.y + textArea.size.y);

      const Theme& theme = drawer.getTheme();

      drawer.drawLine(segment, vec4(theme.caretColors[getState()], 1.f));
    }

    Widget::draw();

    drawer.popClipArea();
  }
}

void Entry::onButtonClicked(Widget& widget,
                            const vec2& point,
                            input::Button button,
                            bool clicked)
{
  if (!clicked)
    return;

  Drawer& drawer = getLayer().getDrawer();

  const float em = drawer.getCurrentEM();
  const float offset = em / 2.f;
  float position = transformToLocal(point).x - offset;

  render::Font::LayoutList layouts;
  drawer.getCurrentFont().getTextLayout(layouts, text.substr(startPosition, String::npos).c_str());

  unsigned int index;

  // TODO: Improve this, it sucks.

  for (index = 0;  index < layouts.size();  index++)
  {
    position -= layouts[index].advance.x;
    if (position < 0.f)
      break;
  }

  setCaretPosition(startPosition + index, true);
}

void Entry::onKeyPressed(Widget& widget, input::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case input::KEY_BACKSPACE:
    {
      if (!text.empty() && caretPosition > 0)
      {
        text.erase(caretPosition - 1, 1);
        textChangedSignal(*this);
        setCaretPosition(caretPosition - 1, true);
      }

      break;
    }

    case input::KEY_DELETE:
    {
      if (!text.empty() && caretPosition < text.length())
      {
        text.erase(caretPosition, 1);
        textChangedSignal(*this);
      }

      break;
    }

    case input::KEY_LEFT:
    {
      if (caretPosition > 0)
        setCaretPosition(caretPosition - 1, true);
      break;
    }

    case input::KEY_RIGHT:
    {
      setCaretPosition(caretPosition + 1, true);
      break;
    }

    case input::KEY_HOME:
    {
      setCaretPosition(0, true);
      break;
    }

    case input::KEY_END:
    {
      setCaretPosition(text.length(), true);
      break;
    }
  }
}

void Entry::onCharInput(Widget& widget, uint32 character)
{
  text.insert(caretPosition, 1, (char) character);
  textChangedSignal(*this);
  setCaretPosition(caretPosition + 1, true);
}

void Entry::setCaretPosition(unsigned int newPosition, bool notify)
{
  if (newPosition > text.length())
    newPosition = text.length();

  if (newPosition == caretPosition)
    return;

  caretPosition = newPosition;

  if (notify)
    caretMovedSignal(*this);

  invalidate();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
