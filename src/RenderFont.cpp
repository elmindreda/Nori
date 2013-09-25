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

#include <wendy/Config.hpp>

#include <wendy/GLTexture.hpp>
#include <wendy/GLBuffer.hpp>
#include <wendy/GLProgram.hpp>
#include <wendy/GLContext.hpp>

#include <wendy/RenderPool.hpp>
#include <wendy/RenderState.hpp>
#include <wendy/RenderFont.hpp>

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

uint findStartY(const Image& image)
{
  const uint8* pixels = (const uint8*) image.pixels();

  uint startY;

  for (startY = 0;  startY < image.height();  startY++)
  {
    uint x;

    for (x = 0;  x < image.width();  x++)
    {
      if (pixels[x + startY * image.width()] > 0)
        break;
    }

    if (x < image.width())
      break;
  }

  return startY;
}

uint findEndY(const Image& image)
{
  const uint8* pixels = (const uint8*) image.pixels();

  uint endY;

  for (endY = image.height();  endY > 0;  endY--)
  {
    uint x;

    for (x = 0;  x < image.width();  x++)
    {
      if (pixels[x + (endY - 1) * image.width()] > 0)
        break;
    }

    if (x < image.width())
      break;
  }

  return endY;
}

const uint FONT_XML_VERSION = 1;

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
  if (!m_pool->allocateVertices(vertexRange, length * 6, Vertex2ft2fv::format))
  {
    logError("Failed to allocate vertices for text drawing");
    return;
  }

  uint count = 0;

  // Realize vertices for glyphs
  {
    vec2 roundedPen;
    roundedPen.x = floor(penPosition.x + 0.5f);
    roundedPen.y = floor(penPosition.y + 0.5f);

    m_vertices.resize(length * 6);

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

        m_vertices[count + 0].texcoord = ta.position;
        m_vertices[count + 0].position = pa.position;
        m_vertices[count + 1].texcoord = ta.position + vec2(ta.size.x, 0.f);
        m_vertices[count + 1].position = pa.position + vec2(pa.size.x, 0.f);
        m_vertices[count + 2].texcoord = ta.position + ta.size;
        m_vertices[count + 2].position = pa.position + pa.size;

        m_vertices[count + 3] = m_vertices[count + 2];
        m_vertices[count + 4].texcoord = ta.position + vec2(0.f, ta.size.y);
        m_vertices[count + 4].position = pa.position + vec2(0.f, pa.size.y);
        m_vertices[count + 5] = m_vertices[count + 0];

        count += 6;
      }
    }

    vertexRange.copyFrom(&m_vertices[0]);
  }

  if (!count)
    return;

  m_pass.setUniformState(m_colorIndex, color);
  m_pass.apply();

  GL::Context& context = m_pool->context();
  context.render(GL::PrimitiveRange(GL::TRIANGLE_LIST,
                                    *vertexRange.vertexBuffer(),
                                    vertexRange.start(),
                                    count));
}

Rect Font::metricsOf(const char* text) const
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

  result.envelop(penPosition);
  return result;
}

Font::LayoutList Font::layoutOf(const char* text) const
{
  LayoutList result;

  for (const char* c = text;  *c != '\0';  c++)
  {
    Layout layout;

    if (getGlyphLayout(layout, *c))
      result.push_back(layout);
  }

  return result;
}

Ref<Font> Font::create(const ResourceInfo& info,
                       VertexPool& pool,
                       const FontData& data)
{
  Ref<Font> font(new Font(info, pool));
  if (!font->init(data))
    return nullptr;

  return font;
}

Ref<Font> Font::read(VertexPool& pool, const String& name)
{
  FontReader reader(pool);
  return reader.read(name);
}

Font::Font(const ResourceInfo& info, VertexPool& pool):
  Resource(info),
  m_pool(&pool)
{
  std::memset(m_characters, 0, sizeof(m_characters));
}

Font::Font(const Font& source):
  Resource(source),
  m_pool(source.m_pool)
{
  panic("Fonts may not be copied");
}

