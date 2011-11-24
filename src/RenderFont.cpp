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

void Font::drawText(const vec2& penPosition, const vec4& color, const char* text) const
{
  const size_t length = std::strlen(text);

  GL::VertexRange vertexRange;
  if (!pool.allocateVertices(vertexRange, length * 6, Vertex2ft2fv::format))
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

    std::vector<Vertex2ft2fv> vertices(length * 6);

    Layout layout;

    for (const char* c = text;  *c != '\0';  c++)
    {
      if (const Glyph* glyph = findGlyph(*c))
      {
        getGlyphLayout(layout, *glyph, *c);
        layout.area.position += roundedPen;
        roundedPen += layout.advance;

        realizeVertices(layout.area, glyph->area, &vertices[count]);
        count += 6;
      }
    }

    vertexRange.copyFrom(&vertices[0]);
  }

  if (!count)
    return;

  pass.setUniformState(colorIndex, color);
  pass.apply();

  pool.getContext().render(GL::PrimitiveRange(GL::TRIANGLE_LIST,
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

  // Create glyph texture
  {
    const unsigned int maxSize = pool.getContext().getLimits().getMaxTextureSize();

    unsigned int totalWidth = 1;

    for (size_t i = 0;  i < data.glyphs.size();  i++)
      totalWidth += data.glyphs[i].image->getWidth() + 1;

    unsigned int textureWidth = min(powerOfTwoAbove(totalWidth), maxSize);

    unsigned int rows = totalWidth / textureWidth;
    if (totalWidth % textureWidth)
      rows++;

    unsigned int textureHeight = (maxHeight + 1) * rows + 1;
    textureHeight = min(powerOfTwoAbove(textureHeight), maxSize);

    Image image(getCache(), PixelFormat::R8, textureWidth, textureHeight);

    texture = GL::Texture::create(getCache(), pool.getContext(), image, 0);
    if (!texture)
    {
      logError("Failed to create glyph texture for font \'%s\'",
               getPath().asString().c_str());
      return false;
    }

    log("Allocated texture of size %ux%u format \'%s\' for font \'%s\'",
        texture->getWidth(),
        texture->getHeight(),
        texture->getFormat().asString().c_str(),
        getPath().asString().c_str());

    texture->setFilterMode(GL::FILTER_NEAREST);
  }

  vec2 texelOffset;
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
    interface.addUniform("color", GL::Uniform::VEC4);
    interface.addAttribute("wyPosition", GL::Attribute::VEC2);
    interface.addAttribute("wyTexCoord", GL::Attribute::VEC2);

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
    pass.setSamplerState("glyphs", texture);
    pass.setUniformState("color", vec4(1.f));

    colorIndex = pass.getUniformStateIndex("color");
  }

  ascender = descender = 0.f;

  ivec2 texelPosition(1, 1);

  GL::TextureImage* textureImage = texture->getImage(0);
  const unsigned int textureWidth = textureImage->getWidth();
  const unsigned int textureHeight = textureImage->getHeight();

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
                 getPath().asString().c_str());
        return false;
      }
    }

    if (!textureImage->copyFrom(*image, texelPosition.x, texelPosition.y))
    {
      logError("Failed to copy glyph image data for font \'%s\'",
               getPath().asString().c_str());
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

void Font::realizeVertices(const Rect& pixelArea,
                           const Rect& texelArea,
                           Vertex2ft2fv* vertices) const
{
  const Rect& pa = pixelArea;
  const Rect& ta = texelArea;

  vertices[0].texCoord = vec2(ta.position.x, ta.position.y);
  vertices[0].position = vec2(pa.position.x, pa.position.y);
  vertices[1].texCoord = vec2(ta.position.x + ta.size.x, ta.position.y);
  vertices[1].position = vec2(pa.position.x + pa.size.x, pa.position.y);
  vertices[2].texCoord = vec2(ta.position.x + ta.size.x, ta.position.y + ta.size.y);
  vertices[2].position = vec2(pa.position.x + pa.size.x, pa.position.y + pa.size.y);

  vertices[3] = vertices[2];
  vertices[4].texCoord = vec2(ta.position.x, ta.position.y + ta.size.y);
  vertices[4].position = vec2(pa.position.x, pa.position.y + pa.size.y);
  vertices[5] = vertices[0];
}

///////////////////////////////////////////////////////////////////////

FontReader::FontReader(GeometryPool& initPool):
  ResourceReader(initPool.getContext().getCache()),
  pool(initPool)
{
}

Ref<Font> FontReader::read(const Path& path)
{
  if (Resource* cached = getCache().findResource(path))
    return dynamic_cast<Font*>(cached);

  std::ifstream stream;
  if (!getCache().openFile(stream, path))
    return NULL;

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load font \'%s\': %s",
             path.asString().c_str(),
             result.description());
    return NULL;
  }

  pugi::xml_node root = document.child("font");
  if (!root || root.attribute("version").as_uint() != FONT_XML_VERSION)
  {
    logError("Font file format mismatch in \'%s\'",
             path.asString().c_str());
    return NULL;
  }

  const String characters(root.attribute("characters").value());
  if (characters.empty())
  {
    logError("No characters specified for font \'%s\'",
             path.asString().c_str());
    return NULL;
  }

  const Path imagePath(root.attribute("image").value());
  if (imagePath.isEmpty())
  {
    logError("Glyph image path missing for font \'%s\'",
              path.asString().c_str());
    return NULL;
  }

  Ref<Image> image = Image::read(getCache(), imagePath);
  if (!image)
  {
    logError("Failed to load glyph image for font \'%s\'",
             path.asString().c_str());
    return NULL;
  }

  bool fixedWidth = false;

  if (pugi::xml_attribute a = root.attribute("fixed"))
    fixedWidth = a.as_bool();

  FontData data;

  if (!extractGlyphs(data, path, *image, characters, fixedWidth))
    return NULL;

  return Font::create(ResourceInfo(getCache(), path), pool, data);
}

bool FontReader::extractGlyphs(FontData& data,
                               const Path& path,
                               const Image& image,
                               const String& characters,
                               bool fixedWidth)
{
  if (image.getFormat() != PixelFormat::R8)
  {
    logError("Image \'%s\' for font \'%s\' has invalid pixel format \'%s\'",
             image.getPath().asString().c_str(),
             path.asString().c_str(),
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
               path.asString().c_str());
      return false;
    }

    const unsigned int endY = findEndY(source);

    if (!source.crop(Recti(0, startY, source.getWidth(), endY - startY)))
    {
      logError("Failed to crop source image for font \'%s\'",
               path.asString().c_str());
      return false;
    }
  }

  data.glyphs.reserve(characters.length());

  const uint8* pixels = (const uint8*) source.getPixels();

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
               path.asString().c_str());
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
               path.asString().c_str());
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
    glyph.image = new Image(getCache(), source.getFormat(), 1, 1);

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
