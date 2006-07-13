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

#include <moira/Config.h>
#include <moira/Core.h>
#include <moira/Log.h>
#include <moira/Color.h>
#include <moira/Vector.h>
#include <moira/Rectangle.h>
#include <moira/Image.h>
#include <moira/Font.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLShader.h>
#include <wendy/GLFont.h>

#include <cstdlib>

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
  
void TextureFont::render(const Vector2& penPosition, const String& text) const
{
  ShaderPass pass;
  pass.setDepthTesting(false);
  pass.setDepthWriting(false);
  pass.setTextureName(texture->getName());
  pass.setCombineMode(GL_MODULATE);
  pass.setBlendFactors(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  pass.apply();

  Vector2 pen = penPosition;

  for (String::const_iterator c = text.begin();  c != text.end();  c++)
  {
    GlyphMap::const_iterator i = glyphMap.find(*c);
    if (i == glyphMap.end())
      continue;

    Glyph* glyph = (*i).second;

    glyph->render(pen);
    pen.x += glyph->glyph->getAdvance();
  }
}

const Vector2& TextureFont::getPenPosition(void) const
{
  return penPosition;
}

void TextureFont::setPenPosition(const Vector2& newPosition)
{
  penPosition = newPosition;
}

TextureFont* TextureFont::createInstance(const Path& path,
				         const String& characters,
			                 const String& name)
{
  Font* font = Font::createInstance(path, characters);
  if (!font)
    return NULL;

  return createInstance(*font, name);
}

TextureFont* TextureFont::createInstance(const Font& font, const String& name)
{
  Ptr<TextureFont> textureFont = new TextureFont(name);
  if (!textureFont->init(font))
    return NULL;

  return textureFont.detachObject();
}

TextureFont::TextureFont(const String& name):
  Managed<TextureFont>(name)
{
}

bool TextureFont::init(const Font& font)
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

    Log::writeInformation("Texture %ux%u", width, height);

    Image image(ImageFormat::ALPHA8, width, height);

    texture = Texture::createInstance("", image, 0);
    if (!texture)
      return false;
  }

  texelPosition.set(1, 1);

  for (unsigned int i = 0;  i < characters.size();  i++)
  {
    glyphs.push_back(Glyph());
    Glyph& glyph = glyphs.back();
    glyphMap[characters[i]] = &glyph;

    glyph.glyph = font.getGlyph(characters[i]);
    if (!glyph.glyph)
    {
      Log::writeError("No glyph for character %c", characters[i]);
      return false;
    }

    const Image& image = glyph.glyph->getImage();

    if (texelPosition.x + image.getWidth() + 2 > texture->getWidth())
    {
      texelPosition.x = 1;
      texelPosition.y += (int) glyphHeight;

      if (texelPosition.y + image.getHeight() + 2 > texture->getHeight())
      {
	// TODO: Allocate new texture.
	Log::writeError("No more room in font texture");
	return false;
      }
    }

    if (!texture->copyFrom(image, texelPosition.x, texelPosition.y))
      return false;

    glyph.area.position.set(texelPosition.x / (float) texture->getWidth(),
			    texelPosition.y / (float) texture->getHeight());
    glyph.area.size.set(image.getWidth() / (float) texture->getWidth(),
			image.getHeight() / (float) texture->getHeight());

    texelPosition.x += image.getWidth();
  }

  Ptr<Image> image = texture->getImage();
  ImageWriter writer;
  writer.write(Path("%s.png", getName().c_str()), *image);

  return true;
}

///////////////////////////////////////////////////////////////////////

void TextureFont::Glyph::render(const Vector2& penPosition)
{
  const Rectangle& texelArea = area;

  Rectangle pixelArea;
  pixelArea.position = penPosition + glyph->getBearing();
  pixelArea.size.set(glyph->getImage().getWidth(), glyph->getImage().getHeight());
    
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
