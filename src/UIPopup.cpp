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
#include <wendy/UIPopup.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Popup::Popup(void):
  selection(0)
{
  const float em = Renderer::get()->getDefaultEM();

  setSize(Vec2(em * 10.f, em * 2.f));

  getKeyPressedSignal().connect(*this, &Popup::onKeyPressed);
  getButtonClickedSignal().connect(*this, &Popup::onButtonClicked);

  menu = new Menu();
  menu->getItemSelectedSignal().connect(*this, &Popup::onItemSelected);
}

void Popup::addItem(Item& item)
{
  menu->addItem(item);
}

void Popup::addItem(const String& value)
{
  Item* item = new Item(value);
  menu->addItem(*item);
}

void Popup::removeItem(Item& item)
{
  menu->removeItem(item);
  setSelection(selection);
}

void Popup::destroyItems(void)
{
  menu->destroyItems();
  selection = 0;
}

unsigned int Popup::getSelection(void) const
{
  return selection;
}

void Popup::setSelection(unsigned int newIndex)
{
  if (menu->getItemCount())
    selection = std::min(newIndex, menu->getItemCount() - 1);
  else
    selection = 0;
}

unsigned int Popup::getItemCount(void) const
{
  return menu->getItemCount();
}

Item* Popup::getItem(unsigned int index)
{
  return menu->getItem(index);
}

const Item* Popup::getItem(unsigned int index) const
{
  return menu->getItem(index);
}

String Popup::getItemValue(unsigned int index) const
{
  if (const Item* item = menu->getItem(index))
    return item->asString();

  return String();
}

SignalProxy2<void, Popup&, unsigned int> Popup::getItemSelectedSignal(void)
{
  return itemSelectedSignal;
}

void Popup::draw(void) const
{
  const Rect& area = getGlobalArea();

  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawFrame(area, getState());

    if (selection < menu->getItemCount())
    {
      const Item* item = menu->getItem(selection);

      const float em = Renderer::get()->getDefaultEM();

      Rect textArea = area;
      textArea.position.x += em / 2.f;
      textArea.size.x -= em;

      renderer->drawText(textArea, item->asString(), LEFT_ALIGNED);
    }

    Widget::draw();

    renderer->popClipArea();
  }
}

void Popup::display(void)
{
  const float width = std::max(menu->getArea().size.x, getArea().size.x);
  menu->setArea(Rect(getGlobalArea().position,
                     Vec2(width, menu->getArea().size.y)));
  menu->display(*getDesktop());
}

void Popup::onButtonClicked(Widget& widget,
		            const Vec2& position,
		            input::Button button,
		            bool clicked)
{
  if (!clicked)
    return;

  display();
}

void Popup::onKeyPressed(Widget& widget, input::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case input::Key::ENTER:
    {
      display();
      break;
    }
  }
}

void Popup::onItemSelected(Menu& menu, unsigned int index)
{
  selection = index;
  itemSelectedSignal.emit(*this, selection);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
