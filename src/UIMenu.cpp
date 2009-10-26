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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>

#include <wendy/RenderMaterial.h>
#include <wendy/RenderFont.h>

#include <wendy/Input.h>

#include <wendy/UIRender.h>
#include <wendy/UIDesktop.h>
#include <wendy/UIWidget.h>
#include <wendy/UIItem.h>
#include <wendy/UIMenu.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{

struct ItemComparator
{
  inline bool operator () (const Item* x, const Item* y)
  {
    return *x < *y;
  }
};

}

///////////////////////////////////////////////////////////////////////

Menu::Menu(void):
  selection(0)
{
  getCursorMovedSignal().connect(*this, &Menu::onCursorMoved);
  getButtonClickedSignal().connect(*this, &Menu::onButtonClicked);
  getKeyPressedSignal().connect(*this, &Menu::onKeyPressed);
  getDragEndedSignal().connect(*this, &Menu::onDragEnded);
  getFocusChangedSignal().connect(*this, &Menu::onFocusChanged);

  setDraggable(true);
  setVisible(false);
}

Menu::~Menu(void)
{
  destroyItems();
}

void Menu::display(Desktop& desktop)
{
  if (getDesktop() != &desktop)
    desktop.addRootWidget(*this);

  selection = 0;
  setVisible(true);
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
  std::advance(i, std::min(index, (unsigned int) items.size()));
  items.insert(i, &item);

  sizeToFit();
}

void Menu::removeItem(Item& item)
{
  ItemList::iterator i = std::find(items.begin(), items.end(), &item);
  if (i != items.end())
  {
    delete *i;
    items.erase(i);

    sizeToFit();
  }
}

void Menu::destroyItems(void)
{
  while (!items.empty())
  {
    delete items.back();
    items.pop_back();
  }

  sizeToFit();
}

void Menu::sortItems(void)
{
  ItemComparator comparator;
  std::sort(items.begin(), items.end(), comparator);
}

unsigned int Menu::getItemCount(void) const
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

SignalProxy2<void, Menu&, unsigned int> Menu::getItemSelectedSignal(void)
{
  return itemSelectedSignal;
}

void Menu::draw(void) const
{
  const Rect& area = getGlobalArea();

  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawFrame(area, getState());

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

    renderer->popClipArea();
  }
}

void Menu::onFocusChanged(Widget& widget, bool activated)
{
  if (!activated)
    setVisible(false);
}

void Menu::onCursorMoved(Widget& widget, const Vec2& position)
{
  Vec2 localPosition = transformToLocal(position);

  unsigned int index = 0;

  const float height = getArea().size.y - 2.f;
  float itemTop = height;

  for (ItemList::const_iterator i = items.begin();  i != items.end();  i++)
  {
    const float itemHeight = (*i)->getHeight();
    if (itemTop - itemHeight < 0.f)
      break;

    if (itemTop - itemHeight <= localPosition.y)
    {
      selection = index;
      return;
    }

    itemTop -= itemHeight;
    index++;
  }
}

void Menu::onButtonClicked(Widget& widget,
                           const Vec2& position,
			   input::Button button,
			   bool clicked)
{
  if (clicked)
    return;

  Vec2 localPosition = transformToLocal(position);

  unsigned int index = 0;

  const float height = getArea().size.y - 2.f;
  float itemTop = height;

  for (ItemList::const_iterator i = items.begin();  i != items.end();  i++)
  {
    const float itemHeight = (*i)->getHeight();
    if (itemTop - itemHeight < 0.f)
      break;

    if (itemTop - itemHeight <= localPosition.y)
    {
      itemSelectedSignal.emit(*this, index);
      setVisible(false);
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
    case input::Key::UP:
    {
      if (selection > 0)
	selection--;
      else
	selection = items.size() - 1;
      break;
    }

    case input::Key::DOWN:
    {
      selection++;
      if (selection == items.size())
	selection = 0;
      break;
    }

    case input::Key::ENTER:
    {
      itemSelectedSignal.emit(*this, selection);
      setVisible(false);
      break;
    }
  }
}

void Menu::onDragEnded(Widget& widget, const Vec2& position)
{
  Vec2 localPosition = transformToLocal(position);

  if (!getArea().contains(localPosition))
    setVisible(false);
}

void Menu::sizeToFit(void)
{
  Vec2 size(0.f, 2.f);

  for (ItemList::const_iterator i = items.begin();  i != items.end();  i++)
  {
    size.x = std::max((*i)->getWidth(), size.x);
    size.y += (*i)->getHeight();
  }

  setSize(size);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
