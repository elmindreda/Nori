///////////////////////////////////////////////////////////////////////
// Wendy default renderer
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
#ifndef WENDY_RENDERFONT_H
#define WENDY_RENDERFONT_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

class FontGlyphData
{
public:
  vec2 bearing;
  float advance;
  Ref<Image> image;
};

///////////////////////////////////////////////////////////////////////

class FontData
{
public:
  typedef std::vector<FontGlyphData> GlyphList;
  FontData();
  FontData(const FontData& source);
  FontData& operator = (const FontData& source);
  GlyphList glyphs;
  int characters[256];
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Font layout and rendering object.
 *
 *  This class provides layout and rendering of a single font, using data
 *  from a wendy::Font object.
 */
class Font : public Resource
{
public:
  class Layout;
  typedef std::vector<Layout> LayoutList;
  /*! Renders the specified text at the current pen position.
   *  @param text The text to render.
   */
  void drawText(const vec2& penPosition, const vec4& color, const char* text);
  /*! @return The width, in pixels, of the character cell for this font.
   */
  float getWidth() const;
  /*! @return The height, in pixels, of the character cell for this font.
   */
  float getHeight() const;
  /*! @return The ascender for this font.
   */
  float getAscender() const;
  /*! @return The descender for this font.
   */
  float getDescender() const;
  /*! @param text The text to measure.
   *  @return The bounding rectangle, in pixels, of the specified text as
   *  rendered by this font.
   */
  Rect getTextMetrics(const char* text) const;
  /*! Calculates the layout of glyphs for the specified text.
   */
  void getTextLayout(LayoutList& result, const char* text) const;
  static Ref<Font> create(const ResourceInfo& info,
                          GeometryPool& pool,
                          const FontData& data);
  static Ref<Font> read(GeometryPool& pool, const String& name);
private:
  class Glyph;
  Font(const ResourceInfo& info, GeometryPool& pool);
  Font(const Font& source);
  Font& operator = (const Font& source);
  bool init(const FontData& font);
  const Glyph* findGlyph(uint8 character) const;
  bool getGlyphLayout(Layout& layout, uint8 character) const;
  void getGlyphLayout(Layout& layout, const Glyph& glyph, uint8 character) const;
  typedef std::vector<Glyph> GlyphList;
  typedef std::vector<Vertex2ft2fv> VertexList;
  Ref<GeometryPool> pool;
  GlyphList glyphs;
  Glyph* characters[256];
  vec2 size;
  float ascender;
  float descender;
  GL::UniformStateIndex colorIndex;
  GL::RenderState pass;
  VertexList vertices;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Glyph layout descriptor.
 */
class Font::Layout
{
public:
  Rect area;
  vec2 advance;
  char character;
};

///////////////////////////////////////////////////////////////////////

/*! @internal
 */
class Font::Glyph
{
public:
  Rect area;
  vec2 bearing;
  vec2 size;
  float advance;
};

///////////////////////////////////////////////////////////////////////

class FontReader : public ResourceReader<Font>
{
public:
  FontReader(GeometryPool& pool);
  using ResourceReader<Font>::read;
  Ref<Font> read(const String& name, const Path& path);
private:
  bool extractGlyphs(FontData& data,
                     const String& name,
                     const Image& image,
                     const String& characters,
                     bool fixedWidth);
  Ref<GeometryPool> pool;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERFONT_H*/
///////////////////////////////////////////////////////////////////////
