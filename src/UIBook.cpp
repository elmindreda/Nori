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
    if (!pages.empty())
    {
      const vec2 tabSize(area.size.x / pages.size(),
                         drawer.getCurrentEM() * 2.f);

      Rect tabArea(area.position.x,
                   area.position.y + area.size.y - tabSize.y,
                   tabSize.x,
                   tabSize.y);

      for (auto p = pages.begin();  p != pages.end();  p++)
      {
        WidgetState state;
        if (isEnabled())
        {
          if (activePage == *p)
            state = STATE_ACTIVE;
          else
            state = STATE_NORMAL;
        }
        else
          state = STATE_DISABLED;

        drawer.drawTab(tabArea, state, (*p)->getText().c_str());

        tabArea.position.x += tabSize.x;
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

    pages.push_back(page);
  }
}

void Book::removedChild(Widget& child)
{
  if (Page* page = dynamic_cast<Page*>(&child))
  {
    if (page == activePage)
    {
      if (pages.empty())
        setActivePage(NULL, false);
      else
        setActivePage(pages.front(), false);
    }

    pages.erase(std::find(pages.begin(), pages.end(), page));
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
  const float em = getLayer().getDrawer().getCurrentEM();
  const vec2& size = getSize();

  for (auto p = pages.begin();  p != pages.end();  p++)
    (*p)->setArea(Rect(0.f, 0.f, size.x, size.y - em * 2.f));
}

void Book::onKeyPressed(Widget& widgeth, input::Key key, bool pressed)
{
  if (!pressed)
    return;

  auto p = std::find(pages.begin(), pages.end(), activePage);
  if (p == pages.end())
    return;

  switch (key)
  {
    case input::KEY_TAB:
    case input::KEY_RIGHT:
    {
      if (++p == pages.end())
        setActivePage(pages.front(), true);
      else
        setActivePage(*p, true);
      break;
    }

    case input::KEY_LEFT:
    {
      if (p == pages.begin())
        setActivePage(pages.back(), true);
      else
        setActivePage(*(--p), true);
      break;
    }
  }
}

void Book::onButtonClicked(Widget& widget,
                           const vec2& point,
                           input::Button button,
                           bool clicked)
{
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
