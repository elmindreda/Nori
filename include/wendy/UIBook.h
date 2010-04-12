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
#ifndef WENDY_UIBOOK_H
#define WENDY_UIBOOK_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Book;

///////////////////////////////////////////////////////////////////////

class Page : public Widget
{
public:
  Page(Desktop& desktop, Book& book, const String& text);
  const String& getText(void) const;
  void setText(const String& newText);
protected:
  void draw(void) const;
private:
  String text;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class Book : public Widget
{
public:
  Book(Desktop& desktop, Widget* parent = NULL);
  Page* getActivePage(void) const;
  void setActivePage(Page* newPage);
  SignalProxy1<void, Book&> getPageChangedSignal(void);
protected:
  void draw(void) const;
  void addedChild(Widget& child);
  void removedChild(Widget& child);
private:
  typedef std::vector<Page*> PageList;
  void getPages(PageList& pages) const;
  void setActivePage(Page* newPage, bool notify);
  void onAreaChanged(Widget& widget);
  void onKeyPressed(Widget& widgeth, input::Key key, bool pressed);
  void onButtonClicked(Widget& widget,
		       const Vec2& position,
		       input::Button button,
		       bool clicked);
  Signal1<void, Book&> pageChangedSignal;
  Page* activePage;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIBOOK_H*/
///////////////////////////////////////////////////////////////////////
