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

#include <wendy/Config.h>

#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLContext.h>
#include <wendy/GLState.h>

#include <wendy/RenderPool.h>
#include <wendy/RenderFont.h>

#include <cctype>
#include <cstdlib>
#include <cstdarg>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

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

} /*namespace*/

///////////////////////////////////////////////////////////////////////

void Font::drawText(const Vec2 penPosition, const ColorRGBA& color, const String& text) const
{
  GL::VertexRange vertexRange;
  if (!GeometryPool::get()->allocateVertices(vertexRange,
                                             text.size() * 6,
                                             GL::Vertex2ft2fv::format))
  {
    Log::writeError("Failed to allocate vertices for text drawing");
    return;
  }

  unsigned int count = 0;

  // Realize vertices for glyphs
  {
    Vec2 roundedPen = penPosition;
    roundedPen.x = floorf(roundedPen.x + 0.5f);
    roundedPen.y = floorf(roundedPen.y + 0.5f);

    Vec2 penOffset(0.f, 0.f);

    GL::VertexRangeLock<GL::Vertex2ft2fv> vertices(vertexRange);
    if (!vertices)
    {
      Log::writeError("Failed to lock vertices for text drawing");
      return;
    }

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
  }

  pass.getUniformState("color").setValue(color);
  pass.apply();

  GL::Context::get()->render(GL::PrimitiveRange(GL::TRIANGLE_LIST,
                                                *vertexRange.getVertexBuffer(),
				                vertexRange.getStart(),
				                count));
}

void Font::drawText(const Vec2 penPosition, const ColorRGBA& color, const char* format, ...) const
{
  va_list vl;
  char* text;
  int result;

  va_start(vl, format);
  result = vasprintf(&text, format, vl);
  va_end(vl);

  if (result < 0)
    return;

  drawText(penPosition, color, String(text));

  std::free(text);
}

float Font::getWidth(void) const
{
  return size.x;
}

float Font::getHeight(void) const
{
  return size.y;
}

float Font::getAscender(void) const
{
  return ascender;
}

float Font::getDescender(void) const
{
  return descender;
}

Rect Font::getTextMetrics(const String& text) const
{
  Rect result(Vec2::ZERO, Vec2::ZERO);

  LayoutList layout;
  getTextLayout(layout, text);

  for (LayoutList::const_iterator i = layout.begin();  i != layout.end();  i++)
    result.envelop((*i).area);

  return result;
}

Rect Font::getTextMetrics(const char* format, ...) const
{
  va_list vl;
  char* text;
  int result;

  va_start(vl, format);
  result = vasprintf(&text, format, vl);
  va_end(vl);

  if (result < 0)
    return Rect(0.f, 0.f, 0.f, 0.f);

  Rect metrics = getTextMetrics(String(text));

  std::free(text);
  return metrics;
}

