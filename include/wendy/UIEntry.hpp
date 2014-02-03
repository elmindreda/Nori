///////////////////////////////////////////////////////////////////////
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
#ifndef WENDY_UIENTRY_HPP
#define WENDY_UIENTRY_HPP
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class Entry : public Widget
{
public:
  Entry(Layer& layer, const char* text = "");
  const String& text() const;
  void setText(const char* newText);
  uint caretPosition() const;
  void setCaretPosition(uint newPosition);
  SignalProxy<void, Entry&> textChangedSignal();
  SignalProxy<void, Entry&> caretMovedSignal();
protected:
  void draw() const;
private:
  void onMouseButton(vec2 point,
                     MouseButton button,
                     Action action,
                     uint mods) override;
  void onKey(Key key, Action action, uint mods) override;
  void onCharacter(uint32 codepoint, uint mods) override;
  void onTextChanged();
  void onCaretMoved();
  void setCaretPosition(uint newPosition, bool notify);
  Signal<void, Entry&> m_textChangedSignal;
  Signal<void, Entry&> m_caretMovedSignal;
  TextController m_controller;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIENTRY_HPP*/
///////////////////////////////////////////////////////////////////////
