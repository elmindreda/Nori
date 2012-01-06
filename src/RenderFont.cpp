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

#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>
#include <wendy/GLState.h>

#include <wendy/RenderPool.h>
#include <wendy/RenderFont.h>

#include <cctype>
#include <cstdlib>
#include <cstring>

#include <glm/gtx/bit.hpp>

#include <pugixml.hpp>

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
  const uint8* pixels = (const uint8*) image.getPixels();

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
  const uint8* pixels = (const uint8*) image.getPixels();

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

FontData::FontData()
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

void Font::drawText(const vec2& penPosition, const vec4& color, const char* text)
{
  const size_t length = std::strlen(text);
  if (!length)
    return;

  GL::VertexRange vertexRange;
  if (!pool->allocateVertices(vertexRange, length * 6, Vertex2ft2fv::format))
  {
    logError("Failed to allocate vertices for text drawing");
    return;
  }

  unsigned int count = 0;

  // Realize vertices for glyphs
  {
    vec2 roundedPen;
    roundedPen.x = floor(penPosition.x + 0.5f);
    roundedPen.y = floor(penPosition.y + 0.5f);

    vertices.resize(length * 6);

    Layout layout;

    for (const char* c = text;  *c != '\0';  c++)
    {
      if (const Glyph* glyph = findGlyph(*c))
      {
        getGlyphLayout(layout, *glyph, *c);
        layout.area.position += roundedPen;
        roundedPen += layout.advance;

        const Rect& pa = layout.area;
        const Rect& ta = glyph->area;

        vertices[count + 0].texCoord = ta.position;
        vertices[count + 0].position = pa.position;
        vertices[count + 1].texCoord = ta.position + vec2(ta.size.x, 0.f);
        vertices[count + 1].position = pa.position + vec2(pa.size.x, 0.f);
        vertices[count + 2].texCoord = ta.position + ta.size;
        vertices[count + 2].position = pa.position + pa.size;

        vertices[count + 3] = vertices[count + 2];
        vertices[count + 4].texCoord = ta.position + vec2(0.f, ta.size.y);
        vertices[count + 4].position = pa.position + vec2(0.f, pa.size.y);
        vertices[count + 5] = vertices[count + 0];

        count += 6;
      }
    }

    vertexRange.copyFrom(&vertices[0]);
  }

  if (!count)
    return;

  pass.setUniformState(colorIndex, color);
  pass.apply();

  GL::Context& context = pool->getContext();
  context.render(GL::PrimitiveRange(GL::TRIANGLE_LIST,
                                    *vertexRange.getVertexBuffer(),
                                    vertexRange.getStart(),
                                    count));
}

float Font::getWidth() const
{
  return size.x;
}

float Font::getHeight() const
{
  return size.y;
}

float Font::getAscender() const
{
  return ascender;
}

float Font::getDescender() const
{
  return descender;
}

