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
#include <cstring>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
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

FontData::FontData(void)
{
  for (int i = 0;  i < 256;  i++)
    characters[i] = -1;
}

FontData::FontData(const FontData& source)
{
  operator = (source);
}

FontData& FontData::operator = (const FontData& source)
{
  glyphs = source.glyphs;

  std::memcpy(characters, source.characters, sizeof(characters));
  return *this;
}

///////////////////////////////////////////////////////////////////////

void Font::drawText(const Vec2& penPosition, const ColorRGBA& color, const String& text) const
{
  GL::VertexRange vertexRange;
  if (!pool.allocateVertices(vertexRange, text.size() * 6, Vertex2ft2fv::format))
  {
    logError("Failed to allocate vertices for text drawing");
    return;
  }

  unsigned int count = 0;

  // Realize vertices for glyphs
  {
    Vec2 roundedPen;
    roundedPen.x = floorf(penPosition.x + 0.5f);
    roundedPen.y = floorf(penPosition.y + 0.5f);

    Vertex2ft2fv* vertices = (Vertex2ft2fv*) vertexRange.lock();
    if (!vertices)
    {
      logError("Failed to lock vertices for text drawing");
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

    vertexRange.unlock();
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

void Font::drawText(const Vec2& penPosition,
                    const ColorRGBA& color,
                    const char* format, ...) const
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

Ref<Font> Font::create(const ResourceInfo& info,
                       GeometryPool& pool,
                       const FontData& data)
{
  Ref<Font> font(new Font(info, pool));
  if (!font->init(data))
    return NULL;

  return font;
}

Ref<Font> Font::read(GeometryPool& pool, const Path& path)
{
  FontReader reader(pool);
  return reader.read(path);
}

Font::Font(const ResourceInfo& info, GeometryPool& initPool):
  Resource(info),
  pool(initPool)
{
  std::memset(characters, 0, sizeof(characters));
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

bool Font::init(const FontData& data)
{
  unsigned int maxWidth = 0, maxHeight = 0;

  for (size_t i = 0;  i < data.glyphs.size();  i++)
  {
    const FontGlyphData& glyph = data.glyphs[i];

    const unsigned int width = glyph.image->getWidth();
    if (maxWidth < width)
      maxWidth = width;

    const unsigned int height = glyph.image->getHeight();
    if (maxHeight < height)
      maxHeight = height;
  }

  Ref<GL::Texture> texture;

  // Create glyph texture
  {
    const unsigned int maxSize = pool.getContext().getLimits().getMaxTextureSize();

    unsigned int width = (maxWidth + 1) * data.glyphs.size() + 1;
    width = std::min(getNextPower(width), maxSize);

    unsigned int rows = data.glyphs.size() * maxWidth / (width - 1);
    if (maxWidth % (width - 1))
      rows++;

    unsigned int height = (maxHeight + 1) * rows + 1;
    height = std::min(getNextPower(height), maxSize);

    Image image(getIndex(), PixelFormat::R8, width, height);

    texture = GL::Texture::create(getIndex(), pool.getContext(), image, 0);
    if (!texture)
    {
      logError("Failed to create glyph texture for font \'%s\'",
               getPath().asString().c_str());
      return false;
    }

    log("Allocated %ux%u texture for font \'%s\'",
        texture->getWidth(),
        texture->getHeight(),
        getPath().asString().c_str());

    texture->setFilterMode(GL::FILTER_NEAREST);
  }

  Vec2 texelOffset;
  texelOffset.x = 0.25f / texture->getWidth();
  texelOffset.y = 0.25f / texture->getHeight();

  // Create render pass
  {
    Path programPath("wendy/RenderFont.program");

    Ref<GL::Program> program = GL::Program::read(pool.getContext(), programPath);
    if (!program)
    {
      logError("Failed to read shader program \'%s\' for font \'%s\'",
               programPath.asString().c_str(),
               getPath().asString().c_str());
      return false;
    }

    GL::ProgramInterface interface;
    interface.addSampler("glyphs", GL::Sampler::SAMPLER_2D);
    interface.addUniform("color", GL::Uniform::FLOAT_VEC4);
    interface.addVarying("position", GL::Varying::FLOAT_VEC2);
    interface.addVarying("mapping", GL::Varying::FLOAT_VEC2);

    if (!interface.matches(*program, true))
    {
      logError("Shader program \'%s\' for font \'%s\' does not conform to the required interface",
               programPath.asString().c_str(),
               getPath().asString().c_str());
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
  const unsigned int textureWidth = textureImage.getWidth();
  const unsigned int textureHeight = textureImage.getHeight();

  glyphs.reserve(data.glyphs.size());

  for (size_t i = 0;  i < data.glyphs.size();  i++)
  {
    const FontGlyphData& glyphData = data.glyphs[i];

    glyphs.push_back(Glyph());
    Glyph& glyph = glyphs.back();

    for (int c = 0;  c < 256;  c++)
    {
      if (data.characters[c] == i)
        characters[c] = &glyph;
    }

    Ref<Image> image = glyphData.image;

    glyph.advance = glyphData.advance;
    glyph.bearing = glyphData.bearing;
    glyph.size.set((float) image->getWidth(), (float) image->getHeight());

    if (glyph.bearing.y > ascender)
      ascender = glyph.bearing.y;

    if (glyph.size.y - glyph.bearing.y > descender)
      descender = glyph.size.y - glyph.bearing.y;

    if (texelPosition.x + image->getWidth() + 2 > textureWidth)
    {
      texelPosition.x = 1;
      texelPosition.y += (int) maxHeight;

      if (texelPosition.y + image->getHeight() + 2 > textureHeight)
      {
        // TODO: Allocate next texture.
        // TODO: Add texture pointer to glyphs.
        logError("Not enough room in glyph texture for font \'%s\'",
                 getPath().asString().c_str());
        return false;
      }
    }

    if (!textureImage.copyFrom(*image, texelPosition.x, texelPosition.y))
    {
      logError("Failed to copy glyph image data for font \'%s\'",
               getPath().asString().c_str());
      return false;
    }

    glyph.area.position.set(texelPosition.x / (float) textureWidth + texelOffset.x,
                            texelPosition.y / (float) textureHeight + texelOffset.y);
    glyph.area.size.set(image->getWidth() / (float) textureWidth,
                        image->getHeight() / (float) textureHeight);

    texelPosition.x += image->getWidth() + 1;
  }

  size.set((float) maxWidth, (float) maxHeight);
  return true;
}

const Font::Glyph* Font::findGlyph(uint8_t character) const
{
  return characters[character];
}

bool Font::getGlyphLayout(Layout& layout, uint8_t character) const
{
  const Glyph* glyph = findGlyph(character);
  if (!glyph)
    return false;

  getGlyphLayout(layout, *glyph, character);
  return true;
}

void Font::getGlyphLayout(Layout& layout, const Glyph& glyph, uint8_t character) const
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
  if (Resource* cache = getIndex().findResource(path))
    return dynamic_cast<Font*>(cache);

  info.path = path;

  std::ifstream stream;
  if (!getIndex().openFile(stream, info.path))
    return NULL;

  if (!XML::Reader::read(stream))
  {
    font = NULL;
    return NULL;
  }

  return font.detachObject();
}

bool FontReader::extractGlyphs(FontData& data,
                               const Image& image,
                               const String& characters,
                               bool fixedWidth)
{
  if (image.getFormat() != PixelFormat::R8)
  {
    logError("Image \'%s\' for font \'%s\' has invalid pixel format \'%s\'",
             image.getPath().asString().c_str(),
             info.path.asString().c_str(),
             image.getFormat().asString().c_str());
    return false;
  }

  Image source = image;

  // Crop top and bottom parts
  {
    const unsigned int startY = findStartY(source);
    if (startY == source.getHeight())
    {
      logError("No glyphs found in source image for font \'%s\'",
               info.path.asString().c_str());
      return false;
    }

    const unsigned int endY = findEndY(source);

    if (!source.crop(Recti(0, startY, source.getWidth(), endY - startY)))
    {
      logError("Failed to crop source image for font \'%s\'",
               info.path.asString().c_str());
      return false;
    }
  }

  data.glyphs.reserve(characters.length());

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
      logError("Font \'%s\' has less characters than glyphs",
               info.path.asString().c_str());
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

    Recti area(startX, 0, endX - startX, source.getHeight());

    Ref<Image> glyphImage = source.getArea(area);
    if (!glyphImage)
    {
      logError("Failed to extract glyph image for font \'%s\'",
               info.path.asString().c_str());
      return false;
    }

    data.characters[characters[index++]] = data.glyphs.size();

    data.glyphs.push_back(FontGlyphData());
    FontGlyphData& glyph = data.glyphs.back();
    glyph.bearing.set(0.f, glyphImage->getHeight() / 2.f);
    glyph.advance = (float) glyphImage->getWidth();
    glyph.image = glyphImage;

    startX = endX;
  }

  // HACK: Make digits same width
  {
    std::vector<FontGlyphData*> digitGlyphs;

    float maxAdvance = 0.f;

    for (int c = '0';  c <= '9';  c++)
    {
      const int index = data.characters[c];
      if (index == -1)
        continue;

      FontGlyphData& glyph = data.glyphs[index];
      if (glyph.advance > maxAdvance)
        maxAdvance = glyph.advance;

      digitGlyphs.push_back(&glyph);
    }

    for (size_t i = 0;  i < digitGlyphs.size();  i++)
    {
      FontGlyphData& glyph = *digitGlyphs[i];

      glyph.bearing.x = (maxAdvance - glyph.advance) / 2.f;
      glyph.advance = maxAdvance;
    }
  }

  float maxAdvance = 0.f;
  float meanAdvance = 0.f;

  for (size_t i = 0;  i < data.glyphs.size();  i++)
  {
    FontGlyphData& glyph = data.glyphs[i];

    maxAdvance = fmaxf(maxAdvance, glyph.advance);
    meanAdvance += glyph.advance;
  }

  meanAdvance /= (float) data.glyphs.size();

  if (data.characters[' '] == -1)
  {
    // HACK: Create space glyph if not already present

    data.characters[' '] = data.glyphs.size();

    data.glyphs.push_back(FontGlyphData());
    FontGlyphData& glyph = data.glyphs.back();

    glyph.bearing.set(0.f, 0.f);
    glyph.image = new Image(getIndex(), source.getFormat(), 1, 1);

    if (fixedWidth)
      glyph.advance = maxAdvance;
    else
      glyph.advance = meanAdvance * 0.6f;
  }

  // HACK: Introduce 'tasteful' spacing

  if (fixedWidth)
  {
    for (size_t i = 0;  i < data.glyphs.size();  i++)
    {
      FontGlyphData& glyph = data.glyphs[i];

      glyph.advance = maxAdvance;
      glyph.bearing.x = (glyph.advance - glyph.image->getWidth()) / 2.f;
    }
  }
  else
  {
    for (size_t i = 0;  i < data.glyphs.size();  i++)
      data.glyphs[i].advance += meanAdvance * 0.2f;
  }

  return true;
}

bool FontReader::onBeginElement(const String& name)
{
  if (name == "font")
  {
    if (font)
    {
      logError("More than one font specification found in font \'%s\'",
               info.path.asString().c_str());
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != FONT_XML_VERSION)
    {
      logError("XML format version mismatch in font \'%s\'",
               info.path.asString().c_str());
      return false;
    }

    String characters = readString("characters");
    if (characters.empty())
    {
      logError("No characters specified for font \'%s\'",
               info.path.asString().c_str());
      return false;
    }

    Path imagePath(readString("image"));
    if (imagePath.isEmpty())
    {
      logError("Image path missing for font \'%s\'",
               info.path.asString().c_str());
      return false;
    }

    Ref<Image> image = Image::read(getIndex(), imagePath);
    if (!image)
    {
      logError("Cannot find image \'%s\' for font \'%s\'",
               imagePath.asString().c_str(),
               info.path.asString().c_str());
      return false;
    }

    bool fixedWidth = readBoolean("fixed", false);

    FontData data;

    if (!extractGlyphs(data, *image, characters, fixedWidth))
      return false;

    font = Font::create(info, pool, data);
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
