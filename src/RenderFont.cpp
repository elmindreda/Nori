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

const unsigned int FONT_XML_VERSION = 1;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

void Font::drawText(const Vec2& penPosition, const ColorRGBA& color, const String& text) const
{
  GL::VertexRange vertexRange;
  if (!pool.allocateVertices(vertexRange, text.size() * 6, Vertex2ft2fv::format))
  {
    Log::writeError("Failed to allocate vertices for text drawing");
    return;
  }

  unsigned int count = 0;

  // Realize vertices for glyphs
  {
    Vec2 roundedPen;
    roundedPen.x = floorf(penPosition.x + 0.5f);
    roundedPen.y = floorf(penPosition.y + 0.5f);

    GL::VertexRangeLock<Vertex2ft2fv> vertices(vertexRange);
    if (!vertices)
    {
      Log::writeError("Failed to lock vertices for text drawing");
      return;
    }

    Layout layout;

    for (String::const_iterator c = text.begin();  c != text.end();  c++)
    {
      if (const Glyph* glyph = findGlyph(*c))
      {
        getGlyphLayout(layout, *glyph, *c);
        layout.area.position += roundedPen;
        roundedPen += layout.advance;

        realizeVertices(layout.area, glyph->area, vertices + count);
        count += 6;
      }
    }
  }

  if (!count)
    return;

  pass.getUniformState("color").setValue(color);
  pass.apply();

  pool.getContext().render(GL::PrimitiveRange(GL::TRIANGLE_LIST,
                                              *vertexRange.getVertexBuffer(),
				              vertexRange.getStart(),
				              count));
}

