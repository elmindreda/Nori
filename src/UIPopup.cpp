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

#include <wendy/Config.hpp>

#include <wendy/UIDrawer.hpp>
#include <wendy/UILayer.hpp>
#include <wendy/UIWidget.hpp>
#include <wendy/UIItem.hpp>
#include <wendy/UIMenu.hpp>
#include <wendy/UIPopup.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Popup::Popup(Layer& layer):
  Widget(layer),
  m_selection(NO_ITEM),
  m_menu(nullptr)
{
  const float em = layer.drawer().currentEM();

  setSize(vec2(em * 10.f, em * 2.f));

  m_menu = new Menu(layer);
  m_menu->itemSelectedSignal().connect(*this, &Popup::onItemSelected);
  m_menu->destroyedSignal().connect(*this, &Popup::onMenuDestroyed);
  layer.addRootWidget(*m_menu);
}

Popup::~Popup()
{
  if (m_menu)
    delete m_menu;
}

void Popup::addItem(Item& item)
{
  m_menu->addItem(item);

  if (m_selection == NO_ITEM)
    m_selection = 0;
}

void Popup::createItem(const char* value, ItemID ID)
{
  Item* item = new Item(layer(), value, ID);
  addItem(*item);
}

Item* Popup::findItem(const char* value)
{
  return m_menu->findItem(value);
}

const Item* Popup::findItem(const char* value) const
{
  return m_menu->findItem(value);
}

void Popup::destroyItem(Item& item)
{
  m_menu->destroyItem(item);
  setSelection(m_selection);
}

void Popup::destroyItems()
{
  m_menu->destroyItems();
  m_selection = NO_ITEM;
}

uint Popup::selection() const
{
  return m_selection;
}

void Popup::setSelection(uint newIndex)
{
  if (m_menu->itemCount())
    m_selection = min(newIndex, m_menu->itemCount() - 1);
  else
    m_selection = NO_ITEM;
}

Item* Popup::selectedItem()
{
  if (m_selection == NO_ITEM)
    return nullptr;

  return m_menu->item(m_selection);
}

void Popup::setSelectedItem(Item& newItem)
{
  const std::vector<Item*>& items = m_menu->items();

  auto i = std::find(items.begin(), items.end(), &newItem);
  assert(i != items.end());

  m_selection = uint(i - items.begin());
}

ItemID Popup::selectedID()
{
  if (Item* item = selectedItem())
    return item->id();

  return NO_ITEM;
}

void Popup::setSelectedID(ItemID newItemID)
{
  for (auto i : m_menu->items())
  {
    if (i->id() == newItemID)
    {
      setSelectedItem(*i);
      break;
    }
  }
}

uint Popup::itemCount() const
{
  return m_menu->itemCount();
}

Item* Popup::item(uint index)
{
  return m_menu->item(index);
}

const Item* Popup::item(uint index) const
{
  return m_menu->item(index);
}

const std::vector<Item*>& Popup::items() const
{
  return m_menu->items();
}

SignalProxy<void, Popup&, uint> Popup::itemSelectedSignal()
{
  return m_itemSelectedSignal;
}

void Popup::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    drawer.drawFrame(area, state());

    if (m_selection != NO_ITEM)
    {
      const Item* item = m_menu->item(m_selection);
      const float em = drawer.currentEM();
      const Rect textArea(area.position + vec2(em / 2.f, 0.f),
                          area.size - vec2(em, 0.f));

      drawer.drawText(textArea, item->value().c_str(), LEFT_ALIGNED, state());
    }

    Widget::draw();

    drawer.popClipArea();
  }
}

void Popup::display()
{
  m_menu->setArea(Rect(globalPos(),
                       vec2(max(m_menu->width(), width()), m_menu->height())));
  m_menu->display();
}

void Popup::onMouseButton(vec2 point,
                          MouseButton button,
                          Action action,
                          uint mods)
{
  if (action == PRESSED)
    display();

  Widget::onMouseButton(point, button, action, mods);
}

void Popup::onKey(Key key, Action action, uint mods)
{
  if (action == PRESSED)
  {
    switch (key)
    {
      case KEY_ENTER:
      {
        display();
        break;
      }

      default:
        break;
    }
  }

  Widget::onKey(key, action, mods);
}

void Popup::onItemSelected(Menu& menu, uint index)
{
  m_selection = index;
  m_itemSelectedSignal(*this, m_selection);
  activate();
}

void Popup::onMenuDestroyed(Widget& widget)
{
  m_menu = nullptr;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
