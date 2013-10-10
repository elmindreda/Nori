//////////////////////////////////////////////////////////////////////
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
  editable(false),
  editing(false),
  offset(0),
  maxOffset(0),
  selection(NO_ITEM),
  scroller(nullptr),
  entry(nullptr)
{
  areaChangedSignal().connect(*this, &List::onAreaChanged);
  buttonClickedSignal().connect(*this, &List::onMouseButton);
  keyPressedSignal().connect(*this, &List::onKey);
  scrolledSignal().connect(*this, &List::onScroll);

  scroller = new Scroller(layer, VERTICAL);
  scroller->setValueRange(0.f, 1.f);
  scroller->setPercentage(1.f);
  scroller->getValueChangedSignal().connect(*this, &List::onValueChanged);
  addChild(*scroller);

  onAreaChanged(*this);
}

List::~List()
{
  delete entry;
  destroyItems();
}

void List::addItem(Item& item)
{
  if (std::find(items.begin(), items.end(), &item) != items.end())
    return;

  items.push_back(&item);
  updateScroller();
}

void List::createItem(const char* value, ItemID ID)
{
  Item* item = new Item(layer(), value, ID);
  addItem(*item);
}

Item* List::findItem(const char* value)
{
  for (auto i : items)
  {
    if (i->asString() == value)
      return i;
  }

  return nullptr;
}

const Item* List::findItem(const char* value) const
{
  for (auto i : items)
  {
    if (i->asString() == value)
      return i;
  }

  return nullptr;
}

void List::destroyItem(Item& item)
{
  auto i = std::find(items.begin(), items.end(), &item);
  assert(i != items.end());

  if (selection == i - items.begin())
    setSelection(NO_ITEM, false);

  delete *i;
  items.erase(i);
  updateScroller();
}

void List::destroyItems()
{
  while (!items.empty())
  {
    delete items.back();
    items.pop_back();
  }

  setSelection(NO_ITEM, false);
  updateScroller();
}

void List::sortItems()
{
  ItemComparator comparator;
  std::sort(items.begin(), items.end(), comparator);

  updateScroller();
}

bool List::isEditable() const
{
  return editable;
}

void List::setEditable(bool newState)
{
  if (editable == newState)
    return;

  editable = newState;

  if (editable)
  {
    entry = new UI::Entry(layer());
    entry->hide();
    entry->focusChangedSignal().connect(*this, &List::onEntryFocusChanged);
    entry->keyPressedSignal().connect(*this, &List::onEntryKeyPressed);
    entry->destroyedSignal().connect(*this, &List::onEntryDestroyed);
    layer().addRootWidget(*entry);
  }
  else
  {
    cancelEditing();
    delete entry;
    entry = nullptr;
  }
}

uint List::getOffset() const
{
  return offset;
}

void List::setOffset(uint newOffset)
{
  offset = min(newOffset, maxOffset);
  scroller->setValue(float(offset));
}

uint List::getSelection() const
{
  return selection;
}

void List::setSelection(uint newSelection)
{
  assert(newSelection == NO_ITEM || newSelection < items.size());
  setSelection(newSelection, false);
}

Item* List::getSelectedItem()
{
  if (selection == NO_ITEM)
    return nullptr;

  assert(selection < items.size());
  return items[selection];
}

void List::setSelectedItem(Item& newItem)
{
  auto i = std::find(items.begin(), items.end(), &newItem);
  assert(i != items.end());
  setSelection(i - items.begin(), false);
}

uint List::getItemCount() const
{
  return (uint) items.size();
}

Item* List::getItem(uint index)
{
  assert(index < items.size());
  return items[index];
}

const Item* List::getItem(uint index) const
{
  assert(index < items.size());
  return items[index];
}

const ItemList& List::getItems() const
{
  return items;
}

SignalProxy1<void, List&> List::getItemSelectedSignal()
{
  return itemSelectedSignal;
}

void List::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    drawer.drawWell(area, state());

    float start = area.size.y;

    for (uint i = offset;  i < items.size();  i++)
    {
      const Item& item = *items[i];

      float height = item.getHeight();
      if (height + start < 0.f)
        break;

      Rect itemArea = area;
      itemArea.position.y += start - height;
      itemArea.size.y = height;

      item.draw(itemArea, i == selection ? STATE_SELECTED : STATE_NORMAL);

      start -= height;
    }

    Widget::draw();

    drawer.popClipArea();
  }
}

