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

#include <wendy/Config.h>
#include <wendy/Portability.h>
#include <wendy/Core.h>
#include <wendy/Block.h>
#include <wendy/Color.h>
#include <wendy/Vector.h>
#include <wendy/Rectangle.h>
#include <wendy/Path.h>
#include <wendy/Stream.h>
#include <wendy/Managed.h>
#include <wendy/Resource.h>
#include <wendy/Pixel.h>
#include <wendy/Image.h>
#include <wendy/Font.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

unsigned int findStartY(const Image& image)
{
  const Byte* pixels = (const Byte*) image.getPixels();

  unsigned int startY;

  for (startY = 0;  startY < image.getHeight();  startY++)
  {
    unsigned int x;

    for (x = 0;  x < image.getWidth();  x++)
    {
      if (pixels[x + startY * image.getWidth()] > 0)
	break;
    }

    if (x < image.getWidth())
      break;
  }

  return startY;
}

unsigned int findEndY(const Image& image)
{
  const Byte* pixels = (const Byte*) image.getPixels();

  unsigned int endY;

  for (endY = image.getHeight();  endY > 0;  endY--)
  {
    unsigned int x;

    for (x = 0;  x < image.getWidth();  x++)
    {
      if (pixels[x + (endY - 1) * image.getWidth()] > 0)
	break;
    }

    if (x < image.getWidth())
      break;
  }

  return endY;
}

}

///////////////////////////////////////////////////////////////////////

Font::Font(const Font& source):
  Resource<Font>(source)
{
  operator = (source);
}

Font& Font::operator = (const Font& source)
{
  glyphs = source.glyphs;

  for (GlyphMap::const_iterator i = source.glyphMap.begin();  i != source.glyphMap.end();  i++)
  {
    for (GlyphList::iterator j = glyphs.begin();  j != glyphs.end();  j++)
    {
      if ((*j).index == (*i).second->index)
      {
	glyphMap[(*i).first] = &(*j);
	break;
      }
    }
  }

  characters = source.characters;
  size = source.size;
  return *this;
}

float Font::getWidth(void) const
{
  return size.x;
}

float Font::getHeight(void) const
{
  return size.y;
}

const Font::Glyph* Font::getGlyph(char character) const
{
  GlyphMap::const_iterator i = glyphMap.find(character);
  if (i == glyphMap.end())
    return NULL;

  return (*i).second;
}

float Font::getKerning(char first, char second) const
{
  return 0.f;
}

const String& Font::getCharacters(void) const
{
  return characters;
}

Vec2 Font::getTextSize(const String& text) const
{
  return getTextMetrics(text).size;
}

Rect Font::getTextMetrics(const String& text) const
{
  Rect result(0.f, 0.f, 0.f, 0.f);

  Vec2 pen(0.f, 0.f);

  for (String::const_iterator c = text.begin();  c != text.end();  c++)
  {
    GlyphMap::const_iterator i = glyphMap.find(*c);
    if (i == glyphMap.end())
      continue;

    Glyph* glyph = (*i).second;

    Rect area;
    area.position.x = pen.x + glyph->bearing.x;
    area.position.y = pen.y - glyph->image->getHeight() + glyph->bearing.y;
    area.size.set((float) glyph->image->getWidth(),
                  (float) glyph->image->getHeight());

    result.envelop(area);

    pen.x += glyph->advance;
  }

  return result;
}

Font* Font::createInstance(const Image& image,
                           const String& characters,
                           const String& name)
{
  Ptr<Font> font(new Font(name));
  if (!font->init(image, characters))
    return NULL;

  return font.detachObject();
}

Font* Font::readInstance(const String& name)
{
  return Resource<Font>::readInstance(name);
}

Font* Font::readInstance(const Path& path,
                         const String& characters,
                         const String& name)
{
  Ref<Image> image = Image::readInstance(path);
  if (!image)
    return NULL;

  return createInstance(*image, characters, name);
}

Font* Font::readInstance(Stream& stream,
                         const String& characters,
                         const String& name)
{
  Ref<Image> image = Image::readInstance(stream);
  if (!image)
    return NULL;

  return createInstance(*image, characters, name);
}

Font::Font(const String& name):
  Resource<Font>(name),
  size(0.f, 0.f)
{
}