void Font::getTextLayout(LayoutList& result, const String& text) const
{
  Vec2 pen(0.f, 0.f);

  for (String::const_iterator c = text.begin();  c != text.end();  c++)
  {
    Layout layout;
    layout.character = *c;
    layout.penOffset = pen;

    switch (*c)
    {
      case '\t':
      {
	layout.area.set(pen, Vec2::ZERO);
	pen.x += size.x * 3.f;
	break;
      }

      case '\n':
      {
	layout.area.set(pen, Vec2::ZERO);
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

void Font::getTextLayout(LayoutList& layout, const char* format, ...) const
{
  va_list vl;
  char* text;
  int result;

  va_start(vl, format);
  result = vasprintf(&text, format, vl);
  va_end(vl);

  if (result < 0)
    return;

  getTextLayout(layout, String(text));

  std::free(text);
}

Font* Font::createInstance(const Path& path,
			   const String& characters,
			   const String& name)
{
  Ptr<wendy::Font> font(wendy::Font::readInstance(path, characters));
  if (!font)
    return NULL;

  return createInstance(*font, name);
}

Font* Font::createInstance(const wendy::Font& font, const String& name)
{
  Ptr<Font> renderFont(new Font(name));
  if (!renderFont->init(font))
    return NULL;

  return renderFont.detachObject();
}

Font::Font(const String& name):
  DerivedResource<Font, wendy::Font>(name)
{
}

Font::Font(const Font& source):
  DerivedResource<Font, wendy::Font>(source)
{
  // NOTE: Not implemented.
}

Font& Font::operator = (const Font& source)
{
  // NOTE: Not implemented.

  return *this;
}

bool Font::init(const wendy::Font& font)
{
  GL::Context* context = GL::Context::get();

  const String& characters = font.getCharacters();

  const unsigned int maxSize = context->getLimits().getMaxTextureSize();

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

    texture = GL::Texture::createInstance(*context, Image(PixelFormat::R8, width, height), 0);
    if (!texture)
      return false;

    texture->setFilterMode(GL::FILTER_NEAREST);
  }

  Vec2 texelOffset;
  texelOffset.x = 0.25f / texture->getWidth();
  texelOffset.y = 0.25f / texture->getHeight();

  // Create render pass
  {
    Ref<GL::Program> program = GL::Program::readInstance("RenderFont");
    if (!program)
    {
      Log::writeError("Unable to read font glyph shader program");
      return false;
    }

    GL::ProgramInterface interface;
    interface.addSampler("glyphs", GL::Sampler::SAMPLER_2D);
    interface.addUniform("color", GL::Uniform::FLOAT_VEC4);
    interface.addVarying("position", GL::Varying::FLOAT_VEC2);
    interface.addVarying("mapping", GL::Varying::FLOAT_VEC2);

    if (!interface.matches(*program, true))
    {
      Log::writeError("Font shader program does not conform to the required interface");
      return false;
    }

    pass.setProgram(program);
    pass.setDepthTesting(false);
    pass.setDepthWriting(false);
    pass.setBlendFactors(GL::BLEND_SRC_ALPHA, GL::BLEND_ONE_MINUS_SRC_ALPHA);
    pass.getSamplerState("glyphs").setTexture(texture);
    pass.getUniformState("color").setValue(Vec4(1, 1, 1, 1));
  }

  ascender = descender = 0.f;

  Vec2i texelPosition(1, 1);

  GL::TextureImage& textureImage = texture->getImage(0);

  for (unsigned int i = 0;  i < characters.size();  i++)
  {
    glyphs.push_back(Glyph());
    Glyph& glyph = glyphs.back();
    glyphMap[characters[i]] = &glyph;

    const wendy::Font::Glyph* sourceGlyph = font.getGlyph(characters[i]);
    if (!sourceGlyph)
    {
      if (std::isgraph(characters[i]))
	Log::writeError("No glyph for character \'%c\'", characters[i]);
      else
	Log::writeError("No glyph for character 0x%02x", (unsigned int) characters[i]);

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

    if (texelPosition.x + image.getWidth() + 2 > textureImage.getWidth())
    {
      texelPosition.x = 1;
      texelPosition.y += (int) glyphHeight;

      if (texelPosition.y + image.getHeight() + 2 > textureImage.getHeight())
      {
	// TODO: Allocate next texture.
	// TODO: Add texture pointer to glyphs.
	Log::writeError("Not enough room in texture for font \'%s\'", getName().c_str());
	return false;
      }
    }

    if (!textureImage.copyFrom(image, texelPosition.x, texelPosition.y))
      return false;

    glyph.area.position.set(texelPosition.x / (float) textureImage.getWidth() + texelOffset.x,
			    texelPosition.y / (float) textureImage.getHeight() + texelOffset.y);
    glyph.area.size.set(image.getWidth() / (float) textureImage.getWidth(),
			image.getHeight() / (float) textureImage.getHeight());

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

void Font::Glyph::realizeVertices(Vec2 penPosition, GL::Vertex2ft2fv* vertices) const
{
  const Rect& ta = area;

  Rect pa;
  pa.position = penPosition;
  pa.position.y += bearing.y - size.y;
  pa.size = size;

  vertices[0].mapping.set(ta.position.x, ta.position.y);
  vertices[0].position.set(pa.position.x, pa.position.y);
  vertices[1].mapping.set(ta.position.x + ta.size.x, ta.position.y);
  vertices[1].position.set(pa.position.x + pa.size.x, pa.position.y);
  vertices[2].mapping.set(ta.position.x + ta.size.x, ta.position.y + ta.size.y);
  vertices[2].position.set(pa.position.x + pa.size.x, pa.position.y + pa.size.y);

  vertices[3] = vertices[2];
  vertices[4].mapping.set(ta.position.x, ta.position.y + ta.size.y);
  vertices[4].position.set(pa.position.x, pa.position.y + pa.size.y);
  vertices[5] = vertices[0];
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