bool Font::init(const FontData& data)
{
  uint maxWidth = 0, maxHeight = 0;

  for (auto& g : data.glyphs)
  {
    maxWidth = max(maxWidth, g.image->width());
    maxHeight = max(maxHeight, g.image->height());
  }

  Ref<GL::Texture> texture;
  GL::Context& context = m_pool->context();

  // Create glyph texture
  {
    const uint maxSize = context.limits().maxTextureSize;

    uint totalWidth = 1;

    for (auto& g : data.glyphs)
      totalWidth += g.image->width() + 1;

    uint textureWidth = min(powerOfTwoAbove(totalWidth), maxSize);

    uint rows = totalWidth / textureWidth;
    if (totalWidth % textureWidth)
      rows++;

    uint textureHeight = (maxHeight + 1) * rows + 1;
    textureHeight = min(powerOfTwoAbove(textureHeight), maxSize);

    Ref<Image> image = Image::create(cache(),
                                     PixelFormat::L8,
                                     textureWidth,
                                     textureHeight);

    GL::TextureParams params(GL::TEXTURE_2D);
    params.mipmapped = false;

    texture = GL::Texture::create(cache(), context, params, *image);
    if (!texture)
    {
      logError("Failed to create glyph texture for font %s",
               name().c_str());
      return false;
    }

    log("Allocated texture of size %ux%u format %s for font %s",
        texture->width(),
        texture->height(),
        texture->format().asString().c_str(),
        name().c_str());

    texture->setFilterMode(GL::FILTER_NEAREST);
  }

  vec2 texelOffset;
  texelOffset.x = 0.25f / texture->width();
  texelOffset.y = 0.25f / texture->height();

  // Create render pass
  {
    Ref<GL::Program> program = GL::Program::read(context,
                                                 "wendy/RenderFont.vs",
                                                 "wendy/RenderFont.fs");
    if (!program)
    {
      logError("Failed to read program for font %s", name().c_str());
      return false;
    }

    GL::ProgramInterface interface;
    interface.addSampler("glyphs", GL::SAMPLER_2D);
    interface.addUniform("color", GL::UNIFORM_VEC4);
    interface.addAttributes(Vertex2ft2fv::format);

    if (!interface.matches(*program, true))
    {
      logError("Program %s for font %s does not conform to the required interface",
               program->name().c_str(),
               name().c_str());
      return false;
    }

    m_pass.setProgram(program);
    m_pass.setMultisampling(false);
    m_pass.setDepthTesting(false);
    m_pass.setDepthWriting(false);
    m_pass.setBlendFactors(GL::BLEND_SRC_ALPHA, GL::BLEND_ONE_MINUS_SRC_ALPHA);
    m_pass.setSamplerState("glyphs", texture);
    m_pass.setUniformState("color", vec4(1.f));

    m_colorIndex = m_pass.uniformStateIndex("color");
  }

  m_ascender = m_descender = 0.f;

  ivec2 texelPosition(1, 1);

  GL::TextureImage& textureImage = texture->image(0);
  const uint textureWidth = textureImage.width();
  const uint textureHeight = textureImage.height();

  m_glyphs.reserve(data.glyphs.size());

  for (size_t i = 0;  i != data.glyphs.size();  i++)
  {
    const FontGlyphData& glyphData = data.glyphs[i];

    m_glyphs.push_back(Glyph());
    Glyph& glyph = m_glyphs.back();

    for (size_t c = 0;  c < 256;  c++)
    {
      if (data.characters[c] == int(i))
        m_characters[c] = &glyph;
    }

    Ref<Image> image = glyphData.image;

    glyph.advance = glyphData.advance;
    glyph.bearing = glyphData.bearing;
    glyph.size = vec2(float(image->width()), float(image->height()));

    if (glyph.bearing.y > m_ascender)
      m_ascender = glyph.bearing.y;

    if (glyph.size.y - glyph.bearing.y > m_descender)
      m_descender = glyph.size.y - glyph.bearing.y;

    if (texelPosition.x + image->width() + 2 > textureWidth)
    {
      texelPosition.x = 1;
      texelPosition.y += int(maxHeight);

      if (texelPosition.y + image->height() + 2 > textureHeight)
      {
        // TODO: Allocate next texture.
        // TODO: Add texture pointer to glyphs.
        logError("Not enough room in glyph texture for font %s",
                 name().c_str());
        return false;
      }
    }

    if (!textureImage.copyFrom(*image, texelPosition.x, texelPosition.y))
    {
      logError("Failed to copy glyph image data for font %s",
               name().c_str());
      return false;
    }

    glyph.area.position = vec2(texelPosition.x / float(textureWidth + texelOffset.x),
                               texelPosition.y / float(textureHeight + texelOffset.y));
    glyph.area.size = vec2(image->width() / float(textureWidth),
                           image->height() / float(textureHeight));

    texelPosition.x += image->width() + 1;
  }

  m_size = vec2(float(maxWidth), float(maxHeight));
  return true;
}

const Font::Glyph* Font::findGlyph(uint8 character) const
{
  return m_characters[character];
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
  layout.area.size.x = float(glyph.size.x);
  layout.area.size.y = float(glyph.size.y);

  layout.advance = floor(vec2(glyph.advance, 0.f) + vec2(0.5f));
}

///////////////////////////////////////////////////////////////////////

FontReader::FontReader(VertexPool& initPool):
  ResourceReader<Font>(initPool.context().cache()),
  pool(&initPool)
{
}

