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

#include <list>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @brief %Font layout and rendering object.
 *
 *  This class provides layout and rendering of a single font, using data
 *  from a moira::Font object.
 */
class Font : public Managed<Font>
{
public:
  class Layout;
  typedef std::vector<Layout> LayoutList;
  /*! Renders the specified text at the current pen position.
   *  @param text The text to render.
   */
  void drawText(const String& text) const;
  /*! Renders the specified text at the current pen position.
   *  @param format The format string for the text to render.
   */
  void drawText(const char* format, ...) const;
  /*! @return The width, in pixels, of the character cell for this font.
   */
  float getWidth(void) const;
  /*! @return The height, in pixels, of the character cell for this font.
   */
  float getHeight(void) const;
  /*! @return The current pen position, in screen units.
   */
  const Vector2& getPenPosition(void) const;
  /*! Sets the current pen position.
   *  @param newPosition The desired pen position, in screen units.
   */
  void setPenPosition(const Vector2& newPosition);
  /*! @return The color and opacity values for glyphs drawn with this font.
   */
  const ColorRGBA& getColor(void) const;
  /*! Sets the color and opacity values for glyphs drawn with this font.
   *  @param newColor The desired color and opacity values.
   */
  void setColor(const ColorRGBA& newColor);
  float getAscender(void) const;
  float getDescender(void) const;
  /*! @param text The text to measure.
   *  @return The bounding rectangle, in pixels, of the specified text as
   *  rendered by this font.
   */
  Rectangle getTextMetrics(const String& text) const;
  /*! @param format The format string for the text to measure.
   *  @return The bounding rectangle, in pixels, of the specified text as
   *  rendered by this font.
   */
  Rectangle getTextMetrics(const char* format, ...) const;
  /*! Calculates the layout of glyphs for the specified text.
   */
  void getTextLayout(LayoutList& result, const String& text) const;
  /*! Calculates the layout of glyphs for the specified text.
   */
  void getTextLayout(LayoutList& result, const char* format, ...) const;
  static Font* createInstance(const Path& path,
			      const String& characters,
			      const String& name = "");
  static Font* createInstance(const moira::Font& font,
                              const String& name = "");
private:
  class Glyph;
  Font(const String& name);
  Font(const Font& source);
  Font& operator = (const Font& source);
  bool init(const moira::Font& font);
  const Glyph* getGlyph(char character) const;
  typedef std::list<Glyph> GlyphList;
  typedef std::map<char, Glyph*> GlyphMap;
  GlyphList glyphs;
  GlyphMap glyphMap;
  Vector2 penPosition;
  Vector2 size;
  float ascender;
  float descender;
  GL::Pass pass;
  Ref<GL::Texture> texture;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Glyph layout descriptor.
 */
class Font::Layout
{
public:
  Rectangle area;
  Vector2 penOffset;
  char character;
};

///////////////////////////////////////////////////////////////////////

/*! @internal
 */
class Font::Glyph
{
public:
  void draw(const Vector2& penPosition) const;
  Rectangle area;
  Vector2 bearing;
  Vector2 size;
  float advance;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERFONT_H*/
///////////////////////////////////////////////////////////////////////
