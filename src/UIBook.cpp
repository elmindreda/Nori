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

#include <wendy/Config.hpp>

#include <wendy/UIDrawer.hpp>
#include <wendy/UILayer.hpp>
#include <wendy/UIWidget.hpp>
#include <wendy/UIBook.hpp>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Page::Page(Layer& layer, const char* text):
  Widget(layer),
  m_text(text)
{
}

const String& Page::text() const
{
  return m_text;
}

void Page::setText(const char* newText)
{
  m_text = newText;
  invalidate();
}

///////////////////////////////////////////////////////////////////////

Book::Book(Layer& layer):
  Widget(layer),
  m_activePage(nullptr)
{
  keyPressedSignal().connect(*this, &Book::onKey);
  buttonClickedSignal().connect(*this, &Book::onMouseButton);
  areaChangedSignal().connect(*this, &Book::onAreaChanged);
}

Page* Book::activePage() const
{
  return m_activePage;
}

void Book::setActivePage(Page* newPage)
{
  setActivePage(newPage, false);
}

SignalProxy1<void, Book&> Book::pageChangedSignal()
{
  return m_pageChangedSignal;
}

void Book::draw() const
{
  Drawer& drawer = layer().drawer();

  const Rect area = globalArea();
  if (drawer.pushClipArea(area))
  {
    if (!m_pages.empty())
    {
      const vec2 tabSize(area.size.x / m_pages.size(),
                         drawer.currentEM() * 2.f);

      Rect tabArea(area.position.x,
                   area.position.y + area.size.y - tabSize.y,
                   tabSize.x,
                   tabSize.y);

      for (auto p : m_pages)
      {
        WidgetState state;
        if (isEnabled())
        {
          if (m_activePage == p)
            state = STATE_ACTIVE;
          else
            state = STATE_NORMAL;
        }
        else
          state = STATE_DISABLED;

        drawer.drawTab(tabArea, state, p->text().c_str());

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
    const float em = layer().drawer().currentEM();

    page->setArea(Rect(0.f, 0.f, size().x, size().y - em * 2.f));

    if (m_activePage)
      page->hide();
    else
      setActivePage(page, false);

    m_pages.push_back(page);
  }
}

void Book::removedChild(Widget& child)
{
  if (Page* page = dynamic_cast<Page*>(&child))
  {
    if (page == m_activePage)
    {
      if (m_pages.empty())
        setActivePage(nullptr, false);
      else
        setActivePage(m_pages.front(), false);
    }

    m_pages.erase(std::find(m_pages.begin(), m_pages.end(), page));
  }
}

void Book::setActivePage(Page* newPage, bool notify)
{
  if (newPage == m_activePage)
    return;

  if (std::find(children().begin(), children().end(), newPage) == children().end())
    return;

  if (m_activePage)
    m_activePage->hide();

  m_activePage = newPage;
  m_activePage->show();
  m_activePage->activate();

  if (notify)
    m_pageChangedSignal(*this);

  invalidate();
}

void Book::onAreaChanged(Widget& widget)
{
  const float em = layer().drawer().currentEM();

  for (auto p : m_pages)
    p->setArea(Rect(0.f, 0.f, size().x, size().y - em * 2.f));
}

void Book::onKey(Widget& widget, Key key, Action action, uint mods)
{
  if (action != PRESSED)
    return;

  auto p = std::find(m_pages.begin(), m_pages.end(), m_activePage);
  if (p == m_pages.end())
    return;

  switch (key)
  {
    case KEY_TAB:
    case KEY_RIGHT:
    {
      if (++p == m_pages.end())
        setActivePage(m_pages.front(), true);
      else
        setActivePage(*p, true);
      break;
    }

    case KEY_LEFT:
    {
      if (p == m_pages.begin())
        setActivePage(m_pages.back(), true);
      else
        setActivePage(*(--p), true);
      break;
    }

    default:
      break;
  }
}

void Book::onMouseButton(Widget& widget,
                         vec2 point,
                         MouseButton button,
                         Action action,
                         uint mods)
{
  if (action != PRESSED)
    return;

  const float position = transformToLocal(point).x;
  const float tabWidth = width() / m_pages.size();

  const uint index = (uint) (position / tabWidth);

  if (m_pages[index] != m_activePage)
    setActivePage(m_pages[index], true);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
