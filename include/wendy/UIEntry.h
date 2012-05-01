///////////////////////////////////////////////////////////////////////
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
#ifndef WENDY_UIENTRY_H
#define WENDY_UIENTRY_H
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
  const String& getText() const;
  void setText(const char* newText);
  unsigned int getCaretPosition() const;
  void setCaretPosition(unsigned int newPosition);
  SignalProxy1<void, Entry&> getTextChangedSignal();
  SignalProxy1<void, Entry&> getCaretMovedSignal();
protected:
  void draw() const;
private:
  void onButtonClicked(Widget& widget,
                       const vec2& position,
                       input::Button button,
                       bool clicked);
  void onKeyPressed(Widget& widget, input::Key key, bool pressed);
  void onCharInput(Widget& widget, uint32 character);
  void setText(const String& newText, bool notify);
  void setCaretPosition(unsigned int newPosition, bool notify);
  Signal1<void, Entry&> textChangedSignal;
  Signal1<void, Entry&> caretMovedSignal;
  String text;
  unsigned int startPosition;
  unsigned int caretPosition;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIENTRY_H*/
///////////////////////////////////////////////////////////////////////
