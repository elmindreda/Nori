//////////////////////////////////////////////////////////////////////
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

#include <wendy/Config.h>

#include <wendy/UIDrawer.h>
#include <wendy/UILayer.h>
#include <wendy/UIWidget.h>
#include <wendy/UIBook.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Page::Page(Layer& layer, const char* initText):
  Widget(layer),
  text(initText)
{
}

const String& Page::getText() const
{
  return text;
}

void Page::setText(const char* newText)
{
  text = newText;
  invalidate();
}

///////////////////////////////////////////////////////////////////////

Book::Book(Layer& layer):
  Widget(layer),
  activePage(NULL)
{
  getKeyPressedSignal().connect(*this, &Book::onKeyPressed);
  getButtonClickedSignal().connect(*this, &Book::onButtonClicked);
  getAreaChangedSignal().connect(*this, &Book::onAreaChanged);
}

Page* Book::getActivePage() const
{
  return activePage;
}

void Book::setActivePage(Page* newPage)
{
  setActivePage(newPage, false);
}

SignalProxy1<void, Book&> Book::getPageChangedSignal()
{
  return pageChangedSignal;
}

void Book::draw() const
{
  const Rect& area = getGlobalArea();

  Drawer& drawer = getLayer().getDrawer();
  if (drawer.pushClipArea(area))
  {
    PageList pages;
    getPages(pages);

    if (!pages.empty())
    {
      const Rect& area = getArea();

      const vec2 buttonSize(area.size.x / pages.size(),
                            drawer.getCurrentEM() * 2.f);

      Rect buttonArea(area.position.x,
                      area.position.y + area.size.y - buttonSize.y,
                      buttonSize.x,
                      buttonSize.y);

      for (unsigned int i = 0;  i < pages.size();  i++)
      {
        WidgetState state;
        if (isEnabled())
        {
          if (activePage == pages[i])
            state = STATE_ACTIVE;
          else
            state = STATE_NORMAL;
        }
        else
          state = STATE_DISABLED;

        drawer.drawButton(buttonArea, state, pages[i]->getText().c_str());

        buttonArea.position.x += buttonSize.x;
      }
    }

    Widget::draw();

    drawer.popClipArea();
  }
}

void Book::addedChild(Widget& child)
{
  if (Page* page = dynamic_cast<Page*>(&child))
  {
    const float em = getLayer().getDrawer().getCurrentEM();

    const vec2& size = getSize();

    page->setArea(Rect(0.f, 0.f, size.x, size.y - em * 2.f));

    if (activePage)
      page->hide();
    else
      setActivePage(page, false);
  }
}

void Book::removedChild(Widget& child)
{
  if (Page* page = dynamic_cast<Page*>(&child))
  {
    if (page == activePage)
    {
      PageList pages;
      getPages(pages);

      if (pages.empty())
        setActivePage(NULL, false);
      else
        setActivePage(pages.front(), false);
    }
  }
}

void Book::getPages(PageList& pages) const
{
  const WidgetList& children = getChildren();

  pages.reserve(children.size());

  for (WidgetList::const_iterator i = children.begin();  i != children.end();  i++)
  {
    if (Page* page = dynamic_cast<Page*>(*i))
      pages.push_back(page);
  }
}

void Book::setActivePage(Page* newPage, bool notify)
{
  if (newPage == activePage)
    return;

  const WidgetList& children = getChildren();

  if (std::find(children.begin(), children.end(), newPage) == children.end())
    return;

  if (activePage)
    activePage->hide();

  activePage = newPage;
  activePage->show();
  activePage->activate();

  if (notify)
    pageChangedSignal(*this);

  invalidate();
}

void Book::onAreaChanged(Widget& widget)
{
  PageList pages;
  getPages(pages);

  const float em = getLayer().getDrawer().getCurrentEM();

  const vec2& size = getSize();

  for (PageList::const_iterator i = pages.begin();  i != pages.end();  i++)
    (*i)->setArea(Rect(0.f, 0.f, size.x, size.y - em * 2.f));
}

void Book::onKeyPressed(Widget& widgeth, input::Key key, bool pressed)
{
  if (!pressed)
    return;

  PageList pages;
  getPages(pages);

  PageList::const_iterator i = std::find(pages.begin(), pages.end(), activePage);
  if (i == pages.end())
    return;

  switch (key)
  {
    case input::KEY_TAB:
    case input::KEY_RIGHT:
    {
      if (++i == pages.end())
        setActivePage(pages.front(), true);
      else
        setActivePage(*i, true);
      break;
    }

    case input::KEY_LEFT:
    {
      if (i == pages.begin())
        setActivePage(pages.back(), true);
      else
        setActivePage(*(--i), true);
      break;
    }
  }
}

void Book::onButtonClicked(Widget& widget,
                           const vec2& point,
                           input::Button button,
                           bool clicked)
{
  PageList pages;
  getPages(pages);

  const float position = transformToLocal(point).x;
  const float width = getWidth() / pages.size();

  const unsigned int index = (unsigned int) (position / width);

  if (pages[index] != activePage)
    setActivePage(pages[index], true);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
