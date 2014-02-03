//////////////////////////////////////////////////////////////////////
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

#include <wendy/Config.hpp>

#include <wendy/UIDrawer.hpp>
#include <wendy/UILayer.hpp>
#include <wendy/UIWidget.hpp>
#include <wendy/UIScroller.hpp>
#include <wendy/UIEntry.hpp>
#include <wendy/UIItem.hpp>
#include <wendy/UIList.hpp>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

List::List(Layer& layer):
  Widget(layer),
  m_editable(false),
  m_editing(false),
  m_offset(0),
  m_maxOffset(0),
  m_selection(NO_ITEM),
  m_scroller(nullptr),
  m_entry(nullptr)
{
  m_scroller = new Scroller(layer, VERTICAL);
  m_scroller->setValueRange(0.f, 1.f);
  m_scroller->setPercentage(1.f);
  m_scroller->valueChangedSignal().connect(*this, &List::onValueChanged);
  addChild(*m_scroller);

  onAreaChanged();
}

List::~List()
{
  delete m_entry;
  destroyItems();
}

void List::addItem(Item& item)
{
  if (std::find(m_items.begin(), m_items.end(), &item) != m_items.end())
    return;

  m_items.push_back(&item);
  updateScroller();
}

void List::createItem(const char* value, ItemID ID)
{
  Item* item = new Item(layer(), value, ID);
  addItem(*item);
}

Item* List::findItem(const char* value)
{
  for (auto i : m_items)
  {
    if (i->asString() == value)
      return i;
  }

  return nullptr;
}

const Item* List::findItem(const char* value) const
{
  for (auto i : m_items)
  {
    if (i->asString() == value)
      return i;
  }

  return nullptr;
}

void List::destroyItem(Item& item)
{
  auto i = std::find(m_items.begin(), m_items.end(), &item);
  assert(i != m_items.end());

  if (m_selection == i - m_items.begin())
    setSelection(NO_ITEM, false);

  delete *i;
  m_items.erase(i);
  updateScroller();
}

void List::destroyItems()
{
  while (!m_items.empty())
  {
    delete m_items.back();
    m_items.pop_back();
  }

  setSelection(NO_ITEM, false);
  updateScroller();
}

void List::sortItems()
{
  ItemComparator comparator;
  std::sort(m_items.begin(), m_items.end(), comparator);

  updateScroller();
}

bool List::isEditable() const
{
  return m_editable;
}

void List::setEditable(bool newState)
{
  if (m_editable == newState)
    return;

  m_editable = newState;

  if (m_editable)
  {
    m_entry = new UI::Entry(layer());
    m_entry->hide();
    m_entry->focusChangedSignal().connect(*this, &List::onEntryFocusChanged);
    m_entry->keySignal().connect(*this, &List::onEntryKey);
    m_entry->destroyedSignal().connect(*this, &List::onEntryDestroyed);
    layer().addRootWidget(*m_entry);
  }
  else
  {
    cancelEditing();
    delete m_entry;
    m_entry = nullptr;
  }
}

uint List::offset() const
{
  return m_offset;
}

void List::setOffset(uint newOffset)
{
  m_offset = min(newOffset, m_maxOffset);
  m_scroller->setValue(float(m_offset));
}

uint List::selection() const
{
  return m_selection;
}

void List::setSelection(uint newSelection)
{
  assert(newSelection == NO_ITEM || newSelection < m_items.size());
  setSelection(newSelection, false);
}

Item* List::selectedItem()
{
  if (m_selection == NO_ITEM)
    return nullptr;

  assert(m_selection < m_items.size());
  return m_items[m_selection];
}

void List::setSelectedItem(Item& newItem)
{
  auto i = std::find(m_items.begin(), m_items.end(), &newItem);
  assert(i != m_items.end());
  setSelection(uint(i - m_items.begin()), false);
}

uint List::itemCount() const
{
  return (uint) m_items.size();
}

Item* List::item(uint index)
{
  assert(index < m_items.size());
  return m_items[index];
}

const Item* List::item(uint index) const
{
  assert(index < m_items.size());
  return m_items[index];
}

const ItemList& List::items() const
{
  return m_items;
}

SignalProxy<void, List&> List::itemSelectedSignal()
{
  return m_itemSelectedSignal;
}

void List::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    drawer.drawWell(area, state());

    float start = area.size.y;

    for (uint i = m_offset;  i < m_items.size();  i++)
    {
      const Item& item = *m_items[i];

      float height = item.height();
      if (height + start < 0.f)
        break;

      const Rect itemArea(area.position + vec2(0.f, start - height),
                          vec2(area.size.x, height));
      item.draw(itemArea, i == m_selection ? STATE_SELECTED : STATE_NORMAL);

      start -= height;
    }

    Widget::draw();

    drawer.popClipArea();
  }
}

void List::onAreaChanged()
{
  m_scroller->setArea(Rect(width() - m_scroller->width(), 0.f,
                           m_scroller->width(), height()));
  updateScroller();

  Widget::onAreaChanged();
}

