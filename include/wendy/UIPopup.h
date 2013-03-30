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
#ifndef WENDY_UIPOPUP_H
#define WENDY_UIPOPUP_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class Popup : public Widget
{
public:
  Popup(Layer& layer);
  ~Popup();
  void addItem(Item& item);
  void addItemAt(Item& item, uint index);
  void createItem(const char* value, ItemID ID = 0);
  void createSeparatorItem();
  Item* findItem(const char* value);
  const Item* findItem(const char* value) const;
  void destroyItem(Item& item);
  void destroyItems();
  uint getSelection() const;
  void setSelection(uint newIndex);
  Item* getSelectedItem();
  void setSelectedItem(Item& newItem);
  uint getItemCount() const;
  Item* getItem(uint index);
  const Item* getItem(uint index) const;
  const ItemList& getItems() const;
  SignalProxy2<void, Popup&, uint> getItemSelectedSignal();
protected:
  void draw() const;
private:
  void display();
  void onButtonClicked(Widget& widget,
                       const vec2& position,
                       input::Button button,
                       input::Action action);
  void onKeyPressed(Widget& widget, input::Key key, input::Action action);
  void onItemSelected(Menu& menu, uint index);
  void onMenuDestroyed(Widget& widget);
  Signal2<void, Popup&, uint> itemSelectedSignal;
  uint selection;
  Menu* menu;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIPOPUP_H*/
///////////////////////////////////////////////////////////////////////
