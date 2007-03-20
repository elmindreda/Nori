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
#ifndef WENDY_UIITEM_H
#define WENDY_UIITEM_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class Item
{
  friend class List;
  friend class Menu;
public:
  Item(const String& value = "");
  virtual bool operator < (const Item& other) const;
  virtual float getWidth(void) const;
  virtual float getHeight(void) const;
  virtual const String& getValue(void) const;
  virtual void setValue(const String& newValue);
protected:
  virtual void draw(const Rectangle& area, bool selected) const;
private:
  String value;
};

///////////////////////////////////////////////////////////////////////

typedef std::vector<Item*> ItemList;

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIITEM_H*/
///////////////////////////////////////////////////////////////////////
