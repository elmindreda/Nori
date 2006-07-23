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

#include <moira/Moira.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLRender.h>
#include <wendy/GLFont.h>

#include <cstdlib>

#if MOIRA_HAVE_STDARG_H
#include <stdarg.h>
#endif

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{

unsigned int getNextPower(unsigned int value)
{
  unsigned int count = 0;

  while (value >>= 1)
    count++;

  return 1 << (count + 1);
}

}
  
///////////////////////////////////////////////////////////////////////
  
void Font::drawText(const String& format, ...) const
{
  va_list vl;
  char* text;

  va_start(vl, format);
  vasprintf(&text, format.c_str(), vl);
  va_end(vl);
  
  RenderPass pass;
  pass.setDepthTesting(false);
  pass.setDepthWriting(false);
  pass.setDefaultColor(color);
  pass.setTextureName(texture->getName());
  pass.setCombineMode(GL_MODULATE);
  pass.setBlendFactors(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  pass.apply();

  Vector2 pen = penPosition;

  for (const char* c = text;  *c != '\0';  c++)
  {
    switch (*c)
    {
      case '\t':
      {
	pen.x += size.x * 3;
	break;
      }
	
      case '\n':
      {
	pen.x = penPosition.x;
	pen.y = pen.y - size.y * 1.2f;
	break;
      }

      default:
      {
	GlyphMap::const_iterator i = glyphMap.find(*c);
	if (i == glyphMap.end())
	  continue;

	Glyph* glyph = (*i).second;

	if (*c != ' ')
	  glyph->render(pen);

	pen.x += glyph->advance;
	break;
      }
    }

    pen.x = floorf(pen.x + 0.5f);
    pen.y = floorf(pen.y + 0.5f);
  }
}

float Font::getWidth(void) const
{
  return size.x;
}

float Font::getHeight(void) const
{
  return size.y;
}

const Vector2& Font::getPenPosition(void) const
{
  return penPosition;
}

void Font::setPenPosition(const Vector2& newPosition)
{
  penPosition = newPosition;
}

const ColorRGBA& Font::getColor(void) const
{
  return color;
}

void Font::setColor(const ColorRGBA& newColor)
{
  color = newColor;
}

Vector2 Font::getTextSize(const String& format, ...) const
{
  va_list vl;
  char* text;

  va_start(vl, format);
  vasprintf(&text, format.c_str(), vl);
  va_end(vl);
  
  return getTextMetrics(text).size;
}

Rectangle Font::getTextMetrics(const String& format, ...) const
{
  va_list vl;
  char* text;

  va_start(vl, format);
  vasprintf(&text, format.c_str(), vl);
  va_end(vl);
  
  Rectangle result(penPosition, Vector2::ZERO);

  Vector2 pen = penPosition;

  for (const char* c = text;  *c != '\0';  c++)
  {
    switch (*c)
    {
      case '\t':
      {
	pen.x += size.x * 3;
	break;
      }
	
      case '\n':
      {
	pen.x = penPosition.x;
	pen.y = pen.y - size.y * 1.2f;
	break;
      }

      default:
      {
	GlyphMap::const_iterator i = glyphMap.find(*c);
	if (i == glyphMap.end())
	  continue;

	Glyph* glyph = (*i).second;

	Rectangle area;
	area.position.x = pen.x + glyph->bearing.x;
	area.position.y = pen.y - glyph->size.y + glyph->bearing.y;
	area.size.set((float) glyph->size.x,
		      (float) glyph->size.y);

	result.envelop(area);

	pen.x += glyph->advance;
	break;
      }
    }

    pen.x = floorf(pen.x + 0.5f);
    pen.y = floorf(pen.y + 0.5f);
  }

  return result;
}

Font* Font::createInstance(const Path& path,
			   const String& characters,
			   const String& name)
{
  Ptr<moira::Font> font = moira::Font::readInstance(path, characters);
  if (!font)
    return NULL;

  return createInstance(*font, name);
}

Font* Font::createInstance(const moira::Font& font, const String& name)
{
  Ptr<Font> textureFont = new Font(name);
  if (!textureFont->init(font))
    return NULL;

  return textureFont.detachObject();
}

Font::Font(const String& name):
  Managed<Font>(name),
  color(ColorRGBA::WHITE)
{
}

bool Font::init(const moira::Font& font)
{
  const String& characters = font.getCharacters();

  unsigned int maxSize;

  glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*) &maxSize);

  const unsigned int glyphWidth = (unsigned int) ceilf(font.getWidth()) + 1;
  const unsigned int glyphHeight = (unsigned int) ceilf(font.getHeight()) + 1;

  // Create texture
  {
    unsigned int width = glyphWidth * characters.size() + 1;
    width = std::min(getNextPower(width), maxSize);

    unsigned int rows = characters.size() * glyphWidth / (width - 1);
    if (glyphWidth % (width - 1))
      rows++;

    unsigned int height = glyphHeight * rows + 1;
    height = std::min(getNextPower(height), maxSize);

    texture = Texture::createInstance(Image(ImageFormat::ALPHA8, width, height), 0);
    if (!texture)
      return false;
  }

  texelPosition.set(1, 1);

  for (unsigned int i = 0;  i < characters.size();  i++)
  {
    glyphs.push_back(Glyph());
    Glyph& glyph = glyphs.back();
    glyphMap[characters[i]] = &glyph;

    const moira::Font::Glyph* sourceGlyph = font.getGlyph(characters[i]);
    if (!sourceGlyph)
    {
      Log::writeError("No glyph for character %c", characters[i]);
      return false;
    }

    const Image& image = sourceGlyph->getImage();

    glyph.advance = sourceGlyph->getAdvance();
    glyph.bearing = sourceGlyph->getBearing();
    glyph.size.set((float) image.getWidth(), (float) image.getHeight());

    if (texelPosition.x + image.getWidth() + 2 > texture->getPhysicalWidth())
    {
      texelPosition.x = 1;
      texelPosition.y += (int) glyphHeight;

      if (texelPosition.y + image.getHeight() + 2 > texture->getPhysicalHeight())
      {
	// TODO: Allocate new texture.
	Log::writeError("No more room in font texture");
	return false;
      }
    }

    if (!texture->copyFrom(image, texelPosition.x, texelPosition.y))
      return false;

    glyph.area.position.set(texelPosition.x / (float) texture->getPhysicalWidth(),
			    texelPosition.y / (float) texture->getPhysicalHeight());
    glyph.area.size.set(image.getWidth() / (float) texture->getPhysicalWidth(),
			image.getHeight() / (float) texture->getPhysicalHeight());

    texelPosition.x += image.getWidth() + 1;
  }

  size.set(font.getWidth(), font.getHeight());
  return true;
}

///////////////////////////////////////////////////////////////////////

void Font::Glyph::render(const Vector2& penPosition)
{
  const Rectangle& texelArea = area;

  Rectangle pixelArea;
  pixelArea.position = penPosition;
  pixelArea.position.y += bearing.y - size.y;
  pixelArea.size = size;
    
  glBegin(GL_QUADS);
  glTexCoord2f(texelArea.position.x, texelArea.position.y);
  glVertex2f(pixelArea.position.x, pixelArea.position.y);
  glTexCoord2f(texelArea.position.x + texelArea.size.x, texelArea.position.y);
  glVertex2f(pixelArea.position.x + pixelArea.size.x, pixelArea.position.y);
  glTexCoord2f(texelArea.position.x + texelArea.size.x, texelArea.position.y + texelArea.size.y);
  glVertex2f(pixelArea.position.x + pixelArea.size.x, pixelArea.position.y + pixelArea.size.y);
  glTexCoord2f(texelArea.position.x, texelArea.position.y + texelArea.size.y);
  glVertex2f(pixelArea.position.x, pixelArea.position.y + pixelArea.size.y);
  glEnd();    
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
