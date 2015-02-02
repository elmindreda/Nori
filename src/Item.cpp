//////////////////////////////////////////////////////////////////////
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

#include <nori/Config.hpp>

#include <nori/Theme.hpp>
#include <nori/Layer.hpp>
#include <nori/Widget.hpp>
#include <nori/Item.hpp>

namespace nori
{

Item::Item(Layer& layer, const std::string& value, ItemID id):
  m_layer(layer),
  m_value(value),
  m_id(id)
{
}

Item::~Item()
{
}

bool Item::operator < (const Item& other) const
{
  return m_value < other.m_value;
}

float Item::width() const
{
  Theme& theme = m_layer.theme();
  VectorContext& vc = theme.context();
  return theme.em() * 2.f + vc.textBounds(vec2(0.f), m_value.c_str()).size.x;
}

float Item::height() const
{
  return m_layer.theme().em() * 1.5f;
}

ItemID Item::id() const
{
  return m_id;
}

const std::string& Item::value() const
{
  return m_value;
}

void Item::setValue(const std::string& value)
{
  m_value = value;
  m_layer.invalidate();
}

void Item::draw(const Rect& area, WidgetState state) const
{
  Theme& theme = m_layer.theme();
  const float em = theme.em();
  const Rect textArea(area.position + vec2(em / 2.f, 0.f),
                      area.size - vec2(em, 0.f));

  if (state == STATE_SELECTED)
    theme.drawSelection(area, state);

  theme.drawText(textArea, state, ALIGN_LEFT | ALIGN_MIDDLE, m_value.c_str());
}

SeparatorItem::SeparatorItem(Layer& layer):
  Item(layer)
{
}

float SeparatorItem::width() const
{
  return m_layer.theme().em() * 2.f;
}

float SeparatorItem::height() const
{
  return m_layer.theme().em() / 2.f;
}

void SeparatorItem::draw(const Rect& area, WidgetState state) const
{
  VectorContext& context = m_layer.theme().context();
  context.beginPath();
  context.moveTo(vec2(area.position.x, area.position.y + area.size.y / 2.f));
  context.lineTo(vec2(area.position.x + area.size.x,
                      area.position.y + area.size.y / 2.f));
  context.fillColor(vec4(0.f, 0.f, 0.f, 1.f));
  context.fill();
}

ImageItem::ImageItem(Layer& layer, int image, const std::string& name, ItemID id):
  Item(layer, name, id),
  m_image(image)
{
}

float ImageItem::width() const
{
  return Item::width() + m_layer.theme().em() * 3.f;
}

float ImageItem::height() const
{
  return m_layer.theme().em() * 3.f;
}

void ImageItem::draw(const Rect& area, WidgetState state) const
{
  Theme& theme = m_layer.theme();
  VectorContext& context = theme.context();
  const float em = theme.em();
  const Rect textArea(area.position + vec2(em * 3.5f, 0.f),
                      area.size - vec2(em, 0.f));

  if (state == STATE_SELECTED)
    theme.drawSelection(area, state);

  theme.drawText(textArea, state, ALIGN_LEFT | ALIGN_MIDDLE, value().c_str());

  const Rect imageArea(area.position, vec2(em * 3.f));
  context.beginPath();
  context.rect(imageArea);
  context.fillPaint(context.imagePattern(imageArea, 0.f, m_image, 1.f));
  context.fill();
}

} /*namespace nori*/