void List::onMouseButton(vec2 point,
                         MouseButton button,
                         Action action,
                         uint mods)
{
  if (action == PRESSED)
  {
    const vec2 local = transformToLocal(point);

    float itemTop = height();

    for (uint i = m_offset;  i < m_items.size();  i++)
    {
      const float itemHeight = m_items[i]->height();
      const float itemBottom = itemTop - itemHeight;

      if (itemBottom <= local.y)
      {
        if (itemBottom < 0.f)
          setOffset(m_offset + 1);

        if (m_selection == i)
        {
          if (m_editable)
            beginEditing();
        }
        else
          setSelection(i, true);

        return;
      }

      itemTop = itemBottom;
      if (itemTop < 0.f)
        break;
    }
  }

  Widget::onMouseButton(point, button, action, mods);
}

void List::onKey(Key key, Action action, uint mods)
{
  if (action == PRESSED)
  {
    switch (key)
    {
      case KEY_UP:
      {
        if (m_selection == NO_ITEM)
        {
          if (!m_items.empty())
            setSelection(uint(m_items.size() - 1), true);
        }
        else if (m_selection > 0)
          setSelection(m_selection - 1, true);
        break;
      }

      case KEY_DOWN:
      {
        if (m_selection == NO_ITEM)
        {
          if (!m_items.empty())
            setSelection(0, true);
        }
        else if (m_selection < m_items.size() - 1)
          setSelection(m_selection + 1, true);
        break;
      }

      case KEY_HOME:
      {
        setSelection(0, true);
        break;
      }

      case KEY_END:
      {
        if (!m_items.empty())
          setSelection(uint(m_items.size() - 1), true);
        break;
      }

      default:
        break;
    }
  }

  Widget::onKey(key, action, mods);
}

void List::onScroll(vec2 offset)
{
  if (!m_items.empty() && !m_editing &&
      int(offset.y) + int(m_offset) >= 0)
  {
    setOffset(m_offset + int(offset.y));
  }

  Widget::onScroll(offset);
}

void List::onEntryFocusChanged(Widget& widget, bool activated)
{
  if (m_editing)
    applyEditing();
}

void List::onEntryKey(Widget& widget, Key key, Action action, uint mods)
{
  if (action != PRESSED)
    return;

  switch (key)
  {
    case KEY_ENTER:
    {
      applyEditing();
      break;
    }

    case KEY_ESCAPE:
    {
      cancelEditing();
      break;
    }

    default:
      break;
  }
}

void List::onEntryDestroyed(Widget& widget)
{
  cancelEditing();
  m_entry = nullptr;
}

void List::onValueChanged(Scroller& scroller)
{
  setOffset((uint) scroller.value());
}

void List::beginEditing()
{
  if (Item* selected = selectedItem())
  {
    const Rect area = globalArea();
    const float selectedHeight = selected->height();

    Rect entryArea(vec2(0.f, area.size.y - selectedHeight),
                   vec2(area.size.x, selectedHeight));

    if (m_scroller->isVisible())
      entryArea.size.x -= m_scroller->width();

    for (uint i = m_offset;  i < m_selection;  i++)
      entryArea.position.y -= m_items[i]->height();

    entryArea.position += area.position;

    const String& value = selected->asString();

    m_entry->setArea(entryArea);
    m_entry->setText(value.c_str());
    m_entry->setCaretPosition(uint(value.length()));
    m_entry->show();
    m_entry->activate();
    m_editing = true;
  }
}

void List::applyEditing()
{
  m_entry->hide();
  m_editing = false;

  if (Item* item = selectedItem())
    item->setStringValue(m_entry->text().c_str());
}

void List::cancelEditing()
{
  m_entry->hide();
  m_editing = false;
}

void List::updateScroller()
{
  m_maxOffset = 0;

  float totalItemHeight = 0.f;

  for (auto i : m_items)
    totalItemHeight += i->height();

  float visibleItemHeight = 0.f;

  for (auto i = m_items.rbegin();  i != m_items.rend();  i++)
  {
    visibleItemHeight += (*i)->height();
    if (visibleItemHeight > height())
    {
      m_maxOffset = uint(m_items.rend() - i);
      break;
    }
  }

  if (m_maxOffset)
  {
    m_scroller->show();
    m_scroller->setValueRange(0.f, float(m_maxOffset));
    m_scroller->setPercentage(height() / totalItemHeight);
  }
  else
    m_scroller->hide();

  setOffset(m_offset);
}

bool List::isSelectionVisible() const
{
  if (m_selection == NO_ITEM)
    return true;

  if (m_selection < m_offset)
    return false;

  float visibleItemHeight = 0.f;

  for (auto i : m_items)
  {
    visibleItemHeight += i->height();
    if (visibleItemHeight > height())
      return false;
  }

  return true;
}

void List::setSelection(uint newSelection, bool notify)
{
  if (m_selection == newSelection)
    return;

  m_selection = newSelection;

  if (isSelectionVisible())
    invalidate();
  else
    setOffset(m_selection);

  if (notify)
    m_itemSelectedSignal(*this);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
