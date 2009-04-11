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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLShader.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>
#include <wendy/GLPass.h>

#include <wendy/RenderFont.h>

#include <cstdlib>

#if MOIRA_HAVE_STDARG_H
#include <stdarg.h>
#endif

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
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
  
void Font::drawText(const String& text) const
{
  GL::Renderer* renderer = GL::Renderer::get();

  GL::VertexRange vertexRange;
  if (!renderer->allocateVertices(vertexRange, text.size() * 6, GL::Vertex2ft2fv::format))
  {
    Log::writeError("Failed to allocate vertices for text drawing");
    return;
  }

  Vector2 roundedPen = penPosition;
  roundedPen.x = floorf(roundedPen.x + 0.5f);
  roundedPen.y = floorf(roundedPen.y + 0.5f);

  Vector2 penOffset(0.f, 0.f);

  GL::Vertex2ft2fv* vertices = (GL::Vertex2ft2fv*) vertexRange.lock();

  unsigned int count = 0;

  for (String::const_iterator c = text.begin();  c != text.end();  c++)
  {
    switch (*c)
    {
      case '\t':
      {
	penOffset.x += size.x * 3.f;
	break;
      }
	
      case '\n':
      {
	penOffset.x = 0.f;
	penOffset.y -= size.y * 1.2f;
	break;
      }

      case ' ':
      {
	const Glyph* glyph = getGlyph(*c);
	if (!glyph)
	  continue;

	penOffset.x += glyph->advance;
	break;
      }

      default:
      {
	const Glyph* glyph = getGlyph(*c);
	if (!glyph)
	  continue;

	glyph->realizeVertices(roundedPen + penOffset, vertices + count);
	count += 6;

	penOffset.x += glyph->advance;
	break;
      }
    }

    penOffset.x = floorf(penOffset.x + 0.5f);
    penOffset.y = floorf(penOffset.y + 0.5f);
  }

  vertexRange.unlock();

  pass.apply();

  renderer->setCurrentPrimitiveRange(GL::PrimitiveRange(GL::TRIANGLE_LIST,
                                                        *vertexRange.getVertexBuffer(),
							vertexRange.getStart(),
							count));
  renderer->render();
}

void Font::drawText(const char* format, ...) const
{
  va_list vl;
  char* text;

  va_start(vl, format);
  vasprintf(&text, format, vl);
  va_end(vl);
  
  drawText(String(text));

  free(text);
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
  pass.getUniformState("color").setValue(Vector4(color.g, color.g, color.b, color.a));
}

float Font::getAscender(void) const
{
  return ascender;
}

float Font::getDescender(void) const
{
  return descender;
}

Rectangle Font::getTextMetrics(const String& text) const
{
  Rectangle result(Vector2::ZERO, Vector2::ZERO);

  LayoutList layout;
  getTextLayout(layout, text);

  for (LayoutList::const_iterator i = layout.begin();  i != layout.end();  i++)
    result.envelop((*i).area);

  return result;
}

Rectangle Font::getTextMetrics(const char* format, ...) const
{
  va_list vl;
  char* text;

  va_start(vl, format);
  vasprintf(&text, format, vl);
  va_end(vl);
  
  Rectangle result = getTextMetrics(String(text));

  free(text);
  return result;
}

void Font::getTextLayout(LayoutList& result, const String& text) const
{
  Vector2 pen(0.f, 0.f);

  for (String::const_iterator c = text.begin();  c != text.end();  c++)
  {
    Layout layout;
    layout.character = *c;
    layout.penOffset = pen;

    switch (*c)
    {
      case '\t':
      {
	layout.area.set(pen, Vector2::ZERO);
	pen.x += size.x * 3.f;
	break;
      }
	
      case '\n':
      {
	layout.area.set(pen, Vector2::ZERO);
	pen.x = 0.f;
	pen.y -= size.y * 1.2f;
	break;
      }

      default:
      {
	const Glyph* glyph = getGlyph(*c);
	if (!glyph)
	{
	  glyph = getGlyph('?');
	  if (!glyph)
	    continue;
	}

	layout.area.position.x = pen.x + glyph->bearing.x;
	layout.area.position.y = pen.y - glyph->size.y + glyph->bearing.y;
	layout.area.size.set((float) glyph->size.x,
	                     (float) glyph->size.y);

	pen.x += glyph->advance;
	break;
      }
    }

    result.push_back(layout);

    pen.x = floorf(pen.x + 0.5f);
    pen.y = floorf(pen.y + 0.5f);
  }
}

