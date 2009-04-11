///////////////////////////////////////////////////////////////////////
// Wendy user interface library
// Copyright (c) 2009 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_UIDESKTOP_H
#define WENDY_UIDESKTOP_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @brief Root object for UI objects.
 *  @ingroup ui
 */
class Desktop : public Trackable
{
public:
  typedef std::list<Widget*> List;
  /*! Constructor.
   */
  Desktop(input::Context& context);
  /*! Destructor.
   */
  ~Desktop(void);
  /*! Draws all root level widgets.
   */
  void drawRootWidgets(void);
  void destroyRootWidgets(void);
  /*! Searches for a widget at the specified point.
   *  @param[in] point The point at which to search.
   *  @return The widget at the specified point, or @c NULL if no matching
   *  widget was found.
   *
   *  @remarks The point is in global coordinates.
   */
  Widget* findWidgetByPoint(const Vector2& point);
  /*! Makes this the active desktop.
   */
  void activate(void);
  /*! @return @c true if this is the active desktop, otherwise @c false.
   */
  bool isActive(void) const;
  /*! @return The root widgets of this desktop.
   */
  const List& getRootWidgets(void) const;
  SignalProxy1<void, Widget&> getDestroyedSignal(void);
  SignalProxy2<void, Desktop&, bool> getFocusChangedSignal(void);
  SignalProxy3<void, Widget&, input::Key, bool> getKeyPressedSignal(void);
  SignalProxy2<void, Widget&, wchar_t> getCharInputSignal(void);
  SignalProxy2<void, Widget&, const Vector2&> getCursorMovedSignal(void);
  SignalProxy4<void, Widget&, const Vector2&, unsigned int, bool> getButtonClickedSignal(void);
  SignalProxy2<void, Widget&, int> getWheelTurnedSignal(void);
  /*! @return The active widget, or @c NULL if no widget is active.
   */
  Widget* getActiveWidget(void);
protected:
  /*! Calls Widget::draw for all children of this widget.
   */
  virtual void draw(void) const;
  virtual void addedChild(Widget& child);
  virtual void removedChild(Widget& child);
  virtual void addedToParent(Widget& parent);
  virtual void removedFromParent(Widget& parent);
private:
  static void onKeyPressed(input::Key key, bool pressed);
  static void onCharInput(wchar_t character);
  static void onCursorMoved(const Vector2& position);
  static void onButtonClicked(unsigned int button, bool clicked);
  static void onWheelTurned(int offset);
  Signal1<void, Widget&> destroyedSignal;
  Signal2<void, Widget&, bool> focusChangedSignal;
  Signal3<void, Widget&, input::Key, bool> keyPressedSignal;
  Signal2<void, Widget&, wchar_t> charInputSignal;
  Signal2<void, Widget&, const Vector2&> cursorMovedSignal;
  Signal4<void, Widget&, const Vector2&, unsigned int, bool> buttonClickedSignal;
  Signal2<void, Widget&, int> wheelTurnedSignal;
  Signal1<void, Widget&> cursorEnteredSignal;
  Signal1<void, Widget&> cursorLeftSignal;
  input::Context& context;
  bool dragging;
  List roots;
  Widget* activeWidget;
  Widget* draggedWidget;
  Widget* hoveredWidget;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIDESKTOP_H*/
///////////////////////////////////////////////////////////////////////
