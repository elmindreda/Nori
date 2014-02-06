///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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
#ifndef WENDY_UIPOPUP_HPP
#define WENDY_UIPOPUP_HPP
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
  uint selection() const;
  void setSelection(uint newIndex);
  Item* selectedItem();
  void setSelectedItem(Item& newItem);
  ItemID selectedID();
  void setSelectedID(ItemID newItemID);
  uint itemCount() const;
  Item* item(uint index);
  const Item* item(uint index) const;
  const std::vector<Item*>& items() const;
  SignalProxy<void, Popup&, uint> itemSelectedSignal();
protected:
  void draw() const;
private:
  void display();
  void onMouseButton(vec2 point,
                     MouseButton button,
                     Action action,
                     uint mods) override;
  void onKey(Key key, Action action, uint mods) override;
  void onItemSelected(Menu& menu, uint index);
  void onMenuDestroyed(Widget& widget);
  Signal<void, Popup&, uint> m_itemSelectedSignal;
  uint m_selection;
  Menu* m_menu;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIPOPUP_HPP*/
///////////////////////////////////////////////////////////////////////
