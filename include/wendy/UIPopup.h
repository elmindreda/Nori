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

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class Popup : public Widget
{
public:
  Popup(void);
  void addItem(Item& item);
  void removeItem(Item& item);
  void destroyItems(void);
  unsigned int getSelection(void) const;
  void setSelection(unsigned int newIndex);
  unsigned int getItemCount(void) const;
  Item* getItem(unsigned int index);
  const Item* getItem(unsigned int index) const;
  SignalProxy2<void, Popup&, unsigned int> getItemSelectedSignal(void);
protected:
  void draw(void) const;
private:
  void display(void);
  void onButtonClicked(Widget& widget,
		       const Vector2& position,
		       unsigned int button,
		       bool clicked);
  void onKeyPressed(Widget& widget, GL::Key key, bool pressed);
  void onItemSelected(Menu& menu, unsigned int index);
  Signal2<void, Popup&, unsigned int> itemSelectedSignal;
  unsigned int selection;
  Ptr<Menu> menu;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIPOPUP_H*/
///////////////////////////////////////////////////////////////////////
