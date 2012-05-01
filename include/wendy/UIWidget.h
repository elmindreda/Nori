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

class Layer;

///////////////////////////////////////////////////////////////////////

/*! @brief Base class for widgets.
 *  @ingroup ui
 *
 *  This is the base class for all %UI widgets.
 *
 *  @remarks Yes, it's big.
 */
class Widget : public Trackable
{
  friend class Layer;
public:
  /*! Constructor.
   */
  Widget(Layer& layer);
  /*! Destructor.
   */
  ~Widget();
  /*! Adds the specified widget as a child of this widget.
   *  @param[in] child The widget to add.
   *  @remarks The specified widget must not be a parent or ancestor of this
   *  widget.
   */
  void addChild(Widget& child);
  /*! Destroys all children, recursively, of this widget.
   */
  void destroyChildren();
  /*! Detaches this widget from its parent, if any.
   */
  void removeFromParent();
  /*! Searches for a widget at the specified point.
   *  @param[in] point The point at which to search.
   *  @return The widget at the specified point, or @c NULL if no matching
   *  widget was found.
   *
   *  @remarks The point is in parent coordinates.  If this is a top-level
   *  widget, it is in global coordinates.
   */
  Widget* findByPoint(const vec2& point);
  /*! Transforms the specified point from global into local (client
   *  area) coordinates.
   *  @param[in] globalPoint The global coordinate point to transform.
   *  @return The corresponding local coordinate point.
   */
  vec2 transformToLocal(const vec2& globalPoint) const;
  /*! Transforms the specified point from local (client area) into
   *  global coordinates.
   *  @param[in] localPoint The local coordinate point to transform.
   *  @return The corresponding global coordinate point.
   */
  vec2 transformToGlobal(const vec2& localPoint) const;
  /*! Makes this widget visible.
   */
  void show();
  /*! Makes this widget non-visible.
   */
  void hide();
  /*! Enables this widget, allowing it to become active and receive input
   *  events.
   */
  void enable();
  /*! Disables this widget, disallowing activation and reception of input
   *  events.
   */
  void disable();
  /*! Flags the layer this widget belongs to as needing to redraw itself.
   *
   *  @remarks This is a helper method for Layer::invalidate.
   */
  void invalidate();
  /*! Makes this the active widget.
   *
   *  @remarks This is ignored if the widget is hidden or disabled.
   */
  void activate();
  /*! Makes this the top-level widget.
   */
  void bringToFront();
  /*! Makes this the bottom-most widget.
   */
  void sendToBack();
  /*! Cancels any current drag operation.
   */
  void cancelDragging();
  /*! @return @c true if this widget is enabled, otherwise @c false.
   *
   *  @remarks This is a helper method for Layer::cancelDragging.
   */
  bool isEnabled() const;
  /*! @return @c true if this widget is visible, otherwise @c false.
   */
  bool isVisible() const;
  /*! @return @c true if this is the active widget, otherwise @c false.
   */
  bool isActive() const;
  /*! @return @c true if this widget is directly under the cursor, otherwise @c
   *  false.
   */
  bool isUnderCursor() const;
  /*! @return @c true if this widget supports dragging, otherwise @c false.
   */
  bool isDraggable() const;
  /*! @return @c true if this widget is currently the source of a dragging
   *  operation, otherwise @c false.
   */
  bool isBeingDragged() const;
  /*! @return @c true if the specified widget is a parent or ancestor of this
   *  widget.
   */
  bool isChildOf(const Widget& widget) const;
  /*! @return @c true if this widget has captured the cursor, otherwise @c
   *  false.
   */
  bool hasCapturedCursor() const;
  /*! @return The layer this widget belongs to.
   */
  Layer& getLayer() const;
  /*! @return The parent of this widget, or @c NULL if it has no parent.
   */
  Widget* getParent() const;
  /*! @return The child widgets of this widget.
   */
  const WidgetList& getChildren() const;
  WidgetState getState() const;
  /*! @return The width of this widget.
   */
  float getWidth() const;
  /*! @return The height of this widget.
   */
  float getHeight() const;
  /*! @return The area of this widget, in parent coordinates.
   */
  const Rect& getArea() const;
  /*! @return The area of this widget, in global coordinates.
   */
  const Rect& getGlobalArea() const;
  /*! Sets the area of this widget.
   *  @param[in] newArea The desired area, in parent coordinates.
   */
  void setArea(const Rect& newArea);
  /*! @return The size of this widget.
   */
  const vec2& getSize() const;
  /*! Sets the size of this widget.
   *  @param[in] newSize The desired size, in parent coordinates.
   *
   *  @remarks This is a helper method for Widget::setArea.
   */
  void setSize(const vec2& newSize);
  /*! Sets the position of this widget.
   *  @param[in] newPosition The desired position, in parent coordinates.
   *
   *  @remarks This is a helper method for Widget::setArea.
   */
  void setPosition(const vec2& newPosition);
  /*! Sets whether this widget can be the source of drag operations.
   */
  void setDraggable(bool newState);
  SignalProxy1<void, Widget&> getDestroyedSignal();
  SignalProxy1<void, Widget&> getAreaChangedSignal();
  SignalProxy2<void, Widget&, bool> getFocusChangedSignal();
  SignalProxy3<void, Widget&, input::Key, bool> getKeyPressedSignal();
  SignalProxy2<void, Widget&, uint32> getCharInputSignal();
  SignalProxy2<void, Widget&, const vec2&> getCursorMovedSignal();
  SignalProxy4<void, Widget&, const vec2&, input::Button, bool> getButtonClickedSignal();
  SignalProxy3<void, Widget&, double, double> getScrolledSignal();
  SignalProxy1<void, Widget&> getCursorEnteredSignal();
  SignalProxy1<void, Widget&> getCursorLeftSignal();
  SignalProxy2<void, Widget&, const vec2&> getDragBegunSignal();
  SignalProxy2<void, Widget&, const vec2&> getDragMovedSignal();
  SignalProxy2<void, Widget&, const vec2&> getDragEndedSignal();
protected:
  /*! Calls Widget::draw for all children of this widget.
   */
  virtual void draw() const;
  virtual void addedChild(Widget& child);
  virtual void removedChild(Widget& child);
  virtual void addedToParent(Widget& parent);
  virtual void removedFromParent(Widget& parent);
private:
  Signal1<void, Widget&> destroyedSignal;
  Signal1<void, Widget&> areaChangedSignal;
  Signal2<void, Widget&, bool> focusChangedSignal;
  Signal3<void, Widget&, input::Key, bool> keyPressedSignal;
  Signal2<void, Widget&, uint32> charInputSignal;
  Signal2<void, Widget&, const vec2&> cursorMovedSignal;
  Signal4<void, Widget&, const vec2&, input::Button, bool> buttonClickedSignal;
  Signal3<void, Widget&, double, double> scrolledSignal;
  Signal1<void, Widget&> cursorEnteredSignal;
  Signal1<void, Widget&> cursorLeftSignal;
  Signal2<void, Widget&, const vec2&> dragBegunSignal;
  Signal2<void, Widget&, const vec2&> dragMovedSignal;
  Signal2<void, Widget&, const vec2&> dragEndedSignal;
  Layer& layer;
  Widget* parent;
  WidgetList children;
  bool enabled;
  bool visible;
  bool draggable;
  Rect area;
  mutable Rect globalArea;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIWIDGET_H*/
///////////////////////////////////////////////////////////////////////
