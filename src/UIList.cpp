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

#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderFont.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>
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

List::List(void):
  offset(0),
  selection(0)
{
  getButtonClickedSignal().connect(*this, &List::onButtonClicked);
  getKeyPressedSignal().connect(*this, &List::onKeyPressed);
  getWheelTurnedSignal().connect(*this, &List::onWheelTurned);
}

List::~List(void)
{
  destroyItems();
}

void List::insertItem(Item* item, unsigned int index)
{
  ItemList::iterator i = std::find(items.begin(), items.end(), item);
  if (i != items.end())
    return;

  ItemList::iterator p = items.begin();
  std::advance(p, index);
  items.insert(p, item);
}

void List::destroyItem(Item* item)
{
  ItemList::iterator i = std::find(items.begin(), items.end(), item);
  if (i != items.end())
  {
    delete *i;
    items.erase(i);

    setOffset(offset);
  }
}

void List::destroyItems(void)
{
  while (!items.empty())
    destroyItem(items.front());
}

void List::sortItems(void)
{
  ItemComparator comparator;
  std::sort(items.begin(), items.end(), comparator);
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
}

unsigned int List::getSelection(void) const
{
  return selection;
}

void List::setSelection(unsigned int newIndex)
{
  setSelection(newIndex, false);
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

void List::render(void) const
{
  const Rectangle& area = getGlobalArea();

  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawFrame(area, getState());

    float start = area.size.y;

    for (unsigned int i = offset;  i < items.size();  i++)
    {
      const Item& item = *items[i];

      float height = item.getHeight();
      if (height + start < 0.f)
	break;

      Rectangle itemArea = area;
      itemArea.position.y += start - height;
      itemArea.size.y = height;

      item.render(itemArea, i == selection);

      start -= height;
    }

    Widget::render();

    renderer->popClipArea();
  }
}

void List::onButtonClicked(Widget& widget,
			   const Vector2& position,
			   unsigned int button,
			   bool clicked)
{
  if (!clicked || button != 0)
    return;

  Vector2 localPosition = transformToLocal(position);

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

void List::onKeyPressed(Widget& widget, GL::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case GL::Key::UP:
    {
      if (selection > 0)
	setSelection(selection - 1, true);
      break;
    }

    case GL::Key::DOWN:
    {
      setSelection(selection + 1, true);
      break;
    }

    case GL::Key::HOME:
    {
      setSelection(0, true);
      break;
    }

    case GL::Key::END:
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
