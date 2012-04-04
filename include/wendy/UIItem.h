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

/*! @ingroup ui
 */
typedef unsigned int ItemID;

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
const unsigned int NO_ITEM = -1;

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class Item
{
  friend class List;
  friend class Menu;
public:
  Item(Layer& layer, const char* value = "", ItemID ID = 0);
  virtual ~Item();
  virtual bool operator < (const Item& other) const;
  virtual float getWidth() const;
  virtual float getHeight() const;
  virtual ItemID getID() const;
  virtual const String& asString() const;
  virtual void setStringValue(const char* newValue);
protected:
  virtual void draw(const Rect& area, WidgetState state) const;
  Layer& layer;
private:
  String value;
  ItemID ID;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
typedef std::vector<Item*> ItemList;

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class SeparatorItem : public Item
{
public:
  SeparatorItem(Layer& layer);
  float getWidth() const;
  float getHeight() const;
protected:
  void draw(const Rect& area, WidgetState state) const;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup ui
 */
class TextureItem : public Item
{
public:
  TextureItem(Layer& layer,
              GL::Texture& texture,
              const char* name = "",
              ItemID ID = 0);
  float getWidth() const;
  float getHeight() const;
  GL::Texture& getTexture() const;
protected:
  void draw(const Rect& area, WidgetState state) const;
private:
  Ref<GL::Texture> texture;
};

///////////////////////////////////////////////////////////////////////

class ItemComparator
{
public:
  bool operator () (const Item* x, const Item* y)
  {
    return *x < *y;
  }
};

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_UIITEM_H*/
///////////////////////////////////////////////////////////////////////
