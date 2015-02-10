///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2013 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/Config.hpp>

#include <wendy/Core.hpp>
#include <wendy/Rect.hpp>
#include <wendy/Path.hpp>
#include <wendy/Pixel.hpp>
#include <wendy/Resource.hpp>
#include <wendy/Image.hpp>
#include <wendy/Face.hpp>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <fstream>

namespace wendy
{

Face::~Face()
{
  if (m_info)
    delete m_info;
}

float Face::scale(uint height) const
{
  return stbtt_ScaleForPixelHeight(m_info, float(height));
}

float Face::ascender(float scale) const
{
  int ascender;
  stbtt_GetFontVMetrics(m_info, &ascender, nullptr, nullptr);
  return ascender * scale;
}

float Face::descender(float scale) const
{
  int descender;
  stbtt_GetFontVMetrics(m_info, nullptr, &descender, nullptr);
  return descender * scale;
}

float Face::leading(float scale) const
{
  int ascender, descender, leading;
  stbtt_GetFontVMetrics(m_info, &ascender, &descender, &leading);
  return (ascender - descender + leading) * scale;
}

float Face::width(float scale) const
{
  int left, top, right, bottom;
  stbtt_GetFontBoundingBox(m_info, &left, &top, &right, &bottom);
  return (right - left + 1) * scale;
}

float Face::height(float scale) const
{
  int left, top, right, bottom;
  stbtt_GetFontBoundingBox(m_info, &left, &top, &right, &bottom);
  return (bottom - top + 1) * scale;
}

int Face::indexForCodePoint(uint32 codepoint)
{
  return stbtt_FindGlyphIndex(m_info, codepoint);
}

float Face::advance(int index, float scale) const
{
  int advance;
  stbtt_GetGlyphHMetrics(m_info, index, &advance, nullptr);
  return advance * scale;
}

vec2 Face::bearing(int index, float scale) const
{
  int left, top, right, bottom;
  stbtt_GetGlyphBitmapBox(m_info, index, scale, scale,
                          &left, &top, &right, &bottom);
  return vec2(left, -bottom);
}

float Face::advance(int first, int second, float scale) const
{
  return stbtt_GetGlyphKernAdvance(m_info, first, second) * scale;
}

float Face::width(int index, float scale) const
{
  int left, top, right, bottom;
  stbtt_GetGlyphBitmapBox(m_info, index, scale, scale,
                          &left, &top, &right, &bottom);
  return float(right - left + 1);
}

float Face::height(int index, float scale) const
{
  int left, top, right, bottom;
  stbtt_GetGlyphBitmapBox(m_info, index, scale, scale,
                          &left, &top, &right, &bottom);
  return float(bottom - top + 1);
}

Ref<Image> Face::glyph(int index, float scale) const
{
  if (stbtt_IsGlyphEmpty(m_info, index))
    return nullptr;

  int left, top, right, bottom;
  stbtt_GetGlyphBitmapBox(m_info, index, scale, scale,
                          &left, &top, &right, &bottom);

  Ref<Image> glyph = Image::create(ResourceInfo(cache()),
                                   PixelFormat::L8,
                                   right - left, bottom - top);

  stbtt_MakeGlyphBitmap(m_info, (unsigned char*) glyph->pixels(),
                        glyph->width(), glyph->height(), glyph->width(),
                        scale, scale, index);

  glyph->flipHorizontal();

  return glyph;
}

Ref<Face> Face::create(const ResourceInfo& info, const char* data, size_t size)
{
  Ref<Face> face = new Face(info);
  if (!face->init(data, size))
    return nullptr;

  return face;
}

Ref<Face> Face::read(ResourceCache& cache, const std::string& name)
{
  if (Face* cached = cache.find<Face>(name))
    return cached;

  const Path path = cache.findFile(name);
  if (path.isEmpty())
  {
    logError("Failed to find face %s", name.c_str());
    return nullptr;
  }

  std::ifstream stream(path.name(), std::ios::in | std::ios::binary);
  if (stream.fail())
  {
    logError("Failed to open face file %s", path.name().c_str());
    return nullptr;
  }

  std::vector<char> data;

  stream.seekg(0, std::ios::end);
  data.resize((size_t) stream.tellg());

  stream.seekg(0, std::ios::beg);
  stream.read(data.data(), data.size());

  return create(ResourceInfo(cache, name, path), data.data(), data.size());
}

Face::Face(const ResourceInfo& info):
  Resource(info),
  m_info(nullptr)
{
}

bool Face::init(const char* data, size_t size)
{
  m_data.assign(data, data + size);
  m_info = new stbtt_fontinfo;

  if (!stbtt_InitFont(m_info, (unsigned char*) m_data.data(), 0))
  {
    logError("Failed to parse TrueType face file");
    return false;
  }

  return true;
}

} /*namespace wendy*/

