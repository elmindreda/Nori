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
#ifndef WENDY_UILAYER_H
#define WENDY_UILAYER_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Input.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

class Widget;
class LayerStack;

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
typedef std::vector<Widget*> WidgetList;

///////////////////////////////////////////////////////////////////////

/*! @brief Root object for widgets.
 *  @ingroup ui
 */
class Layer : public input::Target, public Trackable, public RefObject
{
  friend class Widget;
  friend class LayerStack;
public:
  /*! Constructor.
   */
  Layer(input::Context& context, Drawer& drawer);
  /*! Destructor.
   */
  ~Layer();
  virtual void update();
  /*! Draws all visible widgets in this UI layer.
   */
  virtual void draw();
  void addRootWidget(Widget& root);
  void destroyRootWidgets();
  /*! Searches for a widget at the specified point.
   *  @param[in] point The point at which to search.
   *  @return The widget at the specified point, or @c NULL if no matching
   *  widget was found.
   *
   *  @remarks The point is in global coordinates.
   */
  Widget* findWidgetByPoint(const vec2& point);
  void captureCursor();
  void releaseCursor();
  void cancelDragging();
  void invalidate();
  bool hasCapturedCursor() const;
  Drawer& getDrawer() const;
  input::Context& getInputContext() const;
  /*! @return The root widgets of this layer.
   */
  const WidgetList& getRootWidgets() const;
  /*! @return The active widget, or @c NULL if no widget is active.
   */
  Widget* getActiveWidget();
  Widget* getDraggedWidget();
  Widget* getHoveredWidget();
  void setActiveWidget(Widget* widget);
  LayerStack* getStack() const;
private:
  void updateHoveredWidget();
  void removedWidget(Widget& widget);
  void onKeyPressed(input::Key key, bool pressed);
  void onCharInput(wchar_t character);
  void onCursorMoved(const ivec2& position);
  void onButtonClicked(input::Button button, bool clicked);
  void onWheelTurned(int offset);
  void onFocusChanged(bool activated);
  input::Context& context;
  Drawer& drawer;
  bool dragging;
  WidgetList roots;
  Widget* activeWidget;
  Widget* draggedWidget;
  Widget* hoveredWidget;
  Widget* captureWidget;
  LayerStack* stack;
};

///////////////////////////////////////////////////////////////////////

typedef Ref<Layer> LayerRef;

///////////////////////////////////////////////////////////////////////

class LayerStack
{
public:
  LayerStack(input::Context& context);
  void update() const;
  void draw() const;
  void push(Layer& layer);
  void pop();
  void empty();
  bool isEmpty() const;
private:
  typedef std::vector<LayerRef> LayerList;
  input::Context& context;
  LayerList layers;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UILAYER_H*/
///////////////////////////////////////////////////////////////////////
