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
  render::Font* font = Renderer::get()->getCurrentFont();

  setSize(Vector2(font->getWidth() * 5.f, font->getHeight() * 2.f));

  getKeyPressedSignal().connect(*this, &Popup::onKeyPressed);
  getButtonClickedSignal().connect(*this, &Popup::onButtonClicked);

  menu = new Menu();
  menu->getItemSelectedSignal().connect(*this, &Popup::onItemSelected);
}

void Popup::addItem(Item& item)
{
  menu->addItem(item);
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

SignalProxy2<void, Popup&, unsigned int> Popup::getItemSelectedSignal(void)
{
  return itemSelectedSignal;
}

void Popup::render(void) const
{
  const Rectangle& area = getGlobalArea();

  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    if (selection < menu->getItemCount())
    {
      const Item* item = menu->getItem(selection);
      Renderer::get()->drawFrame(area, getState());
    }
    else
      Renderer::get()->drawFrame(area, getState());

    renderer->popClipArea();

    Widget::render();
  }
}

void Popup::display(void)
{
  menu->setPosition(getGlobalArea().position);
  menu->display();
}

void Popup::onButtonClicked(Widget& widget,
		            const Vector2& position,
		            unsigned int button,
		            bool clicked)
{
  if (!clicked)
    return;

  display();
}

void Popup::onKeyPressed(Widget& widget, GL::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case GL::Key::ENTER:
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
