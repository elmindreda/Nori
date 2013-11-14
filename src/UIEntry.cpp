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

#include <wendy/Config.hpp>

#include <wendy/UIDrawer.hpp>
#include <wendy/UILayer.hpp>
#include <wendy/UIWidget.hpp>
#include <wendy/UIEntry.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Entry::Entry(Layer& layer, const char* text):
  Widget(layer),
  m_controller(text)
{
  const float em = layer.drawer().currentEM();

  setSize(vec2(em * 10.f, em * 1.5f));

  buttonClickedSignal().connect(*this, &Entry::onMouseButton);
  keyPressedSignal().connect(*this, &Entry::onKey);
  charInputSignal().connect(*this, &Entry::onCharacter);

  m_controller.textChangedSignal().connect(*this, &Entry::onTextChanged);
  m_controller.caretMovedSignal().connect(*this, &Entry::onCaretMoved);
}

const String& Entry::text() const
{
  return m_controller.text();
}

void Entry::setText(const char* newText)
{
  m_controller.setText(newText);
  invalidate();
}

uint Entry::caretPosition() const
{
  return m_controller.caretPosition();
}

void Entry::setCaretPosition(uint newPosition)
{
  m_controller.setCaretPosition(newPosition);
}

SignalProxy1<void, Entry&> Entry::textChangedSignal()
{
  return m_textChangedSignal;
}

SignalProxy1<void, Entry&> Entry::caretMovedSignal()
{
  return m_caretMovedSignal;
}

void Entry::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    drawer.drawWell(area, state());

    const float em = drawer.currentEM();
    const Rect textArea(area.position + vec2(em / 2.f, 0.f),
                        area.size + vec2(em, 0.f));
    const String& text = m_controller.text();

    drawer.drawText(textArea, text.c_str(), LEFT_ALIGNED, state());

    if (isActive() && ((uint) (Timer::currentTime() * 2.f) & 1))
    {
      float position = 0.f;

      render::Font& font = drawer.currentFont();
      const Rect bounds = font.boundsOf(text.substr(0, m_controller.caretPosition()).c_str());
      position = bounds.size.x;

      const vec2 start = vec2(textArea.position.x + position,
                              textArea.position.y);
      const vec2 end = vec2(textArea.position.x + position,
                            textArea.position.y + textArea.size.y);

      drawer.drawLine(start, end, vec4(drawer.theme().caretColors[state()], 1.f));
    }

    Widget::draw();

    drawer.popClipArea();
  }
}

void Entry::onMouseButton(Widget& widget,
                          vec2 point,
                          MouseButton button,
                          Action action,
                          uint mods)
{
  if (action != PRESSED)
    return;

  Drawer& drawer = layer().drawer();

  const float em = drawer.currentEM();
  const float offset = em / 2.f;
  float position = transformToLocal(point).x - offset;

  std::vector<Rect> layout = drawer.currentFont().layoutOf(m_controller.text().c_str());

  uint index;

  // TODO: Improve this, it sucks.

  for (index = 0;  index < layout.size();  index++)
  {
    position -= layout[index].position.x;
    if (position < 0.f)
      break;
  }

  m_controller.setCaretPosition(index);
  m_caretMovedSignal(*this);
}

void Entry::onKey(Widget& widget, Key key, Action action, uint mods)
{
  m_controller.inputKey(key, action, mods);
}

void Entry::onCharacter(Widget& widget, uint32 codepoint, uint mods)
{
  m_controller.inputCharacter(codepoint, mods);
}

void Entry::onTextChanged()
{
  m_textChangedSignal(*this);
}

void Entry::onCaretMoved()
{
  m_caretMovedSignal(*this);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
