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
#include <wendy/GLRender.h>
#include <wendy/GLFont.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>
#include <wendy/UIList.h>

#include <cstdlib>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Item::Item(const String& initValue):
  value(initValue)
{
}

bool Item::operator < (const Item& other) const
{
  return value < other.value;
}

float Item::getHeight(void) const
{
  return Renderer::get()->getCurrentFont()->getHeight() * 1.5f;
}

const String& Item::getValue(void) const
{
  return value;
}

void Item::setValue(const String& newValue)
{
  value = newValue;
}

void Item::render(const Rectangle& area, bool selected)
{
  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    GL::Font* font = renderer->getCurrentFont();

    Rectangle textArea = area;
    textArea.position.x += font->getWidth() / 2.f;
    textArea.size.x -= font->getWidth();

    if (selected)
    {
      GL::RenderPass pass;
      pass.setDefaultColor(ColorRGBA(renderer->getSelectionColor(), 1.f));
      pass.setDepthTesting(false);
      pass.setDepthWriting(false);
      pass.apply();

      glRectf(area.position.x, area.position.y, area.position.x + area.size.x, area.position.y + area.size.y);
    }

    renderer->drawText(textArea, value, LEFT_ALIGNED, selected);
    renderer->popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

List::List(const String& name):
  Widget(name),
  selection(0)
{
  getButtonClickSignal().connect(*this, &List::onButtonClick);
  getKeyPressSignal().connect(*this, &List::onKeyPress);
}

void List::insertItem(Item* item, unsigned int index)
{
  ItemList::iterator i = std::find(items.begin(), items.end(), item);
  if (i != items.end())
    return;

  addItemSignal.emit(*this, *item);

  ItemList::iterator p = items.begin();
  std::advance(p, index);
  items.insert(p, item);
}

void List::removeItem(Item* item)
{
  ItemList::iterator i = std::find(items.begin(), items.end(), item);
  if (i != items.end())
  {
    removeItemSignal.emit(*this, **i);

    delete *i;
    items.erase(i);
  }
}

void List::removeItems(void)
{
  while (!items.empty())
    removeItem(items.front());
}

void List::sortItems(void)
{
  // TODO: The code.
}

unsigned int List::getSelection(void) const
{
  return selection;
}

void List::setSelection(unsigned int newIndex)
{
  if (items.empty())
    return;

  newIndex = std::min(newIndex, (unsigned int) items.size() - 1);
  changeSelectionSignal.emit(*this, newIndex);
  selection = newIndex;
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

SignalProxy2<void, List&, Item&> List::getAddItemSignal(void)
{
  return addItemSignal;
}

SignalProxy2<void, List&, Item&> List::getRemoveItemSignal(void)
{
  return removeItemSignal;
}

SignalProxy2<void, List&, unsigned int> List::getChangeSelectionSignal(void)
{
  return changeSelectionSignal;
}

void List::render(void) const
{
  const Rectangle& area = getGlobalArea();

  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawFrame(area, getState());

    float start = area.size.y;

    unsigned int index = 0;

    for (ItemList::const_iterator i = items.begin();  i != items.end();  i++)
    {
      float height = (*i)->getHeight();
      if (height + start < 0.f)
	break;

      Rectangle itemArea = area;
      itemArea.position.y += start - height;
      itemArea.size.y = height;

      (*i)->render(itemArea, index == selection);

      start -= height;
      index++;
    }

    Widget::render();

    renderer->popClipArea();
  }
}

void List::onButtonClick(Widget& widget,
                         const Vector2& position,
		         unsigned int button,
		         bool clicked)
{
  if (!clicked || button != 0)
    return;

  Vector2 localPosition = transformToLocal(position);

  unsigned int index = 0;

  const float height = getArea().size.y;
  float itemTop = height;

  for (ItemList::const_iterator i = items.begin();  i != items.end();  i++)
  {
    const float itemHeight = (*i)->getHeight();
    if (itemTop - itemHeight < 0.f)
      break;

    if (itemTop - itemHeight <= localPosition.y)
    {
      setSelection(index);
      return;
    }

    itemTop -= itemHeight;
    index++;
  }
}

void List::onKeyPress(Widget& widget, GL::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case GL::Key::UP:
    {
      if (selection > 0)
	setSelection(selection - 1);
      break;
    }

    case GL::Key::DOWN:
    {
      setSelection(selection + 1);
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
