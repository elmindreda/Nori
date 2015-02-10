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

#pragma once

namespace wendy
{

/*! @ingroup ui
 */
class Entry : public Widget
{
public:
  Entry(Layer& layer, Widget* parent = nullptr, const std::string& text = "");
  const std::string& text() const { return m_controller.text(); }
  void setText(const std::string& text);
  size_t caretPosition() const { return m_controller.caretPosition(); }
  void setCaretPosition(size_t newPosition);
  SignalProxy<void,Entry&> textChanged() { return m_textChanged; }
  SignalProxy<void,Entry&> caretMoved() { return m_caretMoved; }
protected:
  void draw() const;
private:
  void onFocusChanged(bool activated) override;
  void onMouseButton(vec2 point,
                     MouseButton button,
                     Action action,
                     uint mods) override;
  void onKey(Key key, Action action, uint mods) override;
  void onCharacter(uint32 codepoint) override;
  void onTextChanged();
  void onCaretMoved();
  void setCaretPosition(uint newPosition, bool notify);
  Signal<void,Entry&> m_textChanged;
  Signal<void,Entry&> m_caretMoved;
  TextController m_controller;
  Timer m_timer;
};

} /*namespace wendy*/

