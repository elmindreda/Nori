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

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Menu::Menu(Layer& layer):
  Widget(layer),
  selection(NO_ITEM)
{
  getCursorMovedSignal().connect(*this, &Menu::onCursorMoved);
  getCursorLeftSignal().connect(*this, &Menu::onCursorLeft);
  getButtonClickedSignal().connect(*this, &Menu::onButtonClicked);
  getKeyPressedSignal().connect(*this, &Menu::onKeyPressed);
  getDragEndedSignal().connect(*this, &Menu::onDragEnded);
  getFocusChangedSignal().connect(*this, &Menu::onFocusChanged);

  setDraggable(true);
  hide();
}

Menu::~Menu()
{
  destroyItems();
}

void Menu::display()
{
  selection = NO_ITEM;
  show();
  bringToFront();
  activate();
}

void Menu::addItem(Item& item)
{
  ItemList::iterator i = std::find(items.begin(), items.end(), &item);
  if (i != items.end())
    return;

  items.push_back(&item);

  sizeToFit();
}

void Menu::addItemAt(Item& item, unsigned int index)
{
  ItemList::iterator i = std::find(items.begin(), items.end(), &item);
  if (i != items.end())
    return;

  i = items.begin();
  std::advance(i, min(index, (unsigned int) items.size()));
  items.insert(i, &item);

  sizeToFit();
}

void Menu::createItem(const char* value, ItemID ID)
{
  Item* item = new Item(getLayer(), value, ID);
  addItem(*item);
}

void Menu::createSeparatorItem()
{
  Item* item = new SeparatorItem(getLayer());
  addItem(*item);
}

void Menu::destroyItem(Item& item)
{
  ItemList::iterator i = std::find(items.begin(), items.end(), &item);
  if (i != items.end())
  {
    delete *i;
    items.erase(i);

    sizeToFit();
  }
}

void Menu::destroyItem(const char* value)
{
  for (ItemList::iterator i = items.begin();  i != items.end();  i++)
  {
    if ((*i)->asString() == value)
    {
      delete *i;
      items.erase(i);

      sizeToFit();
      break;
    }
  }
}

void Menu::destroyItems()
{
  while (!items.empty())
  {
    delete items.back();
    items.pop_back();
  }

  sizeToFit();
}

void Menu::sortItems()
{
  ItemComparator comparator;
  std::sort(items.begin(), items.end(), comparator);
}

unsigned int Menu::getItemCount() const
{
  return (unsigned int) items.size();
}

Item* Menu::getItem(unsigned int index)
{
  return items[index];
}

const Item* Menu::getItem(unsigned int index) const
{
  return items[index];
}

SignalProxy2<void, Menu&, unsigned int> Menu::getItemSelectedSignal()
{
  return itemSelectedSignal;
}

void Menu::draw() const
{
  const Rect& area = getGlobalArea();

  Drawer& drawer = getLayer().getDrawer();
  if (drawer.pushClipArea(area))
  {
    drawer.drawFrame(area, getState());

    float itemTop = area.size.y - 1.f;

    unsigned int index = 0;

    for (ItemList::const_iterator i = items.begin();  i != items.end();  i++)
    {
      float height = (*i)->getHeight();
      if (height + itemTop < 0.f)
        break;

      Rect itemArea = area;
      itemArea.position.y += itemTop - height;
      itemArea.size.y = height;

      (*i)->draw(itemArea, index == selection ? STATE_SELECTED : STATE_NORMAL);

      itemTop -= height;
      index++;
    }

    Widget::draw();

    drawer.popClipArea();
  }
}

void Menu::onFocusChanged(Widget& widget, bool activated)
{
  if (!activated)
    hide();
}

void Menu::onCursorMoved(Widget& widget, const vec2& position)
{
  vec2 localPosition = transformToLocal(position);

  unsigned int index = 0;

  const float height = getHeight() - 2.f;
  float itemTop = height;

  for (ItemList::const_iterator i = items.begin();  i != items.end();  i++)
  {
    const float itemHeight = (*i)->getHeight();
    if (itemTop - itemHeight < 0.f)
      break;

    if (itemTop - itemHeight <= localPosition.y)
    {
      selection = index;
      invalidate();
      return;
    }

    itemTop -= itemHeight;
    index++;
  }
}

void Menu::onCursorLeft(Widget& widget)
{
  selection = NO_ITEM;
}

void Menu::onButtonClicked(Widget& widget,
                           const vec2& position,
                           input::Button button,
                           bool clicked)
{
  if (clicked)
    return;

  vec2 localPosition = transformToLocal(position);

  unsigned int index = 0;

  const float height = getHeight() - 2.f;
  float itemTop = height;

  for (ItemList::const_iterator i = items.begin();  i != items.end();  i++)
  {
    const float itemHeight = (*i)->getHeight();
    if (itemTop - itemHeight < 0.f)
      break;

    if (itemTop - itemHeight <= localPosition.y)
    {
      itemSelectedSignal(*this, index);
      hide();
      return;
    }

    itemTop -= itemHeight;
    index++;
  }
}

void Menu::onKeyPressed(Widget& widget, input::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case input::KEY_UP:
    {
      if (selection > 0)
        selection--;
      else
        selection = items.size() - 1;

      invalidate();
      break;
    }

    case input::KEY_DOWN:
    {
      selection++;
      if (selection == items.size())
        selection = 0;

      invalidate();
      break;
    }

    case input::KEY_ENTER:
    {
      itemSelectedSignal(*this, selection);
      hide();
      break;
    }
  }
}

void Menu::onDragEnded(Widget& widget, const vec2& position)
{
  vec2 localPosition = transformToLocal(position);

  if (!getArea().contains(localPosition))
    hide();
}

void Menu::sizeToFit()
{
  vec2 size(0.f, 2.f);

  for (ItemList::const_iterator i = items.begin();  i != items.end();  i++)
  {
    size.x = max((*i)->getWidth(), size.x);
    size.y += (*i)->getHeight();
  }

  setSize(size);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
