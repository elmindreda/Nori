///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
// Copyright (c) 2014 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <nori/Texture.hpp>
#include <nori/RenderBuffer.hpp>
#include <nori/Program.hpp>
#include <nori/RenderContext.hpp>
#include <nori/VectorContext.hpp>

#include <GREG/greg.h>

#include <nanovg.h>
#define NANOVG_GL2_IMPLEMENTATION
#include <nanovg_gl.h>

#include <utf8.h>

namespace nori
{

VectorContext::~VectorContext()
{
  if (m_nvg)
    nvgDeleteGL2(m_nvg);
}

void VectorContext::beginFrame(uint width, uint height, float aspectRatio)
{
  nvgBeginFrame(m_nvg, width, height, aspectRatio);
}

void VectorContext::endFrame()
{
  nvgEndFrame(m_nvg);
  m_context.invalidateStateCache();
}

void VectorContext::save()
{
  nvgSave(m_nvg);
}

void VectorContext::restore()
{
  nvgRestore(m_nvg);
}

void VectorContext::reset()
{
  nvgReset(m_nvg);
}

void VectorContext::strokeColor(vec4 color)
{
  nvgStrokeColor(m_nvg, *(NVGcolor*)&color);
}

void VectorContext::strokePaint(NVGpaint paint)
{
  return nvgStrokePaint(m_nvg, paint);
}

void VectorContext::strokeWidth(float size)
{
  nvgStrokeWidth(m_nvg, size);
}

void VectorContext::stroke()
{
  nvgStroke(m_nvg);
}

void VectorContext::fillColor(vec4 color)
{
  nvgFillColor(m_nvg, *(NVGcolor*)&color);
}

void VectorContext::fillPaint(NVGpaint paint)
{
  nvgFillPaint(m_nvg, paint);
}

void VectorContext::fill()
{
  nvgFill(m_nvg);
}

void VectorContext::miterLimit(float limit)
{
  nvgMiterLimit(m_nvg, limit);
}

void VectorContext::lineCap(uint cap)
{
  nvgLineCap(m_nvg, cap);
}

void VectorContext::lineJoin(uint join)
{
  nvgLineJoin(m_nvg, join);
}

void VectorContext::globalAlpha(float alpha)
{
  nvgGlobalAlpha(m_nvg, alpha);
}

void VectorContext::resetTransform()
{
  nvgResetTransform(m_nvg);
}

void VectorContext::transform(mat3x2 matrix)
{
  nvgTransform(m_nvg, matrix[0].x, matrix[0].y,
                      matrix[1].x, matrix[1].y,
                      matrix[2].x, matrix[2].y);
}

void VectorContext::translate(vec2 offset)
{
  nvgTranslate(m_nvg, offset.x, offset.y);
}

void VectorContext::rotate(float angle)
{
  nvgRotate(m_nvg, angle);
}

void VectorContext::skewX(float angle)
{
  nvgSkewX(m_nvg, angle);
}

void VectorContext::skewY(float angle)
{
  nvgSkewY(m_nvg, angle);
}

void VectorContext::scale(vec2 scale)
{
  nvgScale(m_nvg, scale.x, scale.y);
}

int VectorContext::createImage(Image& data, int flags)
{
  if (data.format() != PixelFormat::RGBA8)
  {
    logError("Vector context image data must be RGBA8");
    return 0;
  }

  return nvgCreateImageRGBA(m_nvg,
                            data.width(), data.height(), flags,
                            (const unsigned char*) data.pixels());
}

void VectorContext::deleteImage(int image)
{
  nvgDeleteImage(m_nvg, image);
}

NVGpaint VectorContext::linearGradient(vec2 start, vec2 end,
                                       vec4 innerColor, vec4 outerColor)
{
  return nvgLinearGradient(m_nvg,
                           start.x, start.y, end.x, end.y,
                           *(NVGcolor*) &innerColor, *(NVGcolor*) &outerColor);
}

NVGpaint VectorContext::boxGradient(Rect rect, float radius, float feather,
                                     vec4 innerColor, vec4 outerColor)
{
  nvgBoxGradient(m_nvg,
                 rect.position.x, rect.position.y, rect.size.x, rect.size.y,
                 radius, feather,
                 *(NVGcolor*) &innerColor, *(NVGcolor*) &outerColor);
}

NVGpaint VectorContext::radialGradient(vec2 center,
                                       float innerRadius, float outerRadius,
                                       vec4 innerColor, vec4 outerColor)
{
  nvgRadialGradient(m_nvg,
                    center.x, center.y, innerRadius, outerRadius,
                    *(NVGcolor*) &innerColor, *(NVGcolor*) &outerColor);
}

NVGpaint VectorContext::imagePattern(vec2 origin, vec2 size,
                                     float angle, int image, float alpha)
{
  return nvgImagePattern(m_nvg,
                         origin.x, origin.y, size.x, size.y,
                         angle, image, alpha);
}

void VectorContext::scissor(Rect area)
{
  nvgScissor(m_nvg, area.position.x, area.position.y, area.size.x, area.size.y);
}

void VectorContext::intersectScissor(Rect area)
{
  nvgIntersectScissor(m_nvg, area.position.x, area.position.y, area.size.x, area.size.y);
}

void VectorContext::resetScissor()
{
  nvgResetScissor(m_nvg);
}

void VectorContext::beginPath()
{
  nvgBeginPath(m_nvg);
}

void VectorContext::moveTo(vec2 point)
{
  nvgMoveTo(m_nvg, point.x, point.y);
}

void VectorContext::lineTo(vec2 point)
{
  nvgLineTo(m_nvg, point.x, point.y);
}

void VectorContext::closePath()
{
  nvgClosePath(m_nvg);
}

void VectorContext::rect(Rect area)
{
  nvgRect(m_nvg, area.position.x, area.position.y, area.size.x, area.size.y);
}

void VectorContext::roundedRect(Rect rect, float radius)
{
  nvgRoundedRect(m_nvg, rect.position.x, rect.position.y, rect.size.x, rect.size.y, radius);
}

void VectorContext::ellipse(vec2 center, vec2 radius)
{
  nvgEllipse(m_nvg, center.x, center.y, radius.x, radius.y);
}

void VectorContext::circle(vec2 center, float radius)
{
  nvgCircle(m_nvg, center.x, center.y, radius);
}

int VectorContext::createFont(const std::string& name, const Path& path)
{
  return nvgCreateFont(m_nvg, name.c_str(), path.name().c_str());
}

void VectorContext::font(int font)
{
  nvgFontFaceId(m_nvg, font);
}

void VectorContext::font(const std::string& name)
{
  nvgFontFace(m_nvg, name.c_str());
}

void VectorContext::fontSize(float size)
{
  nvgFontSize(m_nvg, size);
}

void VectorContext::textAlign(int align)
{
  nvgTextAlign(m_nvg, align);
}

void VectorContext::text(vec2 pen, const char* start, const char* end)
{
  nvgText(m_nvg, pen.x, pen.y, start, end);
}

void VectorContext::textBox(vec2 pen, float width, const char* start, const char* end)
{
  nvgTextBox(m_nvg, pen.x, pen.y, width, start, end);
}

Rect VectorContext::textBounds(vec2 pen, const char* start, const char* end)
{
  float bounds[4];
  nvgTextBounds(m_nvg, pen.x, pen.y, start, end, bounds);
  return Rect(bounds[0], bounds[1], bounds[2] - bounds[0], bounds[3] - bounds[1]);
}

Rect VectorContext::textBoxBounds(vec2 pen, float width, const char* start, const char* end)
{
  float bounds[4];
  nvgTextBoxBounds(m_nvg, pen.x, pen.y, width, start, end, bounds);
  return Rect(bounds[0], bounds[1], bounds[2] - bounds[0], bounds[3] - bounds[1]);
}

std::vector<NVGglyphPosition> VectorContext::textGlyphPositions(vec2 pen, const char* start, const char* end)
{
  std::vector<NVGglyphPosition> positions(utf8::distance(start, end));
  nvgTextGlyphPositions(m_nvg, pen.x, pen.y, start, end, positions.data(), positions.size());
  return positions;
}

std::unique_ptr<VectorContext> VectorContext::create(RenderContext& rc)
{
  std::unique_ptr<VectorContext> vc(new VectorContext(rc));
  if (!vc->init())
    return nullptr;

  return vc;
}

VectorContext::VectorContext(RenderContext& rc):
  m_context(rc)
{
}

bool VectorContext::init()
{
  m_nvg = nvgCreateGL2(0);
  if (!m_nvg)
  {
    logError("Failed to create NanoVG context");
    return false;
  }

  m_context.invalidateStateCache();
  return true;
}

} /*namespace nori*/

