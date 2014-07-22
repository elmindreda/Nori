///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#pragma once

#include <wendy/Window.hpp>

namespace wendy
{

class Widget;
class LayerStack;

/*! @brief Root object for widgets.
 *  @ingroup ui
 */
class Layer : public EventTarget, public Trackable, public RefObject
{
  friend class Widget;
  friend class LayerStack;
public:
  /*! Constructor.
   */
  Layer(Window& window, Drawer& drawer);
  /*! Destructor.
   */
  ~Layer();
  virtual void update();
  /*! Draws all visible widgets in this UI layer.
   */
  virtual void draw();
  void destroyRootWidgets();
  /*! Searches for a widget at the specified point.
   *  @param[in] point The point at which to search.
   *  @return The widget at the specified point, or @c nullptr if no matching
   *  widget was found.
   *
   *  @remarks The point is in global coordinates.
   */
  Widget* findWidgetByPoint(vec2 point);
  void captureCursor();
  void releaseCursor();
  void cancelDragging();
  void activatePrevWidget();
  void activateNextWidget();
  void invalidate();
  virtual bool isOpaque() const;
  bool hasCapturedCursor() const;
  vec2 cursorPoint() const;
  Drawer& drawer() const { return m_drawer; }
  Window& window() const { return m_window; }
  /*! @return The root widgets of this layer.
   */
  const std::vector<Widget*>& roots() const { return m_roots; }
  /*! @return The active widget, or @c nullptr if no widget is active.
   */
  Widget* activeWidget() { return m_activeWidget; }
  Widget* draggedWidget() { return m_draggedWidget; }
  Widget* hoveredWidget() { return m_hoveredWidget; }
  void setActiveWidget(Widget* widget);
  LayerStack* stack() const { return m_stack; }
  SignalProxy<void, Layer&> sizeChangedSignal();
protected:
  void onWindowSize(uint width, uint height) override;
  void onKey(Key key, Action action, uint mods) override;
  void onCharacter(uint32 codepoint) override;
  void onCursorPos(vec2 point) override;
  void onMouseButton(MouseButton button, Action action, uint mods) override;
  void onScroll(vec2 offset) override;
  void onFocus(bool activated) override;
private:
  void updateHoveredWidget();
  void activateWidget(int offset);
  void removeWidget(Widget& widget);
  void focusableWidgets(std::vector<Widget*>& target,
                        const std::vector<Widget*>& source) const;
  Window& m_window;
  Drawer& m_drawer;
  bool m_dragging;
  MouseButton m_dragButton;
  std::vector<Widget*> m_roots;
  Widget* m_activeWidget;
  Widget* m_draggedWidget;
  Widget* m_hoveredWidget;
  Widget* m_captureWidget;
  LayerStack* m_stack;
  Signal<void, Layer&> m_sizeChangedSignal;
};

class LayerStack
{
public:
  LayerStack(Window& window);
  void update() const;
  void draw() const;
  void push(Layer& layer);
  void pop();
  void empty();
  bool isEmpty() const;
  Layer* top() const;
private:
  Window& m_window;
  std::vector<Ref<Layer>> m_layers;
};

} /*namespace wendy*/

