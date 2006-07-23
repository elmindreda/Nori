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
#ifndef WENDY_UIWIDGET_H
#define WENDY_UIWIDGET_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

enum WidgetState
{
  STATE_NORMAL,
  STATE_ACTIVE,
  STATE_SELECTED,
  STATE_DISABLED,
};

///////////////////////////////////////////////////////////////////////

enum HorzAlignment
{
  LEFT_ALIGNED,
  RIGHT_ALIGNED,
  CENTERED_ON_X,
};

///////////////////////////////////////////////////////////////////////

enum VertAlignment
{
  TOP_ALIGNED,
  BOTTOM_ALIGNED,
  CENTERED_ON_Y,
};

///////////////////////////////////////////////////////////////////////

class Alignment
{
public:
  Alignment(HorzAlignment horizontal = CENTERED_ON_X,
            VertAlignment vertical = CENTERED_ON_Y);
  void set(HorzAlignment newHorizontal, VertAlignment newVertical);
  HorzAlignment horizontal;
  VertAlignment vertical;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Widget renderer singleton.
 *
 *  This class implements the default rendering behavior for widgets.
 * 
 *  @remarks This should probable be made overridable at some point.
 */
class WidgetRenderer : public Singleton<WidgetRenderer>
{
public:
  void drawText(const Rectangle& area,
                const String& text,
		const Alignment& alignment = Alignment(),
		bool selected = false);
  void drawFrame(const Rectangle& area, WidgetState state);
  void drawTextFrame(const Rectangle& area, WidgetState state);
  void drawHandle(const Rectangle& area, WidgetState state);
  void drawButton(const Rectangle& area, WidgetState state, const String& text = "");
  const ColorRGB& getWidgetColor(void);
  const ColorRGB& getTextColor(void);
  const ColorRGB& getTextFrameColor(void);
  const ColorRGB& getSelectionColor(void);
  const ColorRGB& getSelectedTextColor(void);
  GL::Font* getCurrentFont(void);
  GL::Font* getDefaultFont(void);
  static bool create(void);
private:
  WidgetRenderer(void);
  bool init(void);
  ColorRGB widgetColor;
  ColorRGB textColor;
  ColorRGB textFrameColor;
  ColorRGB selectionColor;
  ColorRGB selectedTextColor;
  Ptr<GL::Font> defaultFont;
  GL::Font* currentFont;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Base class for interface widgets.
 *
 *  This is the base class for all interface widgets, including windows.
 *  It also translates and dispatches user input events, with no client
 *  setup required.
 *  
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
  /*! Transforms the specified point from global into local (client
   *  area) coordinates.
   */
  Vector2 transformToLocal(const Vector2& globalPoint) const;
  /*! Transforms the specified point from local (client area) into
   *  global coordinates.
   */
  Vector2 transformToGlobal(const Vector2& localPoint) const;
  void enable(void);
  void disable(void);
  void show(void);
  void hide(void);
  void activate(void);
  bool isEnabled(void) const;
  bool isVisible(void) const;
  bool isActive(void) const;
  bool isUnderCursor(void) const;
  bool isBeingDragged(void) const;
  WidgetState getState(void) const;
  const Rectangle& getArea(void) const;
  const Rectangle& getGlobalArea(void) const;
  void setArea(const Rectangle& newArea);
  void setSize(const Vector2& newSize);
  void setPosition(const Vector2& newPosition);
  SignalProxy1<void, Widget&> getDestroySignal(void);
  SignalProxy2<void, Widget&, const Rectangle&> getChangeAreaSignal(void);
  SignalProxy2<void, Widget&, bool> getChangeFocusSignal(void);
  SignalProxy3<void, Widget&, GL::Key, bool> getKeyPressSignal(void);
  SignalProxy2<void, Widget&, const Vector2&> getCursorMoveSignal(void);
  SignalProxy4<void, Widget&, const Vector2&, unsigned int, bool> getButtonClickSignal(void);
  SignalProxy1<void, Widget&> getCursorEnterSignal(void);
  SignalProxy1<void, Widget&> getCursorLeaveSignal(void);
  SignalProxy2<void, Widget&, const Vector2&> getDragBeginSignal(void);
  SignalProxy2<void, Widget&, const Vector2&> getDragMoveSignal(void);
  SignalProxy2<void, Widget&, const Vector2&> getDragEndSignal(void);
  static Widget* getActive(void);
  static void renderRoots(void);
protected:
  virtual void render(void) const;
private:
  void addedToParent(Widget& parent);
  static void onKeyPress(GL::Key key, bool pressed);
  static void onCursorMove(const Vector2& position);
  static void onButtonClick(unsigned int button, bool clicked);
  Signal1<void, Widget&> destroySignal;
  Signal2<void, Widget&, const Rectangle&> changeAreaSignal;
  Signal2<void, Widget&, bool> changeFocusSignal;
  Signal3<void, Widget&, GL::Key, bool> keyPressSignal;
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
};

///////////////////////////////////////////////////////////////////////

class Label : public Widget
{
public:
  Label(const String& text = "", const String& name = "");
  const String& getText(void) const;
  void setText(const String& newText);
  SignalProxy2<void, Label&, const String&> getChangeTextSignal(void);
private:
  void render(void) const;
  Signal2<void, Label&, const String&> changeTextSignal;
  String text;
};

///////////////////////////////////////////////////////////////////////

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

class Entry : public Widget
{
public:
  Entry(const String& text = "", const String& name = "");
  const String& getText(void) const;
  void setText(const String& newText);
  unsigned int getCaretPosition(void) const;
  void setCaretPosition(unsigned int newPosition);
  SignalProxy2<void, Entry&, const String&> getChangeTextSignal(void);
  SignalProxy2<void, Entry&, unsigned int> getCaretMoveSignal(void);
protected:
  void render(void) const;
private:
  void onButtonClick(Widget& widget,
                     const Vector2& position,
		     unsigned int button,
		     bool clicked);
  void onKeyPress(Widget& widget, GL::Key key, bool pressed);
  Signal2<void, Entry&, const String&> changeTextSignal;
  Signal2<void, Entry&, unsigned int> caretMoveSignal;
  String text;
  unsigned int caretPosition;
};

///////////////////////////////////////////////////////////////////////

class Slider : public Widget
{
public:
  enum Orientation { HORIZONTAL, VERTICAL };
  Slider(Orientation orientation = HORIZONTAL, const String& name = "");
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
  void onKeyPress(Widget& widget, GL::Key key, bool pressed);
  Signal2<void, Slider&, float> changeValueSignal;
  float minValue;
  float maxValue;
  float value;
  Orientation orientation;
};

///////////////////////////////////////////////////////////////////////

class Item
{
  friend class List;
public:
  Item(const String& value = "");
  virtual float getHeight(void) const;
  virtual const String& getValue(void) const;
  virtual void setValue(const String& newValue);
protected:
  virtual void render(const Rectangle& area, bool selected);
private:
  String value;
};

///////////////////////////////////////////////////////////////////////

class List : public Widget
{
public:
  List(const String& name = "");
  void insertItem(Item* item, unsigned int index);
  void removeItem(Item* item);
  void removeItems(void);
  void sortItems(void);
  unsigned int getSelection(void) const;
  Item* getSelectedItem(void) const;
  void setSelection(unsigned int newIndex);
  void setSelectedItem(Item* newItem);
  unsigned int getItemCount(void) const;
  Item* getItem(unsigned int index);
  const Item* getItem(unsigned int index) const;
  SignalProxy2<void, List&, Item&> getAddItemSignal(void);
  SignalProxy2<void, List&, Item&> getRemoveItemSignal(void);
  SignalProxy2<void, List&, unsigned int> getChangeSelectionSignal(void);
protected:
  void render(void) const;
private:
  void onButtonClick(Widget& widget,
                     const Vector2& position,
		     unsigned int button,
		     bool clicked);
  void onKeyPress(Widget& widget, GL::Key key, bool pressed);
  typedef std::list<Item*> ItemList;
  Signal2<void, List&, Item&> addItemSignal;
  Signal2<void, List&, Item&> removeItemSignal;
  Signal2<void, List&, unsigned int> changeSelectionSignal;
  ItemList items;
  unsigned int selection;
};

///////////////////////////////////////////////////////////////////////

class Window : public Widget
{
public:
  Window(const String& title = "", const String& name = "");
  const String& getTitle(void) const;
  void setTitle(const String& newTitle);
private:
  void render(void) const;  
  String title;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIWIDGET_H*/
///////////////////////////////////////////////////////////////////////
