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

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class List : public Widget
{
public:
  List(void);
  ~List(void);
  void insertItem(Item* item, unsigned int index);
  void destroyItem(Item* item);
  void destroyItems(void);
  void sortItems(void);
  bool isItemVisible(const Item* item) const;
  unsigned int getOffset(void) const;
  void setOffset(unsigned int newOffset);
  unsigned int getSelection(void) const;
  Item* getSelectedItem(void) const;
  void setSelection(unsigned int newIndex);
  void setSelectedItem(Item* newItem);
  unsigned int getItemCount(void) const;
  Item* getItem(unsigned int index);
  const Item* getItem(unsigned int index) const;
  SignalProxy1<void, List&> getSelectionChangedSignal(void);
protected:
  void draw(void) const;
private:
  void onButtonClicked(Widget& widget,
		       const Vector2& position,
		       unsigned int button,
		       bool clicked);
  void onKeyPressed(Widget& widget, GL::Key key, bool pressed);
  void onWheelTurned(Widget& widget, int wheelOffset);
  void setSelection(unsigned int newIndex, bool notify);
  Signal1<void, List&> selectionChangedSignal;
  ItemList items;
  unsigned int offset;
  unsigned int selection;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UILIST_H*/
///////////////////////////////////////////////////////////////////////
