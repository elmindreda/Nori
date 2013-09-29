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
#ifndef WENDY_RENDERFONT_HPP
#define WENDY_RENDERFONT_HPP
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
  FontData();
  FontData(const FontData& source);
  FontData& operator = (const FontData& source);
  std::vector<FontGlyphData> glyphs;
  int characters[256];
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Font layout and rendering object.
 *
 *  This class provides layout and rendering of a single font.
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
  float width() const { return m_size.x; }
  /*! @return The height, in pixels, of the character cell for this font.
   */
  float height() const { return m_size.y; }
  /*! @return The ascender for this font.
   */
  float ascender() const { return m_ascender; }
  /*! @return The descender for this font.
   */
  float descender() const { return m_descender; }
  /*! @param text The text to measure.
   *  @return The bounding rectangle, in pixels, of the specified text as
   *  rendered by this font.
   */
  Rect metricsOf(const char* text) const;
  /*! Calculates the layout of glyphs for the specified text.
   */
  LayoutList layoutOf(const char* text) const;
  static Ref<Font> create(const ResourceInfo& info,
                          VertexPool& pool,
                          const FontData& data);
  static Ref<Font> read(VertexPool& pool, const String& name);
private:
  class Glyph;
  Font(const ResourceInfo& info, VertexPool& pool);
  Font(const Font&) = delete;
  bool init(const FontData& font);
  const Glyph* findGlyph(uint8 character) const;
  bool getGlyphLayout(Layout& layout, uint8 character) const;
  void getGlyphLayout(Layout& layout, const Glyph& glyph, uint8 character) const;
  Font& operator = (const Font&) = delete;
  Ref<VertexPool> m_pool;
  std::vector<Glyph> m_glyphs;
  Glyph* m_characters[256];
  vec2 m_size;
  float m_ascender;
  float m_descender;
  UniformStateIndex m_colorIndex;
  Pass m_pass;
  std::vector<Vertex2ft2fv> m_vertices;
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
  FontReader(VertexPool& pool);
  using ResourceReader<Font>::read;
  Ref<Font> read(const String& name, const Path& path);
private:
  bool extractGlyphs(FontData& data,
                     const String& name,
                     const Image& image,
                     const String& characters,
                     bool fixedWidth);
  Ref<VertexPool> pool;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERFONT_HPP*/
///////////////////////////////////////////////////////////////////////
