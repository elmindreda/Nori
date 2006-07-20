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

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! Base class for interface widgets.
 *  @remarks Yes, it's big. Get over it.
 */
class Widget : public Node<Widget>,
               public Managed<Widget>,
	       public Trackable
{
  friend class Node<Widget>;
public:
  Widget(const String& name = "");
  virtual ~Widget(void);
  Widget* findByPoint(const Vector2& point);
  /*! Removes this widget from its parent.
   *  @remarks Overridden from moira::Node.  Call this if you override it,
   *  lest terror and destruction befall your code.
   */
  void removeFromParent(void);
  bool isEnabled(void) const;
  bool isVisible(void) const;
  bool isActive(void) const;
  bool isUnderCursor(void) const;
  bool isBeingDragged(void) const;
  void enable(void);
  void disable(void);
  void show(void);
  void hide(void);
  void activate(void);
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
  SignalProxy2<void, Widget&, const Vector2&> getDragBeginSignal(void);
  SignalProxy2<void, Widget&, const Vector2&> getDragMoveSignal(void);
  SignalProxy2<void, Widget&, const Vector2&> getDragEndSignal(void);
  static Widget* getActive(void);
  static Font* getDefaultFont(void);
  static void setDefaultFont(Font* newFont);
  static void renderRoots(void);
protected:
  virtual void render(void) const;
private:
  void addedToParent(Widget& parent);
  static void onKeyPress(Key key, bool pressed);
  static void onCursorMove(const Vector2& position);
  static void onButtonClick(unsigned int button, bool clicked);
  Signal1<void, Widget&> destroySignal;
  Signal2<void, Widget&, const Rectangle&> changeAreaSignal;
  Signal2<void, Widget&, bool> changeFocusSignal;
  Signal3<void, Widget&, Key, bool> keyPressSignal;
  Signal2<void, Widget&, const Vector2&> cursorMoveSignal;
  Signal4<void, Widget&, const Vector2&, unsigned int, bool> buttonClickSignal;
  Signal1<void, Widget&> cursorEnterSignal;
  Signal1<void, Widget&> cursorLeaveSignal;
  Signal2<void, Widget&, const Vector2&> dragBeginSignal;
  Signal2<void, Widget&, const Vector2&> dragMoveSignal;
  Signal2<void, Widget&, const Vector2&> dragEndSignal;
  bool enabled;
  bool visible;
  bool underCursor;
  bool beingDragged;
  Rectangle area;
  mutable Rectangle globalArea;
  typedef std::list<Widget*> WidgetList;
  static bool dragging;
  static WidgetList roots;
  static Widget* activeWidget;
  static Widget* draggedWidget;
  static Font* defaultFont;
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
  void onButtonClick(Widget& widget,
                     const Vector2& position,
		     unsigned int button,
		     bool clicked);
  void onKeyPress(Widget& widget, Key key, bool pressed);
  Signal2<void, Button&, const String&> changeTitleSignal;
  Signal1<void, Button&> pushedSignal;
  String title;
};

///////////////////////////////////////////////////////////////////////

class Slider : public Widget
{
public:
  enum Orientation { HORIZONTAL, VERTICAL };
  Slider(const String& name = "");
  float getMinValue(void) const;
  float getMaxValue(void) const;
  void setValueRange(float newMinValue, float newMaxValue);
  float getValue(void) const;
  void setValue(float newValue);
  Orientation getOrientation(void) const;
  void setOrientation(Orientation newOrientation);
  SignalProxy2<void, Slider&, float> getChangeValueSignal(void);
protected:
  void render(void) const;
private:
  void onButtonClick(Widget& widget,
                     const Vector2& position,
		     unsigned int button,
		     bool clicked);
  void onKeyPress(Widget& widget, Key key, bool pressed);
  Signal2<void, Slider&, float> changeValueSignal;
  float minValue;
  float maxValue;
  float value;
  Orientation orientation;
};

///////////////////////////////////////////////////////////////////////

class Window : public Widget
{
public:
  Window(const String& name = "", const String& title = "");
private:
  void render(void) const;  
  String title;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLCONTROL_H*/
///////////////////////////////////////////////////////////////////////
