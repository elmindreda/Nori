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
#include <wendy/UIScroller.h>
#include <wendy/UIItem.h>
#include <wendy/UIList.h>

#include <algorithm>
#include <cstdlib>

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

List::List(Desktop& desktop, Widget* parent):
  Widget(desktop, parent),
  offset(0),
  selection(0)
{
  getAreaChangedSignal().connect(*this, &List::onAreaChanged);
  getButtonClickedSignal().connect(*this, &List::onButtonClicked);
  getKeyPressedSignal().connect(*this, &List::onKeyPressed);
  getWheelTurnedSignal().connect(*this, &List::onWheelTurned);

  scroller = new Scroller(desktop, this, VERTICAL);
  scroller->setValueRange(0.f, 1.f);
  scroller->setPercentage(1.f);
  scroller->getValueChangedSignal().connect(*this, &List::onValueChanged);

  onAreaChanged(*this);
}

List::~List(void)
{
  destroyItems();
}

void List::addItem(Item& item)
{
  ItemList::iterator i = std::find(items.begin(), items.end(), &item);
  if (i != items.end())
    return;

  items.push_back(&item);
  updateScroller();
}

void List::removeItem(Item& item)
{
  ItemList::iterator i = std::find(items.begin(), items.end(), &item);
  if (i != items.end())
  {
    items.erase(i);
    setOffset(offset);
  }
}

void List::destroyItems(void)
{
  while (!items.empty())
  {
    delete items.back();
    items.pop_back();
  }

  updateScroller();
}

void List::sortItems(void)
{
  ItemComparator comparator;
  std::sort(items.begin(), items.end(), comparator);

  updateScroller();
}

bool List::isItemVisible(const Item* item) const
{
  unsigned int index = 0;

  while (index < items.size() && items[index] != item)
    index++;

  if (index == items.size())
    return false;

  if (index < offset)
    return false;

  float height = items[index]->getHeight();

  for (unsigned int i = offset;  i < index;  i++)
  {
    height += items[i]->getHeight();
    if (height >= getArea().size.y)
      return false;
  }

  return true;
}

unsigned int List::getOffset(void) const
{
  return offset;
}

void List::setOffset(unsigned int newOffset)
{
  if (items.empty())
    return;

  offset = std::min(newOffset, (unsigned int) items.size() - 1);

  updateScroller();
}

unsigned int List::getSelection(void) const
{
  return selection;
}

void List::setSelection(unsigned int newIndex)
{
  setSelection(newIndex, false);
}

Item* List::getSelectedItem(void) const
{
  if (items.empty())
    return NULL;

  return items[selection];
}

unsigned int List::getItemCount(void) const
{
  return (unsigned int) items.size();
}

Item* List::getItem(unsigned int index)
{
  if (index < items.size())
  {
    ItemList::iterator i = items.begin();
    std::advance(i, index);
    return *i;
  }

  return NULL;
}

const Item* List::getItem(unsigned int index) const
{
  if (index < items.size())
  {
    ItemList::const_iterator i = items.begin();
    std::advance(i, index);
    return *i;
  }

  return NULL;
}

SignalProxy1<void, List&> List::getSelectionChangedSignal(void)
{
  return selectionChangedSignal;
}

void List::draw(void) const
{
  const Rect& area = getGlobalArea();

  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawWell(area, getState());

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

    renderer->popClipArea();
  }
}

void List::onAreaChanged(Widget& widget)
{
  const Rect& area = getArea();

  const float width = scroller->getArea().size.x;

  scroller->setArea(Rect(area.size.x - width, 0.f, width, area.size.y));
  updateScroller();
}

void List::onButtonClicked(Widget& widget,
			   const Vec2& position,
			   input::Button button,
			   bool clicked)
{
  if (!clicked || button != input::Button::LEFT)
    return;

  Vec2 localPosition = transformToLocal(position);

  const float height = getArea().size.y;
  float itemTop = height;

  for (unsigned int i = offset;  i < items.size();  i++)
  {
    const float itemHeight = items[i]->getHeight();
    if (itemTop - itemHeight < 0.f)
      break;

    if (itemTop - itemHeight <= localPosition.y)
    {
      setSelection(i, true);
      return;
    }

    itemTop -= itemHeight;
  }
}

void List::onKeyPressed(Widget& widget, input::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case input::Key::UP:
    {
      if (selection > 0)
	setSelection(selection - 1, true);
      break;
    }

    case input::Key::DOWN:
    {
      setSelection(selection + 1, true);
      break;
    }

    case input::Key::HOME:
    {
      setSelection(0, true);
      break;
    }

    case input::Key::END:
    {
      if (!items.empty())
	setSelection(items.size() - 1, true);
      break;
    }
  }
}

void List::onWheelTurned(Widget& widget, int wheelOffset)
{
  if (items.empty())
    return;

  if (wheelOffset + (int) offset < 0)
    return;

  setOffset(offset + wheelOffset);
}

void List::onValueChanged(Scroller& scroller)
{
  setOffset((unsigned int) scroller.getValue());
}

void List::updateScroller(void)
{
  const unsigned int count = getVisibleItemCount();

  if (count < items.size())
  {
    scroller->setVisible(true);
    scroller->setValue((float) offset);
    scroller->setValueRange(0.f, (float) items.size() - 1);
    scroller->setPercentage(count / (float) items.size());
  }
  else
    scroller->setVisible(false);
}

unsigned int List::getVisibleItemCount(void) const
{
  unsigned int index;

  float height = getArea().size.y;

  for (index = offset;  index < items.size();  index++)
  {
    height -= items[index]->getHeight();
    if (height < 0.f)
      break;
  }

  return index - offset;
}

void List::setSelection(unsigned int newIndex, bool notify)
{
  if (items.empty())
    return;

  selection = std::min(newIndex, (unsigned int) items.size() - 1);

  if (!isItemVisible(items[selection]))
    setOffset(selection);

  if (notify)
    selectionChangedSignal.emit(*this);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