void Font::drawText(const Vec2& penPosition, const ColorRGBA& color, const char* format, ...) const
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
  Rect result(0.f, 0.f, 0.f, 0.f);

  Layout layout;
  Vec2 penPosition(Vec2::ZERO);

  for (String::const_iterator c = text.begin();  c != text.end();  c++)
  {
    if (getGlyphLayout(layout, *c))
    {
      layout.area.position += penPosition;
      result.envelop(layout.area);
      penPosition += layout.advance;
    }
  }

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
  for (String::const_iterator c = text.begin();  c != text.end();  c++)
  {
    Layout layout;

    if (getGlyphLayout(layout, *c))
      result.push_back(layout);
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

Ref<Font> Font::createInstance(const ResourceInfo& info,
                               GeometryPool& pool,
                               const wendy::Font& font,
                               GL::Program& program)
{
  Ptr<Font> renderFont(new Font(info, pool));
  if (!renderFont->init(font, program))
    return NULL;

  return renderFont.detachObject();
}

Font::Font(const ResourceInfo& info, GeometryPool& initPool):
  Resource(info, "render::Font"),
  pool(initPool)
{
}

Font::Font(const Font& source):
  Resource(source),
  pool(source.pool)
{
  // NOTE: Not implemented.
}

Font& Font::operator = (const Font& source)
{
  // NOTE: Not implemented.

  return *this;
}

bool Font::init(const wendy::Font& font, GL::Program& program)
{
  const unsigned int maxSize = pool.getContext().getLimits().getMaxTextureSize();

  unsigned int maxWidth = 0, maxHeight = 0;

  for (size_t i = 0;  i < font.glyphs.size();  i++)
  {
    const unsigned int width = font.glyphs[i].image->getWidth();
    if (maxWidth < width)
      maxWidth = width;

    const unsigned int height = font.glyphs[i].image->getHeight();
    if (maxHeight < height)
      maxHeight = height;
  }

  Ref<GL::Texture> texture;

  // Create texture
  {
    unsigned int width = maxWidth * font.glyphs.size() + 1;
    width = std::min(getNextPower(width), maxSize);

    unsigned int rows = font.glyphs.size() * maxWidth / (width - 1);
    if (maxWidth % (width - 1))
      rows++;

    unsigned int height = maxHeight * rows + 1;
    height = std::min(getNextPower(height), maxSize);

    Image image(getIndex(), PixelFormat::R8, width, height);

    texture = GL::Texture::createInstance(getIndex(), pool.getContext(), image, 0);
    if (!texture)
      return false;

    texture->setFilterMode(GL::FILTER_NEAREST);
  }

  Vec2 texelOffset;
  texelOffset.x = 0.25f / texture->getWidth();
  texelOffset.y = 0.25f / texture->getHeight();

  // Create render pass
  {
    GL::ProgramInterface interface;
    interface.addSampler("glyphs", GL::Sampler::SAMPLER_2D);
    interface.addUniform("color", GL::Uniform::FLOAT_VEC4);
    interface.addVarying("position", GL::Varying::FLOAT_VEC2);
    interface.addVarying("mapping", GL::Varying::FLOAT_VEC2);

    if (!interface.matches(program, true))
    {
      Log::writeError("Font shader program does not conform to the required interface");
      return false;
    }

    pass.setProgram(&program);
    pass.setDepthTesting(false);
    pass.setDepthWriting(false);
    pass.setBlendFactors(GL::BLEND_SRC_ALPHA, GL::BLEND_ONE_MINUS_SRC_ALPHA);
    pass.getSamplerState("glyphs").setTexture(texture);
    pass.getUniformState("color").setValue(Vec4(1, 1, 1, 1));
  }

  ascender = descender = 0.f;

  Vec2i texelPosition(1, 1);

  GL::TextureImage& textureImage = texture->getImage(0);

  for (unsigned int i = 0;  i < font.glyphs.size();  i++)
  {
    glyphs.push_back(Glyph());
    Glyph& glyph = glyphs.back();

    const wendy::FontGlyph& sourceGlyph = font.glyphs[i];
    Ref<Image> image = sourceGlyph.image;

    glyph.advance = sourceGlyph.advance;
    glyph.bearing = sourceGlyph.bearing;
    glyph.size.set((float) image->getWidth(), (float) image->getHeight());

    if (glyph.bearing.y > ascender)
      ascender = glyph.bearing.y;

    if (glyph.size.y - glyph.bearing.y > descender)
      descender = glyph.size.y - glyph.bearing.y;

    if (texelPosition.x + image->getWidth() + 2 > textureImage.getWidth())
    {
      texelPosition.x = 1;
      texelPosition.y += (int) maxHeight;

      if (texelPosition.y + image->getHeight() + 2 > textureImage.getHeight())
      {
	// TODO: Allocate next texture.
	// TODO: Add texture pointer to glyphs.
	Log::writeError("Not enough room in texture for font \'%s\'",
                        getPath().asString().c_str());
	return false;
      }
    }

    if (!textureImage.copyFrom(*image, texelPosition.x, texelPosition.y))
      return false;

    glyph.area.position.set(texelPosition.x / (float) textureImage.getWidth() + texelOffset.x,
			    texelPosition.y / (float) textureImage.getHeight() + texelOffset.y);
    glyph.area.size.set(image->getWidth() / (float) textureImage.getWidth(),
			image->getHeight() / (float) textureImage.getHeight());

    texelPosition.x += image->getWidth() + 1;
  }

  size.set(maxWidth, maxHeight);
  return true;
}

const Font::Glyph* Font::findGlyph(char character) const
{
  return characters[character];
}

bool Font::getGlyphLayout(Layout& layout, char character) const
{
  const Glyph* glyph = findGlyph(character);
  if (!glyph)
    return false;

  getGlyphLayout(layout, *glyph, character);
  return true;
}

void Font::getGlyphLayout(Layout& layout, const Glyph& glyph, char character) const
{
  layout.character = character;

  layout.area.position.x = glyph.bearing.x;
  layout.area.position.y = glyph.bearing.y - glyph.size.y;
  layout.area.size.x = (float) glyph.size.x;
  layout.area.size.y = (float) glyph.size.y;

  layout.advance.set(glyph.advance, 0.f);
  layout.advance.x = floorf(layout.advance.x + 0.5f);
  layout.advance.y = floorf(layout.advance.y + 0.5f);
}

void Font::realizeVertices(const Rect& pixelArea,
                           const Rect& texelArea,
                           Vertex2ft2fv* vertices) const
{
  const Rect& pa = pixelArea;
  const Rect& ta = texelArea;

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

FontReader::FontReader(GeometryPool& initPool):
  ResourceReader(initPool.getContext().getIndex()),
  pool(initPool),
  info(getIndex())
{
}

Ref<Font> FontReader::read(const Path& path)
{
  info.path = path;

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

    Path dataPath(readString("data"));
    if (dataPath.asString().empty())
    {
      Log::writeError("Font data path for render font \'%s\' is empty",
                      info.path.asString().c_str());
      return false;
    }

    wendy::FontReader fontReader(getIndex());
    Ref<wendy::Font> data = fontReader.read(dataPath);
    if (!data)
    {
      Log::writeError("Failed to load font data \'%s\' for render font \'%s\'",
                      dataPath.asString().c_str(),
                      info.path.asString().c_str());
      return false;
    }

    Path programPath(readString("program"));
    if (programPath.asString().empty())
    {
      Log::writeError("Shader program path for render font \'%s\' is empty",
                      info.path.asString().c_str());
      return false;
    }

    GL::ProgramReader programReader(pool.getContext());
    Ref<GL::Program> program = programReader.read(programPath);
    {
      Log::writeError("Failed to load shader program \'%s\' for render font \'%s\'",
                      programPath.asString().c_str(),
                      info.path.asString().c_str());
      return false;
    }

    font = Font::createInstance(info, pool, *data, *program);
    if (!font)
      return false;

    return true;
  }

  return true;
}

bool FontReader::onEndElement(const String& name)
{
  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
