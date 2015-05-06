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

#include <algorithm>

namespace nori
{

Menu::Menu(Layer& layer):
  Widget(layer),
  m_selection(NO_ITEM)
{
  setDraggable(true);
  setFocusable(true);
  hide();
}

Menu::~Menu()
{
  destroyItems();
}

void Menu::display(vec2 point)
{
  vec2 position;

  if (point.x + width() + 1.f < layer().window().width())
    position.x = point.x + 1;
  else if (point.x - width() - 1.f > 0.f)
    position.x = point.x - width() - 1.f;
  else
    position.x = 1.f;

  if (point.y + height() + 1.f < layer().window().height())
    position.y = point.y + 1;
  else if (point.y - height() - 1.f > 0.f)
    position.y = point.y - height() - 1.f;
  else
    position.y = 1.f;

  setPosition(position);
  display();
}

void Menu::display()
{
  m_selection = NO_ITEM;
  show();
  bringToFront();
  activate();
  onCursorPos(layer().cursorPoint());
}

void Menu::addItem(Item& item)
{
  m_items.push_back(&item);
  sizeToFit();
}

void Menu::insertItem(Item& item, uint index)
{
  m_items.insert(m_items.begin() + min(index, uint(m_items.size())), &item);
  sizeToFit();
}

void Menu::createItem(const std::string& value, ItemID ID)
{
  Item* item = new Item(layer(), value, ID);
  addItem(*item);
}

void Menu::createSeparatorItem()
{
  Item* item = new SeparatorItem(layer());
  addItem(*item);
}

Item* Menu::findItem(const std::string& value)
{
  for (Item* i : m_items)
  {
    if (i->value() == value)
      return i;
  }

  return nullptr;
}

const Item* Menu::findItem(const std::string& value) const
{
  for (const Item* i : m_items)
  {
    if (i->value() == value)
      return i;
  }

  return nullptr;
}

void Menu::destroyItem(Item& item)
{
  auto i = std::find(m_items.begin(), m_items.end(), &item);
  if (i != m_items.end())
  {
    delete *i;
    m_items.erase(i);

    sizeToFit();
  }
}

void Menu::destroyItems()
{
  while (!m_items.empty())
  {
    delete m_items.back();
    m_items.pop_back();
  }

  sizeToFit();
}

void Menu::sortItems()
{
  ItemComparator comparator;
  std::sort(m_items.begin(), m_items.end(), comparator);
}

void Menu::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    drawer.drawFrame(area, state());

    float itemTop = area.size.y - 1.f;

    uint index = 0;

    for (const Item* i : m_items)
    {
      float height = i->height();
      if (height + itemTop < 0.f)
        break;

      const Rect itemArea(area.position + vec2(0.f, itemTop - height),
                          vec2(area.size.x, height));
      i->draw(itemArea, index == m_selection ? STATE_SELECTED : STATE_NORMAL);

      itemTop -= height;
      index++;
    }

    Widget::draw();

    drawer.popClipArea();
  }
}

void Menu::onFocusChanged(bool activated)
{
  if (!activated)
    hide();

  Widget::onFocusChanged(activated);
}

void Menu::onCursorPos(vec2 point)
{
  const vec2 local = transformToLocal(point);
  float itemTop = height() - 2.f;
  int index = 0;

  for (const Item* i : m_items)
  {
    const float itemHeight = i->height();
    if (itemTop - itemHeight < 0.f)
      break;

    if (itemTop - itemHeight <= local.y)
    {
      m_selection = index;
      invalidate();
      break;
    }

    itemTop -= itemHeight;
    index++;
  }

  Widget::onCursorPos(point);
}

void Menu::onCursorLeft()
{
  m_selection = NO_ITEM;
  invalidate();

  Widget::onCursorLeft();
}

void Menu::onMouseButton(vec2 point,
                         MouseButton button,
                         Action action,
                         uint mods)
{
  if (action == RELEASED)
  {
    const vec2 local = transformToLocal(point);
    float itemTop = height() - 2.f;
    uint index = 0;

    for (const Item* i : m_items)
    {
      const float itemHeight = i->height();
      if (itemTop - itemHeight < 0.f)
        break;

      if (itemTop - itemHeight <= local.y)
      {
        m_itemSelected(*this, index);
        hide();
        break;
      }

      itemTop -= itemHeight;
      index++;
    }
  }

  Widget::onMouseButton(point, button, action, mods);
}

void Menu::onKey(Key key, Action action, uint mods)
{
  if ((action == PRESSED || action == REPEATED) && mods == 0)
  {
    switch (key)
    {
      case KEY_UP:
      {
        if (m_selection == NO_ITEM)
        {
          if (!m_items.empty())
            m_selection = 0;
        }
        else
          m_selection = (m_selection + m_items.size() - 1) % m_items.size();

        invalidate();
        break;
      }

      case KEY_DOWN:
      {
        if (m_selection == NO_ITEM)
        {
          if (!m_items.empty())
            m_selection = int(m_items.size() - 1);
        }
        else
          m_selection = (m_selection + 1) % m_items.size();

        invalidate();
        break;
      }

      case KEY_ENTER:
      {
        m_itemSelected(*this, m_selection);
        hide();
        break;
      }

      case KEY_ESCAPE:
      {
        hide();
        break;
      }

      default:
        break;
    }
  }

  Widget::onKey(key, action, mods);
}

void Menu::onDragEnded(vec2 point, MouseButton button)
{
  if (!area().contains(transformToLocal(point)))
    hide();

  Widget::onDragEnded(point, button);
}

void Menu::sizeToFit()
{
  vec2 size(0.f, 2.f);

  for (const Item* i : m_items)
  {
    size.x = max(i->width(), size.x);
    size.y += i->height();
  }

  setSize(size);
}

} /*namespace nori*/

