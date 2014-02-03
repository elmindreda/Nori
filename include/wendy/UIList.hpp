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
#ifndef WENDY_UILIST_HPP
#define WENDY_UILIST_HPP
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class List : public Widget
{
public:
  List(Layer& layer);
  ~List();
  void addItem(Item& item);
  void createItem(const char* value, ItemID ID = 0);
  Item* findItem(const char* value);
  const Item* findItem(const char* value) const;
  void destroyItem(Item& item);
  void destroyItems();
  void sortItems();
  bool isEditable() const;
  void setEditable(bool newState);
  uint offset() const;
  void setOffset(uint newOffset);
  uint selection() const;
  void setSelection(uint newSelection);
  Item* selectedItem();
  void setSelectedItem(Item& newItem);
  uint itemCount() const;
  Item* item(uint index);
  const Item* item(uint index) const;
  const std::vector<Item*>& items() const;
  SignalProxy<void, List&> itemSelectedSignal();
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
  Signal<void, List&> m_itemSelectedSignal;
  bool m_editable;
  bool m_editing;
  std::vector<Item*> m_items;
  uint m_offset;
  uint m_maxOffset;
  uint m_selection;
  Scroller* m_scroller;
  Entry* m_entry;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UILIST_HPP*/
///////////////////////////////////////////////////////////////////////
