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

#include <wendy/Config.h>

#include <wendy/UIDrawer.h>
#include <wendy/UILayer.h>
#include <wendy/UIWidget.h>
#include <wendy/UIItem.h>
#include <wendy/UIMenu.h>
#include <wendy/UIPopup.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Popup::Popup(Layer& layer):
  Widget(layer),
  selection(0)
{
  const float em = layer.getDrawer().getCurrentEM();

  setSize(vec2(em * 10.f, em * 2.f));

  getKeyPressedSignal().connect(*this, &Popup::onKeyPressed);
  getButtonClickedSignal().connect(*this, &Popup::onButtonClicked);

  menu = new Menu(layer);
  menu->getItemSelectedSignal().connect(*this, &Popup::onItemSelected);
  layer.addRootWidget(*menu);
}

void Popup::addItem(Item& item)
{
  menu->addItem(item);
}

void Popup::createItem(const char* value, ItemID ID)
{
  Item* item = new Item(getLayer(), value, ID);
  menu->addItem(*item);
}

void Popup::destroyItem(Item& item)
{
  menu->destroyItem(item);
  setSelection(selection);
}

void Popup::destroyItems()
{
  menu->destroyItems();
  selection = 0;
}

unsigned int Popup::getSelection() const
{
  return selection;
}

void Popup::setSelection(unsigned int newIndex)
{
  if (menu->getItemCount())
    selection = min(newIndex, menu->getItemCount() - 1);
  else
    selection = 0;
}

unsigned int Popup::getItemCount() const
{
  return menu->getItemCount();
}

Item* Popup::getItem(unsigned int index)
{
  return menu->getItem(index);
}

const Item* Popup::getItem(unsigned int index) const
{
  return menu->getItem(index);
}

String Popup::getItemValue(unsigned int index) const
{
  if (const Item* item = menu->getItem(index))
    return item->asString();

  return String();
}

SignalProxy2<void, Popup&, unsigned int> Popup::getItemSelectedSignal()
{
  return itemSelectedSignal;
}

void Popup::draw() const
{
  const Rect& area = getGlobalArea();

  Drawer& drawer = getLayer().getDrawer();
  if (drawer.pushClipArea(area))
  {
    drawer.drawFrame(area, getState());

    if (selection < menu->getItemCount())
    {
      const Item* item = menu->getItem(selection);

      const float em = drawer.getCurrentEM();

      Rect textArea = area;
      textArea.position.x += em / 2.f;
      textArea.size.x -= em;

      drawer.drawText(textArea, item->asString().c_str(), LEFT_ALIGNED);
    }

    Widget::draw();

    drawer.popClipArea();
  }
}

void Popup::display()
{
  const float width = max(menu->getWidth(), getWidth());
  menu->setArea(Rect(getGlobalArea().position,
                     vec2(width, menu->getHeight())));
  menu->display();
}

void Popup::onButtonClicked(Widget& widget,
                            const vec2& position,
                            input::Button button,
                            bool clicked)
{
  if (!clicked)
    return;

  display();
}

void Popup::onKeyPressed(Widget& widget, input::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case input::KEY_ENTER:
    {
      display();
      break;
    }
  }
}

void Popup::onItemSelected(Menu& menu, unsigned int index)
{
  selection = index;
  itemSelectedSignal(*this, selection);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
