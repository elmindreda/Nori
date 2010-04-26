///////////////////////////////////////////////////////////////////////
// Wendy core library
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
#ifndef WENDY_FONT_H
#define WENDY_FONT_H
///////////////////////////////////////////////////////////////////////

#include <list>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class Stream;
class Image;

///////////////////////////////////////////////////////////////////////

class Font : public Resource<Font>, public RefObject
{
public:
  class Glyph;
  Font(const Font& source);
  Font& operator = (const Font& source);
  float getWidth(void) const;
  float getHeight(void) const;
  const Glyph* getGlyph(char character) const;
  float getKerning(char first, char second) const;
  const String& getCharacters(void) const;
  Vec2 getTextSize(const String& text) const;
  Rect getTextMetrics(const String& text) const;
  static Font* createInstance(const Image& image,
                              const String& characters,
			      const String& name = "");
  static Font* readInstance(const String& name);
  static Font* readInstance(const Path& path,
                            const String& characters,
			    const String& name = "");
  static Font* readInstance(Stream& stream,
                            const String& characters,
			    const String& name = "");
private:
  Font(const String& name);
  bool init(const Image& image, const String& characters);
  typedef std::list<Glyph> GlyphList;
  typedef std::map<char, Glyph*> GlyphMap;
  GlyphList glyphs;
  GlyphMap glyphMap;
  String characters;
  Vec2 size;
};

///////////////////////////////////////////////////////////////////////

class Font::Glyph
{
  friend class Font;
public:
  Glyph(void);
  Glyph(const Glyph& source);
  Glyph& operator = (const Glyph& source);
  unsigned int getIndex(void) const;
  const Image& getImage(void) const;
  const Vec2& getBearing(void) const;
  float getAdvance(void) const;
private:
  Vec2 bearing;
  float advance;
  unsigned int index;
  Ref<Image> image;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_FONT_H*/
///////////////////////////////////////////////////////////////////////
