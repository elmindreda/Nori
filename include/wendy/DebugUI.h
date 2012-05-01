///////////////////////////////////////////////////////////////////////
// Wendy debug interface
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_DEBUGUI_H
#define WENDY_DEBUGUI_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace debug
  {

///////////////////////////////////////////////////////////////////////

class Panel : public UI::Widget
{
public:
  Panel(UI::Layer& layer);
private:
  void draw() const;
};

///////////////////////////////////////////////////////////////////////

class Interface : public UI::Layer
{
public:
  Interface(input::Window& window, UI::Drawer& drawer);
  void update();
  void draw();
private:
  enum Item
  {
    ITEM_FRAMERATE,
    ITEM_STATECHANGES,
    ITEM_OPERATIONS,
    ITEM_VERTICES,
    ITEM_POINTS,
    ITEM_LINES,
    ITEM_TRIANGLES,
    ITEM_TEXTURES,
    ITEM_VERTEXBUFFERS,
    ITEM_INDEXBUFFERS,
    ITEM_RENDERBUFFERS,
    ITEM_PROGRAMS,
    ITEM_COUNT
  };
  void updateCountItem(Item item, const char* unit, size_t count);
  void updateCountSizeItem(Item item, const char* unit, size_t count, size_t size);
  Panel* root;
  UI::Label* labels[ITEM_COUNT];
};

///////////////////////////////////////////////////////////////////////

  } /*namespace debug*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEBUGUI_H*/
///////////////////////////////////////////////////////////////////////
