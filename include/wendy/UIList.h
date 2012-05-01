///////////////////////////////////////////////////////////////////////
// Wendy user interface library
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
#ifndef WENDY_UILIST_H
#define WENDY_UILIST_H
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
  unsigned int getOffset() const;
  void setOffset(unsigned int newOffset);
  unsigned int getSelection() const;
  void setSelection(unsigned int newSelection);
  Item* getSelectedItem();
  void setSelectedItem(Item& newItem);
  unsigned int getItemCount() const;
  Item* getItem(unsigned int index);
  const Item* getItem(unsigned int index) const;
  const ItemList& getItems() const;
  SignalProxy1<void, List&> getItemSelectedSignal();
protected:
  void draw() const;
private:
  void onAreaChanged(Widget& widget);
  void onButtonClicked(Widget& widget,
                       const vec2& position,
                       input::Button button,
                       bool clicked);
  void onEntryFocusChanged(Widget& widget, bool activated);
  void onEntryKeyPressed(Widget& widget, input::Key key, bool pressed);
  void onEntryDestroyed(Widget& widget);
  void onKeyPressed(Widget& widget, input::Key key, bool pressed);
  void onScrolled(Widget& widget, double x, double y);
  void onValueChanged(Scroller& scroller);
  void beginEditing();
  void applyEditing();
  void cancelEditing();
  void updateScroller();
  bool isSelectionVisible() const;
  void setSelection(unsigned int newSelection, bool notify);
  Signal1<void, List&> itemSelectedSignal;
  bool editable;
  bool editing;
  ItemList items;
  unsigned int offset;
  unsigned int maxOffset;
  unsigned int selection;
  Scroller* scroller;
  Entry* entry;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UILIST_H*/
///////////////////////////////////////////////////////////////////////
