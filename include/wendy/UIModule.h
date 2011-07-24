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
#ifndef WENDY_UIMODULE_H
#define WENDY_UIMODULE_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Input.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

class Widget;

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
typedef std::vector<Widget*> WidgetList;

///////////////////////////////////////////////////////////////////////

/*! @brief Root object for UI objects.
 *  @ingroup ui
 */
class Module : public input::Target, public Trackable
{
  friend class Widget;
public:
  /*! Constructor.
   */
  Module(input::Context& context, Drawer& drawer);
  /*! Destructor.
   */
  ~Module(void);
  virtual void draw(void);
  void addRootWidget(Widget& root);
  /*! Draws all root level widgets.
   */
  void destroyRootWidgets(void);
  /*! Searches for a widget at the specified point.
   *  @param[in] point The point at which to search.
   *  @return The widget at the specified point, or @c NULL if no matching
   *  widget was found.
   *
   *  @remarks The point is in global coordinates.
   */
  Widget* findWidgetByPoint(const vec2& point);
  void cancelDragging(void);
  void invalidate(void);
  Drawer& getDrawer(void) const;
  /*! @return The root widgets of this module.
   */
  const WidgetList& getRootWidgets(void) const;
  /*! @return The active widget, or @c NULL if no widget is active.
   */
  Widget* getActiveWidget(void);
  Widget* getDraggedWidget(void);
  Widget* getHoveredWidget(void);
  void setActiveWidget(Widget* widget);
private:
  void updateHoveredWidget(void);
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
};

///////////////////////////////////////////////////////////////////////

class ModuleStack
{
public:
  ModuleStack(input::Context& context);
  void draw(void) const;
  void push(Module& module);
  void pop(void);
  void empty(void);
  bool isEmpty(void) const;
private:
  typedef std::vector<Module*> ModuleList;
  input::Context& context;
  ModuleList modules;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIMODULE_H*/
///////////////////////////////////////////////////////////////////////
