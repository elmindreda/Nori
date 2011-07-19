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
#include <wendy/UIDesktop.h>
#include <wendy/UIWidget.h>
#include <wendy/UIItem.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Item::Item(Desktop& initDesktop, const String& initValue, ItemID initID):
  desktop(initDesktop),
  value(initValue),
  ID(initID)
{
}

Item::~Item(void)
{
}

bool Item::operator < (const Item& other) const
{
  return value < other.value;
}

float Item::getWidth(void) const
{
  Drawer& drawer = desktop.getDrawer();

  const float em = drawer.getCurrentEM();

  float width = em * 2.f;

  if (value.empty())
    width += em * 3.f;
  else
    width += drawer.getCurrentFont().getTextMetrics(value.c_str()).size.x;

  return width;
}

float Item::getHeight(void) const
{
  return desktop.getDrawer().getCurrentFont().getHeight() * 1.5f;
}

ItemID Item::getID(void) const
{
  return ID;
}

const String& Item::asString(void) const
{
  return value;
}

void Item::setStringValue(const String& newValue)
{
  value = newValue;
  desktop.invalidate();
}

void Item::draw(const Rect& area, WidgetState state) const
{
  Drawer& drawer = desktop.getDrawer();
  if (drawer.pushClipArea(area))
  {
    const float em = drawer.getCurrentEM();

    Rect textArea = area;
    textArea.position.x += em / 2.f;
    textArea.size.x -= em;

    if (state == STATE_SELECTED)
    {
      const vec3 color = drawer.getTheme().textColors[STATE_SELECTED];
      drawer.fillRectangle(area, vec4(color, 1.f));
    }

    drawer.drawText(textArea, value, LEFT_ALIGNED, state);
    drawer.popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

SeparatorItem::SeparatorItem(Desktop& desktop):
  Item(desktop)
{
}

float SeparatorItem::getWidth(void) const
{
  const float em = desktop.getDrawer().getCurrentEM();

  return em * 3.f;
}

float SeparatorItem::getHeight(void) const
{
  const float em = desktop.getDrawer().getCurrentEM();

  return em / 2.f;
}

void SeparatorItem::draw(const Rect& area, WidgetState state) const
{
  Drawer& drawer = desktop.getDrawer();
  if (drawer.pushClipArea(area))
  {
    Segment2 segment;
    segment.start = vec2(area.position.x, area.position.y + area.size.y / 2.f);
    segment.end = vec2(area.position.x + area.size.x, area.position.y + area.size.y / 2.f);

    drawer.drawLine(segment, vec4(vec3(0.f), 1.f));

    drawer.popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

TextureItem::TextureItem(Desktop& desktop,
                         GL::Texture& initTexture,
                         const String& name,
			 ItemID ID):
  Item(desktop, name, ID),
  texture(&initTexture)
{
}

float TextureItem::getWidth(void) const
{
  const float em = desktop.getDrawer().getCurrentEM();

  return Item::getWidth() + em * 3.f;
}

float TextureItem::getHeight(void) const
{
  const float em = desktop.getDrawer().getCurrentEM();

  return em * 3.f;
}

GL::Texture& TextureItem::getTexture(void) const
{
  return *texture;
}

void TextureItem::draw(const Rect& area, WidgetState state) const
{
  Drawer& drawer = desktop.getDrawer();
  if (drawer.pushClipArea(area))
  {
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

    drawer.drawText(textArea, asString(), LEFT_ALIGNED, state);

    drawer.popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