void List::onAreaChanged(Widget& widget)
{
  scroller->setArea(Rect(width() - scroller->width(), 0.f,
                         scroller->width(), height()));
  updateScroller();
}

void List::onEntryFocusChanged(Widget& widget, bool activated)
{
  if (editing)
    applyEditing();
}

void List::onEntryKeyPressed(Widget& widget, Key key, Action action, uint mods)
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
  entry = nullptr;
}

void List::onMouseButton(Widget& widget,
                         vec2 position,
                         MouseButton button,
                         Action action,
                         uint mods)
{
  if (action != PRESSED)
    return;

  const vec2 local = transformToLocal(position);

  float itemTop = height();

  for (uint i = offset;  i < items.size();  i++)
  {
    const float itemHeight = items[i]->getHeight();
    const float itemBottom = itemTop - itemHeight;

    if (itemBottom <= local.y)
    {
      if (itemBottom < 0.f)
        setOffset(offset + 1);

      if (selection == i)
      {
        if (editable)
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

void List::onKey(Widget& widget, Key key, Action action, uint mods)
{
  if (action != PRESSED)
    return;

  switch (key)
  {
    case KEY_UP:
    {
      if (selection == NO_ITEM)
      {
        if (!items.empty())
          setSelection(items.size() - 1, true);
      }
      else if (selection > 0)
        setSelection(selection - 1, true);
      break;
    }

    case KEY_DOWN:
    {
      if (selection == NO_ITEM)
      {
        if (!items.empty())
          setSelection(0, true);
      }
      else if (selection < items.size() - 1)
        setSelection(selection + 1, true);
      break;
    }

    case KEY_HOME:
    {
      setSelection(0, true);
      break;
    }

    case KEY_END:
    {
      if (!items.empty())
        setSelection(items.size() - 1, true);
      break;
    }

    default:
      break;
  }
}

void List::onScroll(Widget& widget, vec2 so)
{
  if (items.empty())
    return;

  if (int(so.y) + (int) offset < 0)
    return;

  if (editing)
    return;

  setOffset(offset + int(so.y));
}

void List::onValueChanged(Scroller& scroller)
{
  setOffset((uint) scroller.getValue());
}

void List::beginEditing()
{
  if (Item* selected = getSelectedItem())
  {
    const Rect area = globalArea();
    const float selectedHeight = selected->getHeight();

    Rect entryArea(vec2(0.f, area.size.y - selectedHeight),
                   vec2(area.size.x, selectedHeight));

    if (scroller->isVisible())
      entryArea.size.x -= scroller->width();

    for (uint i = offset;  i < selection;  i++)
      entryArea.position.y -= items[i]->getHeight();

    entryArea.position += area.position;

    const String& value = selected->asString();

    entry->setArea(entryArea);
    entry->setText(value.c_str());
    entry->setCaretPosition(value.length());
    entry->show();
    entry->activate();
    editing = true;
  }
}

void List::applyEditing()
{
  entry->hide();
  editing = false;

  if (Item* item = getSelectedItem())
    item->setStringValue(entry->getText().c_str());
}

void List::cancelEditing()
{
  entry->hide();
  editing = false;
}

void List::updateScroller()
{
  maxOffset = 0;

  float totalItemHeight = 0.f;

  for (auto i : items)
    totalItemHeight += i->getHeight();

  float visibleItemHeight = 0.f;

  for (auto i = items.rbegin();  i != items.rend();  i++)
  {
    visibleItemHeight += (*i)->getHeight();
    if (visibleItemHeight > height())
    {
      maxOffset = items.rend() - i;
      break;
    }
  }

  if (maxOffset)
  {
    scroller->show();
    scroller->setValueRange(0.f, float(maxOffset));
    scroller->setPercentage(height() / totalItemHeight);
  }
  else
    scroller->hide();

  setOffset(offset);
}

bool List::isSelectionVisible() const
{
  if (selection == NO_ITEM)
    return true;

  if (selection < offset)
    return false;

  float visibleItemHeight = 0.f;

  for (uint i = offset;  i <= selection;  i++)
  {
    visibleItemHeight += items[i]->getHeight();
    if (visibleItemHeight > height())
      return false;
  }

  return true;
}

void List::setSelection(uint newSelection, bool notify)
{
  if (selection == newSelection)
    return;

  selection = newSelection;

  if (isSelectionVisible())
    invalidate();
  else
    setOffset(selection);

  if (notify)
    itemSelectedSignal(*this);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