bool Font::init(const Image& image, const String& initCharacters)
{
  if (image.getFormat() != PixelFormat::R8)
  {
    Log::writeError("Invalid pixel format %s for font source image %s",
                    image.getFormat().asString().c_str(),
                    image.getName().c_str());
    return false;
  }


  Image source = image;

  characters = initCharacters;

  // Crop top and bottom parts
  {
    const unsigned int startY = findStartY(source);
    if (startY == source.getHeight())
    {
      Log::writeError("No glyphs found");
      return false;
    }

    const unsigned int endY = findEndY(source);

    if (!source.crop(Recti(0, startY, source.getWidth(), endY - startY)))
      return false;
  }

  const Byte* pixels = (const Byte*) source.getPixels();

  unsigned int index = 0, startX = 0, endX;

  for (;;)
  {
    // Find left edge of glyph, if any

    while (startX < source.getWidth())
    {
      unsigned int y;

      for (y = 0;  y < source.getHeight();  y++)
      {
	if (pixels[startX + y * source.getWidth()] > 0)
	  break;
      }

      if (y < source.getHeight())
	break;

      startX++;
    }

    if (startX == source.getWidth())
      break;

    if (index == characters.size())
    {
      Log::writeError("Not enough characters for font %s", getName().c_str());
      return false;
    }

    // Find right edge of glyph

    for (endX = startX + 1;  endX < source.getWidth();  endX++)
    {
      unsigned int y;

      for (y = 0;  y < source.getHeight();  y++)
      {
	if (pixels[endX + y * source.getWidth()] > 0)
	  break;
      }

      if (y == source.getHeight())
	break;
    }

    Image* image = source.getArea(Recti(startX, 0, endX - startX, source.getHeight()));
    if (!image)
      return false;

    if (image->getWidth() > size.x)
      size.x = (float) image->getWidth();

    if (image->getHeight() > size.y)
      size.y = (float) image->getHeight();

    glyphs.push_back(Glyph());
    Glyph& glyph = glyphs.back();

    glyphMap[characters[index]] = &glyph;

    glyph.bearing.set(0.f, image->getHeight() / 2.f);
    glyph.advance = (float) image->getWidth();
    glyph.index = index++;
    glyph.image = image;

    startX = endX;
  }

  // HACK: Make digits same width
  {
    std::vector<Glyph*> glyphs;

    float maxAdvance = 0.f;

    for (char c = '0';  c <= '9';  c++)
    {
      GlyphMap::iterator i = glyphMap.find(c);
      if (i == glyphMap.end())
	continue;

      Glyph* glyph = (*i).second;
      if (glyph->advance > maxAdvance)
	maxAdvance = glyph->advance;

      glyphs.push_back(glyph);
    }

    for (unsigned int i = 0;  i < glyphs.size();  i++)
    {
      Glyph* glyph = glyphs[i];

      glyph->bearing.x = (maxAdvance - glyph->advance) / 2.f;
      glyph->advance = maxAdvance;
    }
  }

  // HACK: Introduce 'tasteful' spacing
  {
    float meanAdvance = 0.f;

    for (GlyphList::const_iterator i = glyphs.begin();  i != glyphs.end();  i++)
      meanAdvance += (*i).advance;

    meanAdvance /= (float) glyphs.size();

    for (GlyphList::iterator i = glyphs.begin();  i != glyphs.end();  i++)
      (*i).advance += meanAdvance * 0.2f;

    if (characters.find(' ') == String::npos)
    {
      // HACK: Create space glyph if not already present

      glyphs.push_back(Glyph());
      Glyph& glyph = glyphs.back();

      glyphMap[' '] = &glyph;

      glyph.bearing.set(0.f, 0.f);
      glyph.advance = meanAdvance * 0.6f;
      glyph.index = index++;
      glyph.image = new Image(source.getFormat(), 1, 1);

      characters.push_back(' ');
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

Font::Glyph::Glyph(void)
{
}

Font::Glyph::Glyph(const Glyph& source)
{
  operator = (source);
}

Font::Glyph& Font::Glyph::operator = (const Glyph& source)
{
  bearing = source.bearing;
  advance = source.advance;
  index = source.index;

  if (source.image)
    image = new Image(*source.image);
  else
    image = NULL;

  return *this;
}

unsigned int Font::Glyph::getIndex(void) const
{
  return index;
}

const Image& Font::Glyph::getImage(void) const
{
  return *image;
}

const Vec2& Font::Glyph::getBearing(void) const
{
  return bearing;
}

float Font::Glyph::getAdvance(void) const
{
  return advance;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