Ref<Font> FontReader::read(const String& name, const Path& path)
{
  std::ifstream stream(path.asString().c_str());
  if (stream.fail())
  {
    logError("Failed to open font %s", name.c_str());
    return nullptr;
  }

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load font %s: %s",
             name.c_str(),
             result.description());
    return nullptr;
  }

  pugi::xml_node root = document.child("font");
  if (!root || root.attribute("version").as_uint() != FONT_XML_VERSION)
  {
    logError("Font file format mismatch in %s", name.c_str());
    return nullptr;
  }

  const String characters(root.attribute("characters").value());
  if (characters.empty())
  {
    logError("No characters specified for font %s", name.c_str());
    return nullptr;
  }

  const String imageName(root.attribute("image").value());
  if (imageName.empty())
  {
    logError("Glyph image path missing for font %s", name.c_str());
    return nullptr;
  }

  Ref<Image> image = Image::read(cache, imageName);
  if (!image)
  {
    logError("Failed to load glyph image for font %s", name.c_str());
    return nullptr;
  }

  bool fixedWidth = false;

  if (pugi::xml_attribute a = root.attribute("fixed"))
    fixedWidth = a.as_bool();

  FontData data;

  if (!extractGlyphs(data, name, *image, characters, fixedWidth))
    return nullptr;

  return Font::create(ResourceInfo(cache, name, path), *pool, data);
}

bool FontReader::extractGlyphs(FontData& data,
                               const String& name,
                               const Image& image,
                               const String& characters,
                               bool fixedWidth)
{
  if (image.format() != PixelFormat::L8)
  {
    logError("Image %s for font %s has invalid pixel format %s",
             image.name().c_str(),
             name.c_str(),
             image.format().asString().c_str());
    return false;
  }

  Ref<Image> source;

  // Crop top and bottom parts
  {
    const uint startY = findStartY(image);
    if (startY == image.height())
    {
      logError("No glyphs found in source image for font %s", name.c_str());
      return false;
    }

    const uint endY = findEndY(image);

    source = image.area(Recti(0, startY, image.width(), endY - startY));
    if (!source)
    {
      logError("Failed to crop source image for font %s", name.c_str());
      return false;
    }
  }

  data.glyphs.reserve(characters.length());

  const uint8* pixels = (const uint8*) source->pixels();

  uint index = 0, startX = 0, endX;

  for (;;)
  {
    // Find left edge of glyph, if any

    while (startX < source->width())
    {
      uint y;

      for (y = 0;  y < source->height();  y++)
      {
        if (pixels[startX + y * source->width()] > 0)
          break;
      }

      if (y < source->height())
        break;

      startX++;
    }

    if (startX == source->width())
      break;

    if (index == characters.size())
    {
      logError("Font %s has less characters than glyphs", name.c_str());
      return false;
    }

    // Find right edge of glyph

    for (endX = startX + 1;  endX < source->width();  endX++)
    {
      uint y;

      for (y = 0;  y < source->height();  y++)
      {
        if (pixels[endX + y * source->width()] > 0)
          break;
      }

      if (y == source->height())
        break;
    }

    Recti area(startX, 0, endX - startX, source->height());

    Ref<Image> glyphImage = source->area(area);
    if (!glyphImage)
    {
      logError("Failed to extract glyph image for font %s", name.c_str());
      return false;
    }

    data.characters[(unsigned char) characters[index++]] = data.glyphs.size();

    data.glyphs.push_back(FontGlyphData());
    FontGlyphData& glyph = data.glyphs.back();
    glyph.bearing = vec2(0.f, glyphImage->height() / 2.f);
    glyph.advance = float(glyphImage->width());
    glyph.image = glyphImage;

    startX = endX;
  }

  // HACK: Make digits same width
  {
    std::vector<FontGlyphData*> digits;

    float maxAdvance = 0.f;

    for (int c = '0';  c <= '9';  c++)
    {
      const int index = data.characters[c];
      if (index == -1)
        continue;

      FontGlyphData& glyph = data.glyphs[index];
      if (glyph.advance > maxAdvance)
        maxAdvance = glyph.advance;

      digits.push_back(&glyph);
    }

    for (auto d : digits)
    {
      d->bearing.x = (maxAdvance - d->advance) / 2.f;
      d->advance = maxAdvance;
    }
  }

  float maxAdvance = 0.f;
  float meanAdvance = 0.f;

  for (auto& g : data.glyphs)
  {
    maxAdvance = max(maxAdvance, g.advance);
    meanAdvance += g.advance;
  }

  meanAdvance /= float(data.glyphs.size());

  // HACK: Create space glyph if not already present

  if (data.characters[' '] == -1)
  {
    data.characters[' '] = data.glyphs.size();

    data.glyphs.push_back(FontGlyphData());
    FontGlyphData& glyph = data.glyphs.back();

    glyph.bearing = vec2(0.f);
    glyph.image = Image::create(cache, source->format(), 1, 1);

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
    for (auto& g : data.glyphs)
    {
      g.advance = maxAdvance;
      g.bearing.x = (g.advance - g.image->width()) / 2.f;
    }
  }
  else
  {
    for (auto& g : data.glyphs)
      g.advance += meanAdvance * 0.2f;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
