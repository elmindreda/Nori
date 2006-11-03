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
#ifndef WENDY_UIBUTTON_H
#define WENDY_UIBUTTON_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class Button : public Widget
{
public:
  Button(const String& text = "", const String& name = "");
  const String& getText(void) const;
  void setText(const String& newText);
  SignalProxy2<void, Button&, const String&> getChangeTextSignal(void);
  SignalProxy1<void, Button&> getPushedSignal(void);
protected:
  void render(void) const;
private:
  void onButtonClick(Widget& widget,
                     const Vector2& position,
		     unsigned int button,
		     bool clicked);
  void onKeyPress(Widget& widget, GL::Key key, bool pressed);
  Signal2<void, Button&, const String&> changeTextSignal;
  Signal1<void, Button&> pushedSignal;
  String text;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIBUTTON_H*/
///////////////////////////////////////////////////////////////////////
