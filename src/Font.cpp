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

#include <wendy/Core.h>
#include <wendy/Block.h>
#include <wendy/Color.h>
#include <wendy/Vector.h>
#include <wendy/Rectangle.h>
#include <wendy/Path.h>
#include <wendy/Resource.h>
#include <wendy/Pixel.h>
#include <wendy/XML.h>
#include <wendy/Image.h>
#include <wendy/Font.h>

#include <cstring>

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

const unsigned int FONT_XML_VERSION = 1;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Font::Font(const ResourceInfo& info):
  Resource(info, "Font")
{
  std::memset(characters, 0, sizeof(characters));
}

Font::Font(const Font& source):
  Resource(source)
{
  operator = (source);
}

Font& Font::operator = (const Font& source)
{
  glyphs = source.glyphs;

  std::memcpy(characters, source.characters, sizeof(characters));

  return *this;
}

///////////////////////////////////////////////////////////////////////

FontReader::FontReader(ResourceIndex& index):
  ResourceReader(index)
{
}

Ref<Font> FontReader::read(const Path& path)
{
  font = new Font(ResourceInfo(getIndex(), path));

  std::ifstream stream(path.asString().c_str());
  if (!stream)
    return NULL;

  if (!XML::Reader::read(stream))
  {
    font = NULL;
    return NULL;
  }

  return font.detachObject();
}

bool FontReader::extractGlyphs(const Image& image, const String& characters)
{
  if (image.getFormat() != PixelFormat::R8)
  {
    Log::writeError("Image \'%s\' for font \'%s\' has invalid pixel format \'%s\'",
                    image.getPath().asString().c_str(),
                    font->getPath().asString().c_str(),
                    image.getFormat().asString().c_str());
    return false;
  }

  Image source = image;

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
      Log::writeError("Font \'%s\' has less characters than glyphs",
                      font->getPath().asString().c_str());
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

    Ref<Image> glyphImage = source.getArea(Recti(startX, 0, endX - startX, source.getHeight()));
    if (!glyphImage)
      return false;

    font->glyphs.push_back(FontGlyph());
    FontGlyph& glyph = font->glyphs.back();
    glyph.bearing.set(0.f, glyphImage->getHeight() / 2.f);
    glyph.advance = (float) glyphImage->getWidth();
    glyph.image = glyphImage;

    font->characters[characters[index]] = &glyph;

    startX = endX;
  }

  // HACK: Make digits same width
  {
    std::vector<FontGlyph*> digitGlyphs;

    float maxAdvance = 0.f;

    for (char c = '0';  c <= '9';  c++)
    {
      FontGlyph* glyph = font->characters[c];
      if (!glyph)
        continue;

      if (glyph->advance > maxAdvance)
	maxAdvance = glyph->advance;

      digitGlyphs.push_back(glyph);
    }

    for (size_t i = 0;  i < digitGlyphs.size();  i++)
    {
      FontGlyph* glyph = digitGlyphs[i];

      glyph->bearing.x = (maxAdvance - glyph->advance) / 2.f;
      glyph->advance = maxAdvance;
    }
  }

  // HACK: Introduce 'tasteful' spacing
  {
    float meanAdvance = 0.f;

    for (size_t i = 0;  i < font->glyphs.size();  i++)
      meanAdvance += font->glyphs[i].advance;

    meanAdvance /= (float) font->glyphs.size();

    for (size_t i = 0;  i < font->glyphs.size();  i++)
      font->glyphs[i].advance += meanAdvance * 0.2f;

    if (!font->characters[' '])
    {
      // HACK: Create space glyph if not already present

      font->glyphs.push_back(FontGlyph());
      FontGlyph& glyph = font->glyphs.back();

      glyph.bearing.set(0.f, 0.f);
      glyph.advance = meanAdvance * 0.6f;
      glyph.image = new Image(getIndex(), source.getFormat(), 1, 1);

      font->characters[' '] = &glyph;
    }
  }

  return true;
}

bool FontReader::onBeginElement(const String& name)
{
  if (name == "font")
  {
    if (font)
    {
      Log::writeError("Only one font per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != FONT_XML_VERSION)
    {
      Log::writeError("Font specification XML format version mismatch");
      return false;
    }

    String characters = readString("characters");
    if (characters.empty())
    {
      Log::writeError("No characters specified for font");
      return false;
    }

    Path imagePath(readString("image"));

    ImageReader reader(getIndex());
    ImageRef image = reader.read(imagePath);
    if (!image)
    {
      Log::writeError("Cannot find image '%s' for font",
                      imagePath.asString().c_str());
      return false;
    }

    return true;
  }

  return true;
}

bool FontReader::onEndElement(const String& name)
{
  return true;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
