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

#include <wendy/Config.h>

#include <wendy/UIDrawer.h>
#include <wendy/UILayer.h>
#include <wendy/UIWidget.h>
#include <wendy/UIScroller.h>
#include <wendy/UIEntry.h>
#include <wendy/UIItem.h>
#include <wendy/UIList.h>

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
  scroller(NULL),
  entry(NULL)
{
  getAreaChangedSignal().connect(*this, &List::onAreaChanged);
  getButtonClickedSignal().connect(*this, &List::onButtonClicked);
  getKeyPressedSignal().connect(*this, &List::onKeyPressed);
  getScrolledSignal().connect(*this, &List::onScrolled);

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
  Item* item = new Item(getLayer(), value, ID);
  addItem(*item);
}

Item* List::findItem(const char* value)
{
  for (auto i = items.begin();  i != items.end();  i++)
  {
    if ((*i)->asString() == value)
      return *i;
  }

  return NULL;
}

const Item* List::findItem(const char* value) const
{
  for (auto i = items.begin();  i != items.end();  i++)
  {
    if ((*i)->asString() == value)
      return *i;
  }

  return NULL;
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
    entry = new UI::Entry(getLayer());
    entry->hide();
    entry->getFocusChangedSignal().connect(*this, &List::onEntryFocusChanged);
    entry->getKeyPressedSignal().connect(*this, &List::onEntryKeyPressed);
    entry->getDestroyedSignal().connect(*this, &List::onEntryDestroyed);
    getLayer().addRootWidget(*entry);
  }
  else
  {
    cancelEditing();
    delete entry;
    entry = NULL;
  }
}

unsigned int List::getOffset() const
{
  return offset;
}

void List::setOffset(unsigned int newOffset)
{
  offset = min(newOffset, maxOffset);
  scroller->setValue(float(offset));
}

unsigned int List::getSelection() const
{
  return selection;
}

void List::setSelection(unsigned int newSelection)
{
  assert(newSelection == NO_ITEM || newSelection < items.size());
  setSelection(newSelection, false);
}

Item* List::getSelectedItem()
{
  if (selection == NO_ITEM)
    return NULL;

  assert(selection < items.size());
  return items[selection];
}

void List::setSelectedItem(Item& newItem)
{
  auto i = std::find(items.begin(), items.end(), &newItem);
  assert(i != items.end());
  setSelection(i - items.begin(), false);
}

unsigned int List::getItemCount() const
{
  return (unsigned int) items.size();
}

Item* List::getItem(unsigned int index)
{
  assert(index < items.size());
  return items[index];
}

const Item* List::getItem(unsigned int index) const
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
  const Rect& area = getGlobalArea();

  Drawer& drawer = getLayer().getDrawer();
  if (drawer.pushClipArea(area))
  {
    drawer.drawWell(area, getState());

    float start = area.size.y;

    for (unsigned int i = offset;  i < items.size();  i++)
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
  const float width = scroller->getWidth();

  scroller->setArea(Rect(getWidth() - width, 0.f, width, getHeight()));
  updateScroller();
}

void List::onEntryFocusChanged(Widget& widget, bool activated)
{
  if (editing)
    applyEditing();
}

void List::onEntryKeyPressed(Widget& widget, input::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case input::KEY_ENTER:
    {
      applyEditing();
      break;
    }

    case input::KEY_ESCAPE:
    {
      cancelEditing();
      break;
    }
  }
}

void List::onEntryDestroyed(Widget& widget)
{
  cancelEditing();
  entry = NULL;
}

void List::onButtonClicked(Widget& widget,
                           const vec2& position,
                           input::Button button,
                           bool clicked)
{
  if (!clicked)
    return;

  const vec2 local = transformToLocal(position);

  float itemTop = getHeight();

  for (unsigned int i = offset;  i < items.size();  i++)
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

void List::onKeyPressed(Widget& widget, input::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case input::KEY_UP:
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

    case input::KEY_DOWN:
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

    case input::KEY_HOME:
    {
      setSelection(0, true);
      break;
    }

    case input::KEY_END:
    {
      if (!items.empty())
        setSelection(items.size() - 1, true);
      break;
    }
  }
}

void List::onScrolled(Widget& widget, double x, double y)
{
  if (items.empty())
    return;

  if (int(y) + (int) offset < 0)
    return;

  if (editing)
    return;

  setOffset(offset + int(y));
}

void List::onValueChanged(Scroller& scroller)
{
  setOffset((unsigned int) scroller.getValue());
}

void List::beginEditing()
{
  if (Item* selected = getSelectedItem())
  {
    const Rect& area = getGlobalArea();
    const float selectedHeight = selected->getHeight();

    Rect entryArea(vec2(0.f, area.size.y - selectedHeight),
                   vec2(area.size.x, selectedHeight));

    if (scroller->isVisible())
      entryArea.size.x -= scroller->getWidth();

    for (unsigned int i = offset;  i < selection;  i++)
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

  for (auto i = items.begin();  i != items.end();  i++)
    totalItemHeight += (*i)->getHeight();

  float visibleItemHeight = 0.f;

  for (auto i = items.rbegin();  i != items.rend();  i++)
  {
    visibleItemHeight += (*i)->getHeight();
    if (visibleItemHeight > getHeight())
    {
      maxOffset = items.rend() - i;
      break;
    }
  }

  if (maxOffset)
  {
    scroller->show();
    scroller->setValueRange(0.f, float(maxOffset));
    scroller->setPercentage(getHeight() / totalItemHeight);
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

  for (unsigned int i = offset;  i <= selection;  i++)
  {
    visibleItemHeight += items[i]->getHeight();
    if (visibleItemHeight > getHeight())
      return false;
  }

  return true;
}

void List::setSelection(unsigned int newSelection, bool notify)
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
