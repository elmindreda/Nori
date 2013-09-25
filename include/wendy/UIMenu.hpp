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
#ifndef WENDY_UIMENU_HPP
#define WENDY_UIMENU_HPP
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
  Menu(Layer& layer);
  ~Menu();
  void display(const vec2& point);
  void display();
  void addItem(Item& item);
  void addItemAt(Item& item, uint index);
  void createItem(const char* value, ItemID ID = 0);
  void createSeparatorItem();
  Item* findItem(const char* value);
  const Item* findItem(const char* value) const;
  void destroyItem(Item& item);
  void destroyItems();
  void sortItems();
  uint getItemCount() const;
  Item* getItem(uint index);
  const Item* getItem(uint index) const;
  const ItemList& getItems() const;
  SignalProxy2<void, Menu&, uint> getItemSelectedSignal();
private:
  void draw() const;
  void onFocusChanged(Widget& widget, bool activated);
  void onCursorPos(Widget& widget, vec2 position);
  void onCursorLeft(Widget& widget);
  void onMouseButton(Widget& widget,
                     vec2 position,
                     MouseButton button,
                     Action action);
  void onKey(Widget& widget, Key key, Action action);
  void onDragEnded(Widget& widget, vec2 position);
  void sizeToFit();
  ItemList items;
  uint selection;
  Signal2<void, Menu&, uint> itemSelectedSignal;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIMENU_HPP*/
///////////////////////////////////////////////////////////////////////
