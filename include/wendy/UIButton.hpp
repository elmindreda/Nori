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
#ifndef WENDY_UIBUTTON_HPP
#define WENDY_UIBUTTON_HPP
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

enum ButtonType
{
  PUSH_BUTTON,
  CHECK_BUTTON
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class Button : public Widget
{
public:
  ButtonType type() const { return m_type; }
  bool checked() const { return m_checked; }
  void setChecked(bool checked);
  const String& text() const;
  void setText(const char* newText);
  SignalProxy<void, Button&> pushedSignal();
protected:
  Button(Layer& layer, ButtonType type, const char* text);
  Button(Widget& parent, ButtonType type, const char* text);
  void draw() const;
  void onMouseButton(vec2 point,
                     MouseButton button,
                     Action action,
                     uint mods) override;
  void onCursorEntered() override;
  void onCursorLeft() override;
  void onDragEnded(vec2 point) override;
  void onKey(Key key, Action action, uint mods) override;
private:
  void init();
  Signal<void, Button&> m_pushedSignal;
  ButtonType m_type;
  String m_text;
  bool m_selected;
  bool m_checked;
};

///////////////////////////////////////////////////////////////////////

class PushButton : public Button
{
public:
  PushButton(Layer& layer, const char* text = ""):
    Button(layer, PUSH_BUTTON, text)
  {
  }
  PushButton(Widget& parent, const char* text = ""):
    Button(parent, PUSH_BUTTON, text)
  {
  }
};

///////////////////////////////////////////////////////////////////////

class CheckButton : public Button
{
public:
  CheckButton(Layer& layer, const char* text = ""):
    Button(layer, CHECK_BUTTON, text)
  {
  }
  CheckButton(Widget& parent, const char* text = ""):
    Button(parent, CHECK_BUTTON, text)
  {
  }
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIBUTTON_HPP*/
///////////////////////////////////////////////////////////////////////