void Font::getTextLayout(LayoutList& result, const char* format, ...) const
{
  va_list vl;
  char* text;

  va_start(vl, format);
  vasprintf(&text, format, vl);
  va_end(vl);

  getTextLayout(result, String(text));
  
  free(text);
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
  Ptr<Font> renderFont = new Font(name);
  if (!renderFont->init(font))
    return NULL;

  return renderFont.detachObject();
}

Font::Font(const String& name):
  DerivedResource<Font, moira::Font>(name)
{
}

Font::Font(const Font& source):
  DerivedResource<Font, moira::Font>(source)
{
  // NOTE: Not implemented.
}

Font& Font::operator = (const Font& source)
{
  // NOTE: Not implemented.

  return *this;
}

bool Font::init(const moira::Font& font)
{
  const String& characters = font.getCharacters();

  const unsigned int maxSize = GL::Context::get()->getLimits().getMaxTextureSize();

  const unsigned int glyphWidth = (unsigned int) ceilf(font.getWidth()) + 1;
  const unsigned int glyphHeight = (unsigned int) ceilf(font.getHeight()) + 1;

  Ref<GL::Texture> texture;

  // Create texture
  {
    unsigned int width = glyphWidth * characters.size() + 1;
    width = std::min(getNextPower(width), maxSize);

    unsigned int rows = characters.size() * glyphWidth / (width - 1);
    if (glyphWidth % (width - 1))
      rows++;

    unsigned int height = glyphHeight * rows + 1;
    height = std::min(getNextPower(height), maxSize);

    texture = GL::Texture::createInstance(Image(ImageFormat::ALPHA8, width, height), 0);
    if (!texture)
      return false;
  }

  // Create render pass
  {
    Ref<GL::Program> program = GL::Program::readInstance("font");
    if (!program)
    {
      Log::writeError("Unable to read font glyph shader program");
      return false;
    }

    if (!program->findSampler("glyphs") || !program->findUniform("color"))
    {
      Log::writeError("Font shader program does not have the required uniforms");
      return false;
    }

    pass.setProgram(program);
    pass.setDepthTesting(false);
    pass.setDepthWriting(false);
    pass.setBlendFactors(GL::BLEND_SRC_ALPHA, GL::BLEND_ONE_MINUS_SRC_ALPHA);
    pass.getSamplerState("glyphs").setTexture(texture);
    pass.getUniformState("color").setValue(Vector4(1, 1, 1, 1));

    color = ColorRGBA::WHITE;
  }

  ascender = descender = 0.f;

  Vector2i texelPosition(1, 1);

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

    if (glyph.bearing.y > ascender)
      ascender = glyph.bearing.y;

    if (glyph.size.y - glyph.bearing.y > descender)
      descender = glyph.size.y - glyph.bearing.y;

    if (texelPosition.x + image.getWidth() + 2 > texture->getPhysicalWidth())
    {
      texelPosition.x = 1;
      texelPosition.y += (int) glyphHeight;

      if (texelPosition.y + image.getHeight() + 2 > texture->getPhysicalHeight())
      {
	// TODO: Allocate next texture.
	// TODO: Add texture pointer to glyphs.
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

const Font::Glyph* Font::getGlyph(char character) const
{
  GlyphMap::const_iterator i = glyphMap.find(character);
  if (i == glyphMap.end())
    return NULL;

  return (*i).second;
}

///////////////////////////////////////////////////////////////////////

/*
void Font::Glyph::draw(const Vector2& penPosition) const
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
*/

void Font::Glyph::realizeVertices(Vector2 penPosition, GL::Vertex2ft2fv* vertices) const
{
  const Rectangle& ta = area;

  Rectangle pa;
  pa.position = penPosition;
  pa.position.y += bearing.y - size.y;
  pa.size = size;
    
  vertices[0].mapping.set(ta.position.x, ta.position.y);
  vertices[0].position.set(pa.position.x, pa.position.y);
  vertices[1].mapping.set(ta.position.x + ta.size.x, ta.position.y);
  vertices[1].position.set(pa.position.x + pa.size.x, pa.position.y);
  vertices[2].mapping.set(ta.position.x + ta.size.x, ta.position.y + ta.size.y);
  vertices[2].position.set(pa.position.x + pa.size.x, pa.position.y + pa.size.y);

  vertices[3].mapping.set(ta.position.x + ta.size.x, ta.position.y + ta.size.y);
  vertices[3].position.set(pa.position.x + pa.size.x, pa.position.y + pa.size.y);
  vertices[4].mapping.set(ta.position.x, ta.position.y + ta.size.y);
  vertices[4].position.set(pa.position.x, pa.position.y + pa.size.y);
  vertices[5].mapping.set(ta.position.x, ta.position.y);
  vertices[5].position.set(pa.position.x, pa.position.y);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
