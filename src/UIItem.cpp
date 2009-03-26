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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLShader.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderFont.h>

#include <wendy/Input.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>
#include <wendy/UIItem.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Item::Item(const String& initValue, unsigned int initID):
  value(initValue),
  ID(initID)
{
}

bool Item::operator < (const Item& other) const
{
  return value < other.value;
}

float Item::getWidth(void) const
{
  Renderer* renderer = Renderer::get();

  const float em = renderer->getDefaultEM();

  float width = em * 2.f;

  if (value.empty())
    width += em * 3.f;
  else
    width += renderer->getDefaultFont()->getTextMetrics(value).size.x;

  return width;
}

float Item::getHeight(void) const
{
  return Renderer::get()->getCurrentFont()->getHeight() * 1.5f;
}

unsigned int Item::getID(void) const
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
}

void Item::draw(const Rectangle& area, WidgetState state) const
{
  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    const float em = Renderer::get()->getDefaultEM();

    Rectangle textArea = area;
    textArea.position.x += em / 2.f;
    textArea.size.x -= em;

    if (state == STATE_SELECTED)
      renderer->fillRectangle(area, ColorRGBA(renderer->getSelectionColor(), 1.f));

    renderer->drawText(textArea, value, LEFT_ALIGNED, state);
    renderer->popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

SeparatorItem::SeparatorItem(void)
{
}

float SeparatorItem::getWidth(void) const
{
  const float em = Renderer::get()->getDefaultEM();

  return em * 3.f;
}

float SeparatorItem::getHeight(void) const
{
  const float em = Renderer::get()->getDefaultEM();

  return em / 2.f;
}

void SeparatorItem::draw(const Rectangle& area, WidgetState state) const
{
  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    Segment2 segment;
    segment.start.set(area.position.x, area.position.y + area.size.y / 2.f);
    segment.end.set(area.position.x + area.size.x, area.position.y + area.size.y / 2.f);

    renderer->drawLine(segment, ColorRGBA::BLACK);

    renderer->popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

TextureItem::TextureItem(GL::Texture& initTexture,
                         const String& name,
			 unsigned int ID):
  Item(name, ID),
  texture(&initTexture)
{
}

float TextureItem::getWidth(void) const
{
  const float em = Renderer::get()->getDefaultEM();

  return Item::getWidth() + em * 3.f;
}

float TextureItem::getHeight(void) const
{
  const float em = Renderer::get()->getDefaultEM();

  return em * 3.f;
}

GL::Texture& TextureItem::getTexture(void) const
{
  return *texture;
}

void TextureItem::draw(const Rectangle& area, WidgetState state) const
{
  Renderer* renderer = Renderer::get();
  if (renderer->pushClipArea(area))
  {
    const float em = renderer->getDefaultEM();

    if (state == STATE_SELECTED)
      renderer->fillRectangle(area, ColorRGBA(renderer->getSelectionColor(), 1.f));

    Rectangle textureArea = area;
    textureArea.size.set(em * 3.f, em * 3.f);

    renderer->blitTexture(textureArea, *texture);

    Rectangle textArea = area;
    textArea.position.x += em * 3.5f;
    textArea.size.x -= em;

    renderer->drawText(textArea, asString(), LEFT_ALIGNED, state);

    renderer->popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
