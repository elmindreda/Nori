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
#include <wendy/UIItem.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Item::Item(Layer& initLayer, const char* initValue, ItemID initID):
  layer(initLayer),
  value(initValue),
  ID(initID)
{
}

Item::~Item()
{
}

bool Item::operator < (const Item& other) const
{
  return value < other.value;
}

float Item::getWidth() const
{
  Drawer& drawer = layer.getDrawer();

  const float em = drawer.getCurrentEM();

  float width = em * 2.f;

  if (value.empty())
    width += em * 3.f;
  else
    width += drawer.getCurrentFont().getTextMetrics(value.c_str()).size.x;

  return width;
}

float Item::getHeight() const
{
  return layer.getDrawer().getCurrentFont().getHeight() * 1.5f;
}

ItemID Item::getID() const
{
  return ID;
}

const String& Item::asString() const
{
  return value;
}

void Item::setStringValue(const char* newValue)
{
  value = newValue;
  layer.invalidate();
}

void Item::draw(const Rect& area, WidgetState state) const
{
  Drawer& drawer = layer.getDrawer();

  const float em = drawer.getCurrentEM();

  Rect textArea = area;
  textArea.position.x += em / 2.f;
  textArea.size.x -= em;

  if (state == STATE_SELECTED)
  {
    const vec3 color = drawer.getTheme().backColors[STATE_SELECTED];
    drawer.fillRectangle(area, vec4(color, 1.f));
  }

  drawer.drawText(textArea, value.c_str(), LEFT_ALIGNED, state);
}

///////////////////////////////////////////////////////////////////////

SeparatorItem::SeparatorItem(Layer& layer):
  Item(layer)
{
}

float SeparatorItem::getWidth() const
{
  const float em = layer.getDrawer().getCurrentEM();

  return em * 3.f;
}

float SeparatorItem::getHeight() const
{
  const float em = layer.getDrawer().getCurrentEM();

  return em / 2.f;
}

void SeparatorItem::draw(const Rect& area, WidgetState state) const
{
  Drawer& drawer = layer.getDrawer();

  Segment2 segment;
  segment.start = vec2(area.position.x, area.position.y + area.size.y / 2.f);
  segment.end = vec2(area.position.x + area.size.x, area.position.y + area.size.y / 2.f);

  drawer.drawLine(segment, vec4(vec3(0.f), 1.f));
}

///////////////////////////////////////////////////////////////////////

TextureItem::TextureItem(Layer& layer,
                         GL::Texture& initTexture,
                         const char* name,
                         ItemID ID):
  Item(layer, name, ID),
  texture(&initTexture)
{
}

float TextureItem::getWidth() const
{
  const float em = layer.getDrawer().getCurrentEM();

  return Item::getWidth() + em * 3.f;
}

float TextureItem::getHeight() const
{
  const float em = layer.getDrawer().getCurrentEM();

  return em * 3.f;
}

GL::Texture& TextureItem::getTexture() const
{
  return *texture;
}

void TextureItem::draw(const Rect& area, WidgetState state) const
{
  Drawer& drawer = layer.getDrawer();

  const float em = drawer.getCurrentEM();

  if (state == STATE_SELECTED)
  {
    const vec3 color = drawer.getTheme().textColors[STATE_SELECTED];
    drawer.fillRectangle(area, vec4(color, 1.f));
  }

  Rect textureArea = area;
  textureArea.size = vec2(em * 3.f, em * 3.f);

  drawer.blitTexture(textureArea, *texture);

  Rect textArea = area;
  textArea.position.x += em * 3.5f;
  textArea.size.x -= em;

  drawer.drawText(textArea, asString().c_str(), LEFT_ALIGNED, state);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
