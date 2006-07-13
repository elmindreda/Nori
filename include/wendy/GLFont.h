///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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
#ifndef WENDY_GLFONT_H
#define WENDY_GLFONT_H
///////////////////////////////////////////////////////////////////////

#include <list>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class TextureFont : public Managed<TextureFont>
{
public:
  void render(const Vector2& penPosition, const String& text) const;
  const Vector2& getPenPosition(void) const;
  void setPenPosition(const Vector2& newPosition);
  static TextureFont* createInstance(const Path& path,
				     const String& characters,
			             const String& name = "");
  static TextureFont* createInstance(const Font& font,
                                     const String& name = "");
private:
  class Glyph;
  TextureFont(const String& name);
  bool init(const Font& font);
  typedef std::list<Glyph> GlyphList;
  typedef std::map<char, Glyph*> GlyphMap;
  Font* font;
  GlyphList glyphs;
  GlyphMap glyphMap;
  Vector2 penPosition;
  Vector2i texelPosition;
  Ptr<Texture> texture;
};

///////////////////////////////////////////////////////////////////////

class TextureFont::Glyph
{
public:
  void render(const Vector2& penPosition);
  Rectangle area;
  const Font::Glyph* glyph;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLFONT_H*/
///////////////////////////////////////////////////////////////////////
