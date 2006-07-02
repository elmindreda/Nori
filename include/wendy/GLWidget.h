///////////////////////////////////////////////////////////////////////
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
#ifndef WENDY_GLCONTROL_H
#define WENDY_GLCONTROL_H
///////////////////////////////////////////////////////////////////////

#include <sigc++/sigc++.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Widget : public Node<Widget>,
               public Managed<Widget>
{
  friend class Window;
public:
  Widget(const String& name = "");
  virtual ~Widget(void);
  Widget* findByPoint(const Vector2& point);
  bool isEnabled(void) const;
  bool isVisible(void) const;
  void setEnabled(bool newState);
  void setVisible(bool newState);
  const Rectangle& getArea(void) const;
  const Rectangle& getGlobalArea(void) const;
  void setArea(const Rectangle& newArea);
  void setSize(const Vector2& newSize);
  void setPosition(const Vector2& newPosition);
  SignalProxy1<void, Widget&> getDestroySignal(void);
  SignalProxy2<void, Widget&, const Rectangle&> getChangeAreaSignal(void);
  SignalProxy2<void, Widget&, bool> getChangeFocusSignal(void);
  SignalProxy3<void, Widget&, Key, bool> getKeyPressSignal(void);
  SignalProxy2<void, Widget&, const Vector2&> getCursorMoveSignal(void);
  SignalProxy4<void, Widget&, const Vector2&, unsigned int, bool> getButtonClickSignal(void);
  SignalProxy1<void, Widget&> getCursorEnterSignal(void);
  SignalProxy1<void, Widget&> getCursorLeaveSignal(void);
protected:
  virtual void render(void) const;
private:
  Signal1<void, Widget&> destroySignal;
  Signal2<void, Widget&, const Rectangle&> changeAreaSignal;
  Signal2<void, Widget&, bool> changeFocusSignal;
  Signal3<void, Widget&, Key, bool> keyPressSignal;
  Signal2<void, Widget&, const Vector2&> cursorMoveSignal;
  Signal4<void, Widget&, const Vector2&, unsigned int, bool> buttonClickSignal;
  Signal1<void, Widget&> cursorEnterSignal;
  Signal1<void, Widget&> cursorLeaveSignal;
  bool enabled;
  bool visible;
  Rectangle area;
  mutable Rectangle globalArea;
};

///////////////////////////////////////////////////////////////////////

class Button : public Widget
{
public:
  Button(const String& name = "", const String& title = "");
  const String& getTitle(void) const;
  void setTitle(const String& newTitle);
  SignalProxy2<void, Button&, const String&> getChangeTitleSignal(void);
  SignalProxy1<void, Button&> getPushedSignal(void);
protected:
  void render(void) const;
private:
  typedef Signal2<void, Button&, const String&> ChangeTitleSignal;
  typedef Signal1<void, Button&> PushedSignal;
  ChangeTitleSignal changeTitleSignal;
  PushedSignal pushedSignal;
  String title;
};

///////////////////////////////////////////////////////////////////////

class Window : public Widget
{
public:
  Window(const String& name = "", const String& title = "");
  void render(void) const;  
  Widget& getActiveWidget(void);
  void setActiveWidget(const Widget& child);
private:
  void onKeyPress(Key key, bool pressed);
  void onCursorMove(const Vector2& position);
  void onButtonClick(unsigned int button, bool clicked);
  ManagedPtr<Widget> activeWidget;
  String title;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLCONTROL_H*/
///////////////////////////////////////////////////////////////////////
