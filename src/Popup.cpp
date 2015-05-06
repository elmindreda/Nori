///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

#include <nori/Config.hpp>

#include <nori/Drawer.hpp>
#include <nori/Layer.hpp>
#include <nori/Widget.hpp>
#include <nori/Item.hpp>
#include <nori/Menu.hpp>
#include <nori/Popup.hpp>

namespace nori
{

Popup::Popup(Layer& layer, Widget* parent):
  Widget(layer, parent),
  m_selection(NO_ITEM),
  m_menu(nullptr)
{
  const float em = layer.drawer().theme().em();
  setDesiredSize(vec2(em * 10.f, em * 2.f));

  m_menu = new Menu(layer);
  m_menu->itemSelected().connect(*this, &Popup::onItemSelected);
  m_menu->destroyed().connect(*this, &Popup::onMenuDestroyed);

  setFocusable(true);
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

void Popup::insertItem(Item& item, uint index)
{
  m_menu->insertItem(item, index);

  if (m_selection == NO_ITEM)
    m_selection = 0;
}

void Popup::createItem(const std::string& value, ItemID ID)
{
  Item* item = new Item(layer(), value, ID);
  addItem(*item);
}

Item* Popup::findItem(const std::string& value)
{
  return m_menu->findItem(value);
}

const Item* Popup::findItem(const std::string& value) const
{
  return m_menu->findItem(value);
}

void Popup::sortItems()
{
  m_menu->sortItems();
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

void Popup::setSelection(uint newIndex)
{
  if (m_menu->items().empty())
    m_selection = NO_ITEM;
  else
    m_selection = min(newIndex, uint(m_menu->items().size()) - 1);
}

Item* Popup::selectedItem()
{
  if (m_selection == NO_ITEM)
    return nullptr;

  return m_menu->items().at(m_selection);
}

void Popup::setSelectedItem(Item& newItem)
{
  const auto& items = m_menu->items();
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
  for (Item* i : m_menu->items())
  {
    if (i->id() == newItemID)
    {
      setSelectedItem(*i);
      break;
    }
  }
}

void Popup::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    WidgetState buttonState = state();
    if (m_menu->isVisible())
      buttonState = STATE_SELECTED;

    drawer.setFont(nullptr);
    drawer.drawButton(area, buttonState);

    if (m_selection != NO_ITEM)
    {
      const Item* item = m_menu->items().at(m_selection);
      const float em = drawer.theme().em();
      const Rect textArea(area.position + vec2(em / 2.f, 0.f),
                          area.size - vec2(em, 0.f));

      drawer.drawText(textArea, item->value().c_str(), LEFT_ALIGNED, buttonState);
    }

    Widget::draw();
    drawer.popClipArea();
  }
}

void Popup::display()
{
  m_menu->setSize(vec2(max(m_menu->width(), width()), m_menu->height()));

  const Rect area = globalArea();

  if (area.position.y - m_menu->height() > 0.f)
    m_menu->setPosition(area.position - vec2(0.f, m_menu->height()));
  else if (area.position.y + area.size.y + m_menu->height() < layer().window().height())
    m_menu->setPosition(area.position + area.size);
  else
    m_menu->setPosition(vec2(area.position.x, 1.f));

  m_menu->display();
}

void Popup::onMouseButton(vec2 point,
                          MouseButton button,
                          Action action,
                          uint mods)
{
  if (button == MOUSE_BUTTON_LEFT && action == PRESSED)
  {
    if (m_menu->isVisible())
      m_menu->hide();
    else
      display();
  }

  Widget::onMouseButton(point, button, action, mods);
}

void Popup::onKey(Key key, Action action, uint mods)
{
  if ((action == PRESSED || action == REPEATED) && mods == MOD_NONE)
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
  m_itemSelected(*this, m_selection);
  activate();
}

void Popup::onMenuDestroyed(Widget& widget)
{
  m_menu = nullptr;
}

} /*namespace nori*/

