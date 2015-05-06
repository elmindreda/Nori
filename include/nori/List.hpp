///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

namespace nori
{

/*! @ingroup ui
 */
class List : public Widget, public ItemContainer
{
public:
  List(Layer& layer, Widget* parent = nullptr);
  ~List();
  void addItem(Item& item) override;
  void insertItem(Item& item, uint index) override;
  void createItem(const std::string& value, ItemID id = 0);
  Item* findItem(const std::string& value) override;
  const Item* findItem(const std::string& value) const override;
  void destroyItem(Item& item) override;
  void destroyItems() override;
  void sortItems() override;
  bool isEditable() const { return m_editable; }
  void setEditable(bool newState);
  uint offset() const { return m_offset; }
  void setOffset(uint newOffset);
  uint selection() const { return m_selection; }
  void setSelection(uint newSelection);
  Item* selectedItem();
  void setSelectedItem(Item& newItem);
  ItemID selectedID();
  void setSelectedID(ItemID newItemID);
  const std::vector<Item*>& items() const override { return m_items; }
  SignalProxy<void,List&> itemSelected() { return m_itemSelected; }
  SignalProxy<void,List&,const std::string&> itemEdited() { return m_itemEdited; }
protected:
  void draw() const;
private:
  void onAreaChanged() override;
  void onMouseButton(vec2 point,
                     MouseButton button,
                     Action action,
                     uint mods) override;
  void onKey(Key key, Action action, uint mods) override;
  void onScroll(vec2 offset) override;
  void onEntryFocusChanged(Widget& widget, bool activated);
  void onEntryKey(Widget& widget, Key key, Action action, uint mods);
  void onEntryDestroyed(Widget& widget);
  void onValueChanged(Scroller& scroller);
  void beginEditing();
  void applyEditing();
  void cancelEditing();
  void updateScroller();
  bool isSelectionVisible() const;
  void setSelection(uint newSelection, bool notify);
  Signal<void,List&> m_itemSelected;
  Signal<void,List&,const std::string&> m_itemEdited;
  bool m_editable;
  std::vector<Item*> m_items;
  uint m_offset;
  uint m_maxOffset;
  uint m_selection;
  Scroller* m_scroller;
  Entry* m_entry;
};

} /*namespace nori*/

