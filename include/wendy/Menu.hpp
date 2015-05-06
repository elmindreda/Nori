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
class Menu : public Widget, public ItemContainer
{
public:
  Menu(Layer& layer);
  ~Menu();
  void display(vec2 point);
  void display();
  void addItem(Item& item) override;
  void insertItem(Item& item, uint index) override;
  void createItem(const std::string& value, ItemID id = 0);
  void createSeparatorItem();
  Item* findItem(const std::string& value) override;
  const Item* findItem(const std::string& value) const override;
  void destroyItem(Item& item) override;
  void destroyItems() override;
  void sortItems() override;
  const std::vector<Item*>& items() const override { return m_items; }
  SignalProxy<void,Menu&,uint> itemSelected() { return m_itemSelected; }
private:
  void draw() const;
  void onFocusChanged(bool activated) override;
  void onCursorPos(vec2 point) override;
  void onCursorLeft() override;
  void onMouseButton(vec2 point,
                     MouseButton button,
                     Action action,
                     uint mods) override;
  void onKey(Key key, Action action, uint mods) override;
  void onDragEnded(vec2 point, MouseButton button) override;
  void sizeToFit();
  std::vector<Item*> m_items;
  int m_selection;
  Signal<void,Menu&,uint> m_itemSelected;
};

} /*namespace nori*/

