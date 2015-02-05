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

class Layer;

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
  Widget(Layer& layer, Widget* parent = nullptr);
  /*! Destructor.
   */
  ~Widget();
  /*! Destroys all children, recursively, of this widget.
   */
  void destroyChildren();
  /*! Searches for a widget at the specified point.
   *  @param[in] point The point at which to search.
   *  @return The widget at the specified point, or @c nullptr if no matching
   *  widget was found.
   *
   *  @remarks The point is in parent coordinates.  If this is a top-level
   *  widget, it is in global coordinates.
   */
  Widget* findByPoint(vec2 point);
  /*! Transforms the specified point from global into local (client
   *  area) coordinates.
   *  @param[in] globalPoint The global coordinate point to transform.
   *  @return The corresponding local coordinate point.
   */
  vec2 transformToLocal(vec2 globalPoint) const;
  /*! Transforms the specified point from local (client area) into
   *  global coordinates.
   *  @param[in] localPoint The local coordinate point to transform.
   *  @return The corresponding global coordinate point.
   */
  vec2 transformToGlobal(vec2 localPoint) const;
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
  bool isFocusable() const { return m_focusable; }
  /*! @return @c true if this widget is directly under the cursor, otherwise @c
   *  false.
   */
  bool isUnderCursor() const;
  /*! @return @c true if this widget supports dragging, otherwise @c false.
   */
  bool isDraggable() const { return m_draggable; }
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
  Layer& layer() const { return m_layer; }
  /*! @return The parent of this widget, or @c nullptr if it has no parent.
   */
  Widget* parent() const { return m_parent; }
  /*! @return The child widgets of this widget.
   */
  const std::vector<Widget*>& children() const { return m_children; }
  WidgetState state() const;
  /*! @return The width of this widget.
   */
  float width() const { return m_area.size.x; }
  /*! @return The height of this widget.
   */
  float height() const { return m_area.size.y; }
  /*! @return The area of this widget, in parent coordinates.
   */
  Rect area() const { return m_area; }
  /*! @return The area of this widget, in global coordinates.
   */
  Rect globalArea() const;
  /*! @return The position of this widget, in global coordinates.
   */
  vec2 globalPos() const;
  /*! Sets the area of this widget.
   *  @param[in] newArea The desired area, in parent coordinates.
   */
  void setArea(const Rect& newArea);
  /*! @return The size of this widget.
   */
  vec2 size() const { return m_area.size; }
  /*! Sets the size of this widget.
   *  @param[in] newSize The desired size, in parent coordinates.
   *
   *  @remarks This is a helper method for Widget::setArea.
   */
  void setSize(vec2 newSize);
  /*! Sets the position of this widget.
   *  @param[in] newPosition The desired position, in parent coordinates.
   *
   *  @remarks This is a helper method for Widget::setArea.
   */
  void setPosition(vec2 newPosition);
  vec2 desiredSize() const { return m_desired; }
  void setDesiredSize(vec2 newSize);
  void setFocusable(bool focusable);
  /*! Sets whether this widget can be the source of drag operations.
   */
  void setDraggable(bool newState);
  SignalProxy<void,Widget&> destroyed() { return m_destroyed; }
  SignalProxy<void,Widget&> areaChanged() { return m_areaChanged; }
  SignalProxy<void,Widget&,bool> focusChanged() { return m_focusChanged; }
  SignalProxy<void,Widget&,Key,Action,uint> keyInput() { return m_keyInput; }
  SignalProxy<void,Widget&,uint32> characterInput() { return m_characterInput; }
  SignalProxy<void,Widget&,vec2,MouseButton,Action,uint> mouseButtonInput() { return m_mouseButtonInput; }
  SignalProxy<void,Widget&,vec2> scrolled() { return m_scrolled; }
  SignalProxy<void,Widget&,vec2> cursorMoved() { return m_cursorMoved; }
  SignalProxy<void,Widget&> cursorEntered() { return m_cursorEntered; }
  SignalProxy<void,Widget&> cursorLeft() { return m_cursorLeft; }
  SignalProxy<void,Widget&,vec2,MouseButton> dragBegun() { return m_dragBegun; }
  SignalProxy<void,Widget&,vec2,MouseButton> dragMoved() { return m_dragMoved; }
  SignalProxy<void,Widget&,vec2,MouseButton> dragEnded() { return m_dragEnded; }
protected:
  /*! Calls Widget::draw for all children of this widget.
   */
  virtual void draw() const;
  virtual void onChildAdded(Widget& child);
  virtual void onChildRemoved(Widget& child);
  virtual void onChildDesiredSizeChanged(Widget& child);
  virtual void onAreaChanged();
  virtual void onFocusChanged(bool activated);
  virtual void onKey(Key key, Action action, uint mods);
  virtual void onCharacter(uint32 codepoint);
  virtual void onMouseButton(vec2 point,
                               MouseButton button,
                               Action action,
                               uint mods);
  virtual void onScroll(vec2 offset);
  virtual void onCursorPos(vec2 point);
  virtual void onCursorEntered();
  virtual void onCursorLeft();
  virtual void onDragBegun(vec2 point, MouseButton button);
  virtual void onDragMoved(vec2 point, MouseButton button);
  virtual void onDragEnded(vec2 point, MouseButton button);
private:
  Signal<void,Widget&> m_destroyed;
  Signal<void,Widget&> m_areaChanged;
  Signal<void,Widget&,bool> m_focusChanged;
  Signal<void,Widget&,Key,Action,uint> m_keyInput;
  Signal<void,Widget&,uint32> m_characterInput;
  Signal<void,Widget&,vec2,MouseButton,Action,uint> m_mouseButtonInput;
  Signal<void,Widget&,vec2> m_scrolled;
  Signal<void,Widget&,vec2> m_cursorMoved;
  Signal<void,Widget&> m_cursorEntered;
  Signal<void,Widget&> m_cursorLeft;
  Signal<void,Widget&,vec2,MouseButton> m_dragBegun;
  Signal<void,Widget&,vec2,MouseButton> m_dragMoved;
  Signal<void,Widget&,vec2,MouseButton> m_dragEnded;
  Layer& m_layer;
  Widget* m_parent;
  std::vector<Widget*> m_children;
  bool m_enabled;
  bool m_visible;
  bool m_draggable;
  bool m_focusable;
  Rect m_area;
  vec2 m_desired;
};

} /*namespace wendy*/