Rect Font::getTextMetrics(const char* text) const
{
  Rect result;
  Layout layout;
  vec2 penPosition;

  for (const char* c = text;  *c != '\0';  c++)
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

void Font::getTextLayout(LayoutList& result, const char* text) const
{
  for (const char* c = text;  *c != '\0';  c++)
  {
    Layout layout;

    if (getGlyphLayout(layout, *c))
      result.push_back(layout);
  }
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

Ref<Font> Font::read(GeometryPool& pool, const String& name)
{
  FontReader reader(pool);
  return reader.read(name);
}

Font::Font(const ResourceInfo& info, GeometryPool& initPool):
  Resource(info),
  pool(&initPool)
{
  std::memset(characters, 0, sizeof(characters));
}

Font::Font(const Font& source):
  Resource(source),
  pool(source.pool)
{
  panic("Fonts may not be copied");
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
  GL::Context& context = pool->getContext();

  // Create glyph texture
  {
    const unsigned int maxSize = context.getLimits().maxTextureSize;

    unsigned int totalWidth = 1;

    for (size_t i = 0;  i < data.glyphs.size();  i++)
      totalWidth += data.glyphs[i].image->getWidth() + 1;

    unsigned int textureWidth = min(powerOfTwoAbove(totalWidth), maxSize);

    unsigned int rows = totalWidth / textureWidth;
    if (totalWidth % textureWidth)
      rows++;

    unsigned int textureHeight = (maxHeight + 1) * rows + 1;
    textureHeight = min(powerOfTwoAbove(textureHeight), maxSize);

    Ref<Image> image = Image::create(cache,
                                     PixelFormat::R8,
                                     textureWidth,
                                     textureHeight);

    GL::TextureParams params(GL::TEXTURE_2D);
    params.mipmapped = false;

    texture = GL::Texture::create(cache, context, params, *image);
    if (!texture)
    {
      logError("Failed to create glyph texture for font \'%s\'",
               getName().c_str());
      return false;
    }

    log("Allocated texture of size %ux%u format \'%s\' for font \'%s\'",
        texture->getWidth(),
        texture->getHeight(),
        texture->getFormat().asString().c_str(),
        getName().c_str());

    texture->setFilterMode(GL::FILTER_NEAREST);
  }

  vec2 texelOffset;
  texelOffset.x = 0.25f / texture->getWidth();
  texelOffset.y = 0.25f / texture->getHeight();

  // Create render pass
  {
    Ref<GL::Program> program = GL::Program::read(context,
                                                 "wendy/RenderFont.vs",
                                                 "wendy/RenderFont.fs");
    if (!program)
    {
      logError("Failed to read program for font \'%s\'", getName().c_str());
      return false;
    }

    GL::ProgramInterface interface;
    interface.addSampler("glyphs", GL::SAMPLER_2D);
    interface.addUniform("color", GL::UNIFORM_VEC4);
    interface.addAttributes(Vertex2ft2fv::format);

    if (!interface.matches(*program, true))
    {
      logError("Program \'%s\' for font \'%s\' does not conform to the required interface",
               program->getName().c_str(),
               getName().c_str());
      return false;
    }

    pass.setProgram(program);
    pass.setDepthTesting(false);
    pass.setDepthWriting(false);
    pass.setBlendFactors(GL::BLEND_SRC_ALPHA, GL::BLEND_ONE_MINUS_SRC_ALPHA);
    pass.setSamplerState("glyphs", texture);
    pass.setUniformState("color", vec4(1.f));

    colorIndex = pass.getUniformStateIndex("color");
  }

  ascender = descender = 0.f;

  ivec2 texelPosition(1, 1);

  GL::TextureImage& textureImage = texture->getImage(0);
  const unsigned int textureWidth = textureImage.getWidth();
  const unsigned int textureHeight = textureImage.getHeight();

  glyphs.reserve(data.glyphs.size());

  for (size_t i = 0;  i < data.glyphs.size();  i++)
  {
    const FontGlyphData& glyphData = data.glyphs[i];

    glyphs.push_back(Glyph());
    Glyph& glyph = glyphs.back();

    for (size_t c = 0;  c < 256;  c++)
    {
      if (data.characters[c] == int(i))
        characters[c] = &glyph;
    }

    Ref<Image> image = glyphData.image;

    glyph.advance = glyphData.advance;
    glyph.bearing = glyphData.bearing;
    glyph.size = vec2((float) image->getWidth(), (float) image->getHeight());

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
                 getName().c_str());
        return false;
      }
    }

    if (!textureImage.copyFrom(*image, texelPosition.x, texelPosition.y))
    {
      logError("Failed to copy glyph image data for font \'%s\'",
               getName().c_str());
      return false;
    }

    glyph.area.position = vec2(texelPosition.x / (float) textureWidth + texelOffset.x,
                               texelPosition.y / (float) textureHeight + texelOffset.y);
    glyph.area.size = vec2(image->getWidth() / (float) textureWidth,
                           image->getHeight() / (float) textureHeight);

    texelPosition.x += image->getWidth() + 1;
  }

  size = vec2((float) maxWidth, (float) maxHeight);
  return true;
}

const Font::Glyph* Font::findGlyph(uint8 character) const
{
  return characters[character];
}

Font& Font::operator = (const Font& source)
{
  panic("Fonts may not be assigned");
}

bool Font::getGlyphLayout(Layout& layout, uint8 character) const
{
  const Glyph* glyph = findGlyph(character);
  if (!glyph)
    return false;

  getGlyphLayout(layout, *glyph, character);
  return true;
}

void Font::getGlyphLayout(Layout& layout, const Glyph& glyph, uint8 character) const
{
  layout.character = character;

  layout.area.position.x = glyph.bearing.x;
  layout.area.position.y = glyph.bearing.y - glyph.size.y;
  layout.area.size.x = (float) glyph.size.x;
  layout.area.size.y = (float) glyph.size.y;

  layout.advance = floor(vec2(glyph.advance, 0.f) + vec2(0.5f));
}

///////////////////////////////////////////////////////////////////////

FontReader::FontReader(GeometryPool& initPool):
  ResourceReader<Font>(initPool.getContext().getCache()),
  pool(&initPool)
{
}

