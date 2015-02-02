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

#pragma once

#include <nori/Core.hpp>
#include <nori/Rect.hpp>
#include <nori/Path.hpp>
#include <nori/Resource.hpp>
#include <nori/Pixel.hpp>
#include <nori/Image.hpp>

#include <nanovg.h>

#include <memory>

namespace nori
{

class RenderContext;

enum
{
  ALIGN_LEFT     = 0x01,
  ALIGN_CENTER   = 0x02,
  ALIGN_RIGHT    = 0x04,
  ALIGN_TOP      = 0x08,
  ALIGN_MIDDLE   = 0x10,
  ALIGN_BOTTOM   = 0x20,
  ALIGN_BASELINE = 0x40,
};

class VectorContext
{
public:
  ~VectorContext();
  void beginFrame(uint width, uint height, float aspectRatio);
  void endFrame();
  void save();
  void restore();
  void reset();
  void beginPath();
  void strokeColor(vec4 color);
  void strokePaint(NVGpaint paint);
  void strokeWidth(float size);
  void stroke();
  void fillColor(vec4 color);
  void fillPaint(NVGpaint paint);
  void fill();
  void miterLimit(float limit);
  void lineCap(uint cap);
  void lineJoin(uint join);
  void globalAlpha(float alpha);
  void resetTransform();
  void transform(mat3x2 matrix);
  void translate(vec2 offset);
  void rotate(float angle);
  void skewX(float angle);
  void skewY(float angle);
  void scale(vec2 scale);
  int createImage(Image& data, int flags);
  void deleteImage(int handle);
  NVGpaint linearGradient(vec2 start, vec2 end, vec4 innerColor, vec4 outerColor);
  NVGpaint boxGradient(Rect rect, float radius, float feather, vec4 innerColor, vec4 outerColor);
  NVGpaint radialGradient(vec2 center, float innerRadius, float outerRadius, vec4 innerColor, vec4 outerColor);
  NVGpaint imagePattern(Rect area, float angle, int image, float alpha);
  void scissor(Rect rect);
  void intersectScissor(Rect rect);
  void resetScissor();
  void moveTo(vec2 point);
  void lineTo(vec2 point);
  void closePath();
  void rect(Rect rect);
  void roundedRect(Rect rect, float radius);
  void ellipse(vec2 center, vec2 radius);
  void circle(vec2 center, float radius);
  int createFont(const std::string& name, const Path& path);
  void font(int font);
  void font(const std::string& name);
  void fontSize(float size);
  void textAlign(int align);
  void text(vec2 pen, const char* start, const char* end = nullptr);
  void textBox(vec2 pen, float width, const char* start, const char* end = nullptr);
  Rect textBounds(vec2 pen, const char* start, const char* end = nullptr);
  Rect textBoxBounds(vec2 pen, float width, const char* start, const char* end = nullptr);
  std::vector<NVGglyphPosition> textGlyphPositions(vec2 pen, const char* start, const char* end = nullptr);
  RenderContext& context() const { return m_context; }
  static std::unique_ptr<VectorContext> create(RenderContext& rc);
private:
  VectorContext(RenderContext& rc);
  bool init();
  RenderContext& m_context;
  NVGcontext* m_nvg;
};

} /*namespace nori*/

