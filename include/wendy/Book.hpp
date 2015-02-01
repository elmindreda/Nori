///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#pragma once

namespace wendy
{

class Book;

/*! @ingroup ui
 */
class Page : public Widget
{
public:
  Page(Layer& layer, Book& parent, const char* text = "");
  ~Page();
  const std::string& text() const { return m_text; }
  void setText(const char* newText);
private:
  Book& m_book;
  std::string m_text;
};

/*! @ingroup ui
 */
class Book : public Widget
{
  friend class Page;
public:
  Book(Layer& layer, Widget* parent = nullptr);
  ~Book();
  Page* activePage() const { return m_activePage; }
  void setActivePage(Page* newPage);
  SignalProxy<void, Book&> pageChangedSignal();
protected:
  void draw() const;
  void onPageAdded(Page& page);
  void onPageRemoved(Page& page);
  void onAreaChanged() override;
  void onKey(Key key, Action action, uint mods) override;
  void onMouseButton(vec2 point,
                     MouseButton button,
                     Action action,
                     uint mods) override;
private:
  void setActivePage(Page* newPage, bool notify);
  Signal<void, Book&> m_pageChangedSignal;
  Page* m_activePage;
  std::vector<Page*> m_pages;
};

} /*namespace wendy*/