Ref<Font> FontReader::read(const String& name, const Path& path)
{
  std::ifstream stream(path.asString().c_str());
  if (stream.fail())
  {
    logError("Failed to open font \'%s\'", name.c_str());
    return NULL;
  }

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load font \'%s\': %s",
             name.c_str(),
             result.description());
    return NULL;
  }

  pugi::xml_node root = document.child("font");
  if (!root || root.attribute("version").as_uint() != FONT_XML_VERSION)
  {
    logError("Font file format mismatch in \'%s\'", name.c_str());
    return NULL;
  }

  const String characters(root.attribute("characters").value());
  if (characters.empty())
  {
    logError("No characters specified for font \'%s\'", name.c_str());
    return NULL;
  }

  const String imageName(root.attribute("image").value());
  if (imageName.empty())
  {
    logError("Glyph image path missing for font \'%s\'", name.c_str());
    return NULL;
  }

  Ref<Image> image = Image::read(cache, imageName);
  if (!image)
  {
    logError("Failed to load glyph image for font \'%s\'", name.c_str());
    return NULL;
  }

  bool fixedWidth = false;

  if (pugi::xml_attribute a = root.attribute("fixed"))
    fixedWidth = a.as_bool();

  FontData data;

  if (!extractGlyphs(data, name, *image, characters, fixedWidth))
    return NULL;

  return Font::create(ResourceInfo(cache, name, path), *pool, data);
}

bool FontReader::extractGlyphs(FontData& data,
                               const String& name,
                               const Image& image,
                               const String& characters,
                               bool fixedWidth)
{
  if (image.getFormat() != PixelFormat::R8)
  {
    logError("Image \'%s\' for font \'%s\' has invalid pixel format \'%s\'",
             image.getName().c_str(),
             name.c_str(),
             image.getFormat().asString().c_str());
    return false;
  }

  Ref<Image> source;

  // Crop top and bottom parts
  {
    const unsigned int startY = findStartY(image);
    if (startY == image.getHeight())
    {
      logError("No glyphs found in source image for font \'%s\'", name.c_str());
      return false;
    }

    const unsigned int endY = findEndY(image);

    source = image.getArea(Recti(0, startY, image.getWidth(), endY - startY));
    if (!source)
    {
      logError("Failed to crop source image for font \'%s\'", name.c_str());
      return false;
    }
  }

  data.glyphs.reserve(characters.length());

  const uint8* pixels = (const uint8*) source->getPixels();

  unsigned int index = 0, startX = 0, endX;

  for (;;)
  {
    // Find left edge of glyph, if any

    while (startX < source->getWidth())
    {
      unsigned int y;

      for (y = 0;  y < source->getHeight();  y++)
      {
        if (pixels[startX + y * source->getWidth()] > 0)
          break;
      }

      if (y < source->getHeight())
        break;

      startX++;
    }

    if (startX == source->getWidth())
      break;

    if (index == characters.size())
    {
      logError("Font \'%s\' has less characters than glyphs", name.c_str());
      return false;
    }

    // Find right edge of glyph

    for (endX = startX + 1;  endX < source->getWidth();  endX++)
    {
      unsigned int y;

      for (y = 0;  y < source->getHeight();  y++)
      {
        if (pixels[endX + y * source->getWidth()] > 0)
          break;
      }

      if (y == source->getHeight())
        break;
    }

    Recti area(startX, 0, endX - startX, source->getHeight());

    Ref<Image> glyphImage = source->getArea(area);
    if (!glyphImage)
    {
      logError("Failed to extract glyph image for font \'%s\'", name.c_str());
      return false;
    }

    data.characters[(unsigned char) characters[index++]] = data.glyphs.size();

    data.glyphs.push_back(FontGlyphData());
    FontGlyphData& glyph = data.glyphs.back();
    glyph.bearing = vec2(0.f, glyphImage->getHeight() / 2.f);
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

    maxAdvance = max(maxAdvance, glyph.advance);
    meanAdvance += glyph.advance;
  }

  meanAdvance /= (float) data.glyphs.size();

  // HACK: Create space glyph if not already present

  if (data.characters[' '] == -1)
  {
    data.characters[' '] = data.glyphs.size();

    data.glyphs.push_back(FontGlyphData());
    FontGlyphData& glyph = data.glyphs.back();

    glyph.bearing = vec2(0.f);
    glyph.image = Image::create(cache, source->getFormat(), 1, 1);

    if (fixedWidth)
      glyph.advance = maxAdvance;
    else
      glyph.advance = meanAdvance * 0.6f;
  }

  // HACK: Create tab glyph if not already present

  if (data.characters['\t'] == -1)
    data.characters['\t'] = data.characters[' '];

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

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
