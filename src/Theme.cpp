//////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <nori/Core.hpp>
#include <nori/Rect.hpp>
#include <nori/Path.hpp>
#include <nori/Resource.hpp>
#include <nori/Pixel.hpp>
#include <nori/Image.hpp>

#include <nori/Texture.hpp>
#include <nori/Program.hpp>
#include <nori/RenderBuffer.hpp>
#include <nori/RenderContext.hpp>

#include <nori/Theme.hpp>

namespace nori
{

Theme::~Theme()
{
}

void Theme::beginLayer()
{
  RenderContext& rc = m_vc.context();
  Framebuffer& framebuffer = rc.framebuffer();
  const Recti area(0, 0, framebuffer.width(), framebuffer.height());
  rc.setViewportArea(area);
  m_stack.push(Rect(area));
  m_vc.beginFrame(area.size.x, area.size.y, 1.f);
  //m_vc.translate(vec2(0.f, float(area.size.y)));
  //m_vc.scale(vec2(1.f, -1.f));
}

void Theme::endLayer()
{
  m_vc.endFrame();
  m_stack.pop();
}

bool Theme::pushClipArea(Rect area)
{
  m_vc.intersectScissor(area);
  return m_stack.push(area);
}

void Theme::popClipArea()
{
  m_stack.pop();
  m_vc.scissor(m_stack.total());
}

void Theme::drawText(Rect area, WidgetState state, int alignment, const char* text)
{
  m_vc.font(m_font);
  m_vc.textAlign(alignment);
  m_vc.fillColor(vec4(1.f));
  m_vc.text(area.position + area.size / 2.f, text);
}

void Theme::drawWell(Rect area, WidgetState state)
{
}

void Theme::drawFrame(Rect area, WidgetState state)
{
  m_vc.beginPath();
  m_vc.rect(area);
  m_vc.fillColor(vec4(0.6f, 0.6f, 0.6f, 1.f));
  m_vc.fill();
}

void Theme::drawHandle(Rect area, WidgetState state)
{
}

void Theme::drawPushButton(Rect area, WidgetState state, const char* text)
{
  if (state == STATE_SELECTED)
  {
    m_vc.fillPaint(m_vc.linearGradient(area.position,
                                       area.position + vec2(0.f, area.size.y),
                                       vec4(0.8f, 0.8f, 0.8f, 1.f),
                                       vec4(0.6f, 0.6f, 0.6f, 1.f)));
  }
  else
  {
    m_vc.fillPaint(m_vc.linearGradient(area.position,
                                       area.position + vec2(0.f, area.size.y),
                                       vec4(0.6f, 0.6f, 0.6f, 1.f),
                                       vec4(0.8f, 0.8f, 0.8f, 1.f)));
  }

  m_vc.beginPath();
  m_vc.roundedRect(area, m_em / 2.f);
  m_vc.fill();

  drawText(area, state, ALIGN_CENTER | ALIGN_MIDDLE, text);
  /*
  drawElement(area, m_theme->m_buttonElements[state]);

  if (state == STATE_SELECTED)
  {
    const Rect textArea(area.position.x + 2.f,
                        area.position.y,
                        area.size.x - 2.f,
                        area.size.y - 2.f);

    drawText(textArea, text, Alignment(), state);
  }
  else
    drawText(area, text, Alignment(), state);
  */
}

void Theme::drawCheckButton(Rect area, WidgetState state, bool checked, const char* text)
{
  /*
  const float checkSize = min(area.size.x, area.size.y);
  const Rect checkArea(area.position + vec2(checkSize) * 0.2f,
                       vec2(checkSize) * 0.6f);

  if (checked)
    drawElement(checkArea, m_theme->m_checkElements[state]);
  else
    drawElement(checkArea, m_theme->m_clearElements[state]);

  const Rect textArea(area.position + vec2(checkSize, 0.f),
                      area.size - vec2(checkSize, 0.f));

  drawText(textArea, text, LEFT_ALIGNED, state);
  */
}

void Theme::drawPopup(Rect area, WidgetState state, const char* text)
{
  const Rect textArea(area.position + vec2(m_em / 2.f, 0.f),
                      area.size - vec2(m_em, 0.f));

  drawPushButton(area, state, "");
  drawText(textArea, state, ALIGN_LEFT | ALIGN_MIDDLE, text);
}

void Theme::drawTab(Rect area, WidgetState state, const char* text)
{
  if (state == STATE_ACTIVE)
  {
    m_vc.fillPaint(m_vc.linearGradient(area.position,
                                       area.position + vec2(0.f, area.size.y),
                                       vec4(0.8f, 0.8f, 0.8f, 1.f),
                                       vec4(0.6f, 0.6f, 0.6f, 1.f)));
  }
  else
  {
    m_vc.fillPaint(m_vc.linearGradient(area.position,
                                       area.position + vec2(0.f, area.size.y),
                                       vec4(0.6f, 0.6f, 0.6f, 1.f),
                                       vec4(0.8f, 0.8f, 0.8f, 1.f)));
  }

  m_vc.beginPath();
  m_vc.rect(area);
  m_vc.fill();

  drawText(area, state, ALIGN_CENTER | ALIGN_MIDDLE, text);
}

void Theme::drawSelection(Rect area, WidgetState state)
{
  m_vc.beginPath();
  m_vc.rect(area);
  m_vc.fillColor(vec4(0.8f, 0.8f, 0.8f, 1.f));
  m_vc.fill();
}

std::unique_ptr<Theme> Theme::create(VectorContext& context)
{
  std::unique_ptr<Theme> theme(new Theme(context));
  if (!theme->init())
    return nullptr;

  return theme;
}

Theme::Theme(VectorContext& context):
  m_vc(context)
{
}

bool Theme::init()
{
  ResourceCache& cache = m_vc.context().cache();

  const Path fontPath = cache.findFile("nori/Ubuntu-R.ttf");
  if (fontPath.isEmpty())
  {
    logError("Failed to find UI font");
    return false;
  }

  m_font = m_vc.createFont("default", fontPath);
  if (m_font == -1)
  {
    logError("Failed to load UI font");
    return false;
  }

  m_em = 20.f;
  m_vc.font(m_font);
  m_vc.fontSize(m_em);

  return true;
}

} /*namespace nori*/

