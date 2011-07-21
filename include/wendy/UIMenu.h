///////////////////////////////////////////////////////////////////////
// Wendy user interface library
// Copyright (c) 2007 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_UIMENU_H
#define WENDY_UIMENU_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class Menu : public Widget
{
public:
  Menu(Module& module);
  ~Menu(void);
  void display(void);
  void addItem(Item& item);
  void addItemAt(Item& item, unsigned int index);
  void removeItem(Item& item);
  void destroyItems(void);
  void sortItems(void);
  unsigned int getItemCount(void) const;
  Item* getItem(unsigned int index);
  const Item* getItem(unsigned int index) const;
  SignalProxy2<void, Menu&, unsigned int> getItemSelectedSignal(void);
private:
  void draw(void) const;
  void onFocusChanged(Widget& widget, bool activated);
  void onCursorMoved(Widget& widget, const vec2& position);
  void onButtonClicked(Widget& widget,
                       const vec2& position,
		       input::Button button,
		       bool clicked);
  void onKeyPressed(Widget& widget, input::Key key, bool pressed);
  void onDragEnded(Widget& widget, const vec2& position);
  void sizeToFit(void);
  ItemList items;
  unsigned int selection;
  Signal2<void, Menu&, unsigned int> itemSelectedSignal;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIMENU_H*/
///////////////////////////////////////////////////////////////////////
