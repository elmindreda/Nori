///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

namespace nori
{

/*! @ingroup ui
 */
typedef uint ItemID;

/*! @ingroup ui
 */
const uint NO_ITEM = -1;

/*! @ingroup ui
 */
class Item
{
  friend class List;
  friend class Menu;
public:
  Item(Layer& layer, const std::string& value = "", ItemID id = 0);
  virtual ~Item();
  virtual bool operator < (const Item& other) const;
  virtual float width() const;
  virtual float height() const;
  virtual ItemID id() const;
  virtual const std::string& value() const;
  virtual void setValue(const std::string& value);
protected:
  virtual void draw(const Rect& area, WidgetState state) const;
  Layer& m_layer;
private:
  std::string m_value;
  ItemID m_id;
};

/*! @ingroup ui
 */
class SeparatorItem : public Item
{
public:
  SeparatorItem(Layer& layer);
  float width() const override;
  float height() const override;
protected:
  void draw(const Rect& area, WidgetState state) const;
};

/*! @ingroup ui
 */
class TextureItem : public Item
{
public:
  TextureItem(Layer& layer,
              Texture& texture,
              const std::string& name = "",
              ItemID id = 0);
  float width() const override;
  float height() const override;
  Texture& texture() const;
protected:
  void draw(const Rect& area, WidgetState state) const;
private:
  Ref<Texture> m_texture;
};

class ItemComparator
{
public:
  bool operator () (const Item* x, const Item* y)
  {
    return *x < *y;
  }
};

class ItemContainer
{
public:
  virtual ~ItemContainer() { }
  virtual void addItem(Item& item) = 0;
  virtual void insertItem(Item& item, uint index) = 0;
  virtual void destroyItem(Item& item) = 0;
  virtual void destroyItems() = 0;
  virtual Item* findItem(const std::string& value) = 0;
  virtual const Item* findItem(const std::string& value) const = 0;
  virtual void sortItems() = 0;
  virtual const std::vector<Item*>& items() const = 0;
};

} /*namespace nori*/

