//////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#include <wendy/Core.hpp>
#include <wendy/Bimap.hpp>

#include <wendy/Drawer.hpp>

#include <pugixml.hpp>

#include <fstream>

namespace wendy
{

namespace
{

Bimap<String, WidgetState> widgetStateMap;

class ElementVertex
{
public:
  inline void set(vec2 newSizeScale, vec2 newOffsetScale, vec2 newTexScale)
  {
    sizeScale = newSizeScale;
    offsetScale = newOffsetScale;
    texScale = newTexScale;
  }
  vec2 sizeScale;
  vec2 offsetScale;
  vec2 texScale;
  static VertexFormat format;
};

VertexFormat ElementVertex::format("2f:vSizeScale 2f:vOffsetScale 2f:vTexScale");

const uint THEME_XML_VERSION = 3;

} /*namespace*/

Alignment::Alignment(HorzAlignment horizontal, VertAlignment vertical):
  horizontal(horizontal),
  vertical(vertical)
{
}

void Alignment::set(HorzAlignment newHorizontal, VertAlignment newVertical)
{
  horizontal = newHorizontal;
  vertical = newVertical;
}

Theme::Theme(const ResourceInfo& info):
  Resource(info)
{
}

Ref<Theme> Theme::read(RenderContext& context, const String& name)
{
  ThemeReader reader(context);
  return reader.read(name);
}

ThemeReader::ThemeReader(RenderContext& context):
  ResourceReader<Theme>(context.cache()),
  m_context(context)
{
  if (widgetStateMap.isEmpty())
  {
    widgetStateMap["disabled"] = STATE_DISABLED;
    widgetStateMap["normal"] = STATE_NORMAL;
    widgetStateMap["active"] = STATE_ACTIVE;
    widgetStateMap["selected"] = STATE_SELECTED;
  }
}

Ref<Theme> ThemeReader::read(const String& name, const Path& path)
{
  std::ifstream stream(path.name());
  if (stream.fail())
  {
    logError("Failed to open animation %s", name.c_str());
    return nullptr;
  }

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load UI theme %s: %s",
             name.c_str(),
             result.description());
    return nullptr;
  }

  pugi::xml_node root = document.child("theme");
  if (!root || root.attribute("version").as_uint() != THEME_XML_VERSION)
  {
    logError("UI theme file format mismatch in %s", name.c_str());
    return nullptr;
  }

  Ref<Theme> theme = new Theme(ResourceInfo(cache, name, path));

  const String imageName(root.attribute("image").value());
  if (imageName.empty())
  {
    logError("No image specified for UI theme %s", name.c_str());
    return nullptr;
  }

  const TextureParams params(TEXTURE_RECT, TF_NONE, FILTER_BILINEAR, ADDRESS_CLAMP);

  theme->m_texture = Texture::read(m_context, params, imageName);
  if (!theme->m_texture)
  {
    logError("Failed to create texture for UI theme %s", name.c_str());
    return nullptr;
  }

  const String fontName(root.attribute("font").value());
  if (fontName.empty())
  {
    logError("Font for UI theme %s is empty", name.c_str());
    return nullptr;
  }

  theme->m_font = Font::read(m_context, fontName);
  if (!theme->m_font)
  {
    logError("Failed to load font for UI theme %s", name.c_str());
    return nullptr;
  }

  const vec3 scale(1.f / 255.f);

  for (auto sn : root.children())
  {
    if (!widgetStateMap.hasKey(sn.name()))
    {
      logError("Unknown widget state %s in UI theme %s",
               sn.name(),
               name.c_str());
      return nullptr;
    }

    WidgetState state = widgetStateMap[sn.name()];

    if (pugi::xml_node node = sn.child("text"))
      theme->m_textColors[state] = vec3Cast(node.attribute("color").value()) * scale;

    if (pugi::xml_node node = sn.child("back"))
      theme->m_backColors[state] = vec3Cast(node.attribute("color").value()) * scale;

    if (pugi::xml_node node = sn.child("caret"))
      theme->m_caretColors[state] = vec3Cast(node.attribute("color").value()) * scale;

    if (pugi::xml_node node = sn.child("button"))
      theme->m_buttonElements[state] = rectCast(node.attribute("area").value());

    if (pugi::xml_node node = sn.child("handle"))
      theme->m_handleElements[state] = rectCast(node.attribute("area").value());

    if (pugi::xml_node node = sn.child("frame"))
      theme->m_frameElements[state] = rectCast(node.attribute("area").value());

    if (pugi::xml_node node = sn.child("well"))
      theme->m_wellElements[state] = rectCast(node.attribute("area").value());

    if (pugi::xml_node node = sn.child("check"))
      theme->m_checkElements[state] = rectCast(node.attribute("area").value());

    if (pugi::xml_node node = sn.child("clear"))
      theme->m_clearElements[state] = rectCast(node.attribute("area").value());

    if (pugi::xml_node node = sn.child("tab"))
      theme->m_tabElements[state] = rectCast(node.attribute("area").value());
  }

  return theme;
}

void Drawer::begin()
{
  Framebuffer& framebuffer = m_context.currentFramebuffer();
  const uint width = framebuffer.width();
  const uint height = framebuffer.height();

  m_context.setCurrentSharedProgramState(m_state);
  m_context.setViewportArea(Recti(0, 0, width, height));
  m_context.setScissorArea(Recti(0, 0, width, height));

  m_state->setOrthoProjectionMatrix(float(width), float(height));
}

void Drawer::end()
{
  m_context.setCurrentSharedProgramState(nullptr);
}

bool Drawer::pushClipArea(const Rect& area)
{
  return m_clipAreaStack.push(area);
}

void Drawer::popClipArea()
{
  m_clipAreaStack.pop();
}

void Drawer::drawPoint(vec2 point, vec4 color)
{
  Vertex2fv vertex;
  vertex.position = point;

  VertexRange range = m_context.allocateVertices(1, Vertex2fv::format);
  if (range.isEmpty())
    return;

  range.copyFrom(&vertex);
  setDrawingState(color, true);
  m_context.render(PrimitiveRange(POINT_LIST, range));
}

void Drawer::drawLine(vec2 start, vec2 end, vec4 color)
{
  Vertex2fv vertices[2];
  vertices[0].position = start;
  vertices[1].position = end;

  VertexRange range = m_context.allocateVertices(2, Vertex2fv::format);
  if (range.isEmpty())
    return;

  range.copyFrom(vertices);
  setDrawingState(color, true);
  m_context.render(PrimitiveRange(LINE_LIST, range));
}

void Drawer::drawRectangle(const Rect& rectangle, vec4 color)
{
  float minX, minY, maxX, maxY;
  rectangle.bounds(minX, minY, maxX, maxY);

  if (maxX - minX < 1.f || maxY - minY < 1.f)
    return;

  Vertex2fv vertices[4];
  vertices[0].position = vec2(minX, minY);
  vertices[1].position = vec2(maxX, minY);
  vertices[2].position = vec2(maxX, maxY);
  vertices[3].position = vec2(minX, maxY);

  VertexRange range = m_context.allocateVertices(4, Vertex2fv::format);
  if (range.isEmpty())
    return;

  range.copyFrom(vertices);
  setDrawingState(color, true);
  m_context.render(PrimitiveRange(LINE_LOOP, range));
}

void Drawer::fillRectangle(const Rect& rectangle, vec4 color)
{
  float minX, minY, maxX, maxY;
  rectangle.bounds(minX, minY, maxX, maxY);

  if (maxX - minX < 1.f || maxY - minY < 1.f)
    return;

  Vertex2fv vertices[4];
  vertices[0].position = vec2(minX, minY);
  vertices[1].position = vec2(maxX, minY);
  vertices[2].position = vec2(maxX, maxY);
  vertices[3].position = vec2(minX, maxY);

  VertexRange range = m_context.allocateVertices(4, Vertex2fv::format);
  if (range.isEmpty())
    return;

  range.copyFrom(vertices);
  setDrawingState(color, false);
  m_context.render(PrimitiveRange(TRIANGLE_FAN, range));
}

void Drawer::blitTexture(const Rect& area, Texture& texture, vec4 color)
{
  float minX, minY, maxX, maxY;
  area.bounds(minX, minY, maxX, maxY);

  if (maxX - minX < 1.f || maxY - minY < 1.f)
    return;

  Vertex2ft2fv vertices[4];
  vertices[0].texcoord = vec2(0.f, 0.f);
  vertices[0].position = vec2(minX, minY);
  vertices[1].texcoord = vec2(1.f, 0.f);
  vertices[1].position = vec2(maxX, minY);
  vertices[2].texcoord = vec2(1.f, 1.f);
  vertices[2].position = vec2(maxX, maxY);
  vertices[3].texcoord = vec2(0.f, 1.f);
  vertices[3].position = vec2(minX, maxY);

  VertexRange range = m_context.allocateVertices(4, Vertex2ft2fv::format);
  if (range.isEmpty())
    return;

  range.copyFrom(vertices);

  if (color.a < 1.f || texture.format().semantic() == PixelFormat::RGBA)
    m_blitPass.setBlendFactors(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
  else
    m_blitPass.setBlendFactors(BLEND_ONE, BLEND_ZERO);

  m_blitPass.setUniformState("color", color);
  m_blitPass.setSamplerState("image", &texture);
  m_blitPass.apply();

  m_context.render(PrimitiveRange(TRIANGLE_FAN, range));
}

void Drawer::drawText(const Rect& area,
                      const char* text,
                      Alignment alignment,
                      vec3 color)
{
  const Rect bounds = m_font->boundsOf(text);

  vec2 pen;

  switch (alignment.horizontal)
  {
    case LEFT_ALIGNED:
      pen.x = area.position.x - bounds.position.x;
      break;
    case CENTERED_ON_X:
      pen.x = area.center().x - bounds.center().x;
      break;
    case RIGHT_ALIGNED:
      pen.x = (area.position.x + area.size.x) -
              (bounds.position.x + bounds.size.x);
      break;
    default:
      panic("Invalid horizontal alignment");
  }

  switch (alignment.vertical)
  {
    case BOTTOM_ALIGNED:
      pen.y = area.position.y - m_font->descender();
      break;
    case CENTERED_ON_Y:
      pen.y = area.center().y - m_font->descender() - m_font->height() / 2.f;
      break;
    case TOP_ALIGNED:
      pen.y = area.position.y + area.size.y - m_font->ascender();
      break;
    default:
      panic("Invalid vertical alignment");
  }

  m_font->drawText(pen, vec4(color, 1.f), text);
}

void Drawer::drawText(const Rect& area,
                      const char* text,
                      Alignment alignment,
                      WidgetState state)
{
  drawText(area, text, alignment, m_theme->m_textColors[state]);
}

void Drawer::drawWell(const Rect& area, WidgetState state)
{
  drawElement(area, m_theme->m_wellElements[state]);
}

void Drawer::drawFrame(const Rect& area, WidgetState state)
{
  drawElement(area, m_theme->m_frameElements[state]);
}

void Drawer::drawHandle(const Rect& area, WidgetState state)
{
  drawElement(area, m_theme->m_handleElements[state]);
}

void Drawer::drawButton(const Rect& area, WidgetState state, const char* text)
{
  drawElement(area, m_theme->m_buttonElements[state]);

  if (state == STATE_SELECTED)
  {
    const Rect textArea(area.position.x + 2.f,
                        area.position.y,
                        area.size.x - 2.f,
                        area.size.y - 2.f);

    drawText(textArea, text, Alignment(), state);
  }
  else
    drawText(area, text, Alignment(), state);
}

void Drawer::drawCheck(const Rect& area, WidgetState state, bool checked, const char* text)
{
  const float checkSize = min(area.size.x, area.size.y);
  const Rect checkArea(area.position + vec2(checkSize) * 0.2f,
                       vec2(checkSize) * 0.6f);

  if (checked)
    drawElement(checkArea, m_theme->m_checkElements[state]);
  else
    drawElement(checkArea, m_theme->m_clearElements[state]);

  const Rect textArea(area.position + vec2(checkSize, 0.f),
                      area.size - vec2(checkSize, 0.f));

  drawText(textArea, text, LEFT_ALIGNED, state);
}

void Drawer::drawTab(const Rect& area, WidgetState state, const char* text)
{
  drawElement(area, m_theme->m_tabElements[state]);
  drawText(area, text, Alignment(), state);
}

const Theme& Drawer::theme() const
{
  return *m_theme;
}

RenderContext& Drawer::context()
{
  return m_context;
}

Font& Drawer::currentFont()
{
  return *m_font;
}

void Drawer::setCurrentFont(Font* newFont)
{
  if (newFont)
    m_font = newFont;
  else
    m_font = m_theme->m_font;
}

Drawer* Drawer::create(RenderContext& context)
{
  std::unique_ptr<Drawer> drawer(new Drawer(context));
  if (!drawer->init())
    return nullptr;

  return drawer.release();
}

Drawer::Drawer(RenderContext& context):
  m_context(context)
{
}

bool Drawer::init()
{
  m_state = new SharedProgramState();

  // Set up element geometry
  {
    m_vertexBuffer = VertexBuffer::create(m_context, 16, ElementVertex::format, USAGE_STATIC);
    if (!m_vertexBuffer)
      return false;

    ElementVertex vertices[16];

    // These are scaling factors used when rendering UI widget elements
    //
    // There are three kinds:
    //  * The size scale, which when multiplied by the screen space size
    //    of the element places vertices in the closest corner
    //  * The offset scale, which when multiplied by the texture space size of
    //    the element pulls the vertices defining its inner edges towards the
    //    center of the element
    //  * The texture coordinate scale, which when multiplied by the texture
    //    space size of the element becomes the relative texture coordinate
    //    of that vertex
    //
    // This allows rendering of UI elements by changing only four uniforms: the
    // position and size of the element in screen and texture space.

    vertices[0x0].set(vec2(0.f, 0.f), vec2(  0.f,   0.f), vec2( 0.f,  0.f));
    vertices[0x1].set(vec2(0.f, 0.f), vec2( 0.5f,   0.f), vec2(0.5f,  0.f));
    vertices[0x2].set(vec2(1.f, 0.f), vec2(-0.5f,   0.f), vec2(0.5f,  0.f));
    vertices[0x3].set(vec2(1.f, 0.f), vec2(  0.f,   0.f), vec2( 1.f,  0.f));

    vertices[0x4].set(vec2(0.f, 0.f), vec2(  0.f,  0.5f), vec2( 0.f, 0.5f));
    vertices[0x5].set(vec2(0.f, 0.f), vec2( 0.5f,  0.5f), vec2(0.5f, 0.5f));
    vertices[0x6].set(vec2(1.f, 0.f), vec2(-0.5f,  0.5f), vec2(0.5f, 0.5f));
    vertices[0x7].set(vec2(1.f, 0.f), vec2(  0.f,  0.5f), vec2( 1.f, 0.5f));

    vertices[0x8].set(vec2(0.f, 1.f), vec2(  0.f, -0.5f), vec2( 0.f, 0.5f));
    vertices[0x9].set(vec2(0.f, 1.f), vec2( 0.5f, -0.5f), vec2(0.5f, 0.5f));
    vertices[0xa].set(vec2(1.f, 1.f), vec2(-0.5f, -0.5f), vec2(0.5f, 0.5f));
    vertices[0xb].set(vec2(1.f, 1.f), vec2(  0.f, -0.5f), vec2( 1.f, 0.5f));

    vertices[0xc].set(vec2(0.f, 1.f), vec2(  0.f,   0.f), vec2( 0.f,  1.f));
    vertices[0xd].set(vec2(0.f, 1.f), vec2( 0.5f,   0.f), vec2(0.5f,  1.f));
    vertices[0xe].set(vec2(1.f, 1.f), vec2(-0.5f,   0.f), vec2(0.5f,  1.f));
    vertices[0xf].set(vec2(1.f, 1.f), vec2(  0.f,   0.f), vec2( 1.f,  1.f));

    m_vertexBuffer->copyFrom(vertices, 16);

    m_indexBuffer = IndexBuffer::create(m_context, 54, INDEX_UINT8, USAGE_STATIC);
    if (!m_indexBuffer)
      return false;

    uint8 indices[54];
    size_t index = 0;

    // This is a perfectly normal indexed triangle list using the vertices above

    for (int y = 0;  y < 3;  y++)
    {
      for (int x = 0;  x < 3;  x++)
      {
        indices[index++] = x + y * 4;
        indices[index++] = (x + 1) + (y + 1) * 4;
        indices[index++] = x + (y + 1) * 4;

        indices[index++] = x + y * 4;
        indices[index++] = (x + 1) + y * 4;
        indices[index++] = (x + 1) + (y + 1) * 4;
      }
    }

    m_indexBuffer->copyFrom(indices, 54);

    m_range = PrimitiveRange(TRIANGLE_LIST,
                             *m_vertexBuffer,
                             *m_indexBuffer);
  }

  // Load default theme
  {
    const String themeName("wendy/UIDefault.theme");

    m_theme = Theme::read(m_context, themeName);
    if (!m_theme)
    {
      logError("Failed to load default UI theme %s", themeName.c_str());
      return false;
    }

    m_font = m_theme->m_font;
  }

  // Set up solid pass
  {
    Ref<Program> program = Program::read(m_context,
                                         "wendy/UIElement.vs",
                                         "wendy/UIElement.fs");
    if (!program)
    {
      logError("Failed to load UI element program");
      return false;
    }

    ProgramInterface interface;
    interface.addUniform("elementPos", UNIFORM_VEC2);
    interface.addUniform("elementSize", UNIFORM_VEC2);
    interface.addUniform("texPos", UNIFORM_VEC2);
    interface.addUniform("texSize", UNIFORM_VEC2);
    interface.addSampler("image", SAMPLER_RECT);
    interface.addAttributes(ElementVertex::format);

    if (!interface.matches(*program, true))
    {
      logError("UI element program %s does not conform to the required interface",
               program->name().c_str());
      return false;
    }

    m_elementPass.setProgram(program);
    m_elementPass.setDepthTesting(false);
    m_elementPass.setDepthWriting(false);
    m_elementPass.setSamplerState("image", m_theme->m_texture);
    m_elementPass.setBlendFactors(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
    m_elementPass.setMultisampling(false);

    m_elementPosIndex = m_elementPass.uniformStateIndex("elementPos");
    m_elementSizeIndex = m_elementPass.uniformStateIndex("elementSize");
    m_texPosIndex = m_elementPass.uniformStateIndex("texPos");
    m_texSizeIndex = m_elementPass.uniformStateIndex("texSize");
  }

  // Set up solid pass
  {
    Ref<Program> program = Program::read(m_context,
                                         "wendy/UIDrawSolid.vs",
                                         "wendy/UIDrawSolid.fs");
    if (!program)
    {
      logError("Failed to load UI drawing shader program");
      return false;
    }

    ProgramInterface interface;
    interface.addUniform("color", UNIFORM_VEC4);
    interface.addAttributes(Vertex2fv::format);

    if (!interface.matches(*program, true))
    {
      logError("UI drawing shader program %s does not conform to the required interface",
               program->name().c_str());
      return false;
    }

    m_drawPass.setProgram(program);
    m_drawPass.setCullFace(FACE_NONE);
    m_drawPass.setDepthTesting(false);
    m_drawPass.setDepthWriting(false);
    m_drawPass.setMultisampling(false);
  }

  // Set up blitting pass
  {
    Ref<Program> program = Program::read(m_context,
                                         "wendy/UIDrawMapped.vs",
                                         "wendy/UIDrawMapped.fs");
    if (!program)
    {
      logError("Failed to load UI blitting shader program");
      return false;
    }

    ProgramInterface interface;
    interface.addSampler("image", SAMPLER_2D);
    interface.addUniform("color", UNIFORM_VEC4);
    interface.addAttributes(Vertex2ft2fv::format);

    if (!interface.matches(*program, true))
    {
      logError("UI blitting shader program %s does not conform to the required interface",
               program->name().c_str());
      return false;
    }

    m_blitPass.setProgram(program);
    m_blitPass.setCullFace(FACE_NONE);
    m_blitPass.setDepthTesting(false);
    m_blitPass.setDepthWriting(false);
    m_blitPass.setMultisampling(false);
  }

  return true;
}

void Drawer::drawElement(const Rect& area, const Rect& mapping)
{
  m_elementPass.setUniformState(m_elementPosIndex, area.position);
  m_elementPass.setUniformState(m_elementSizeIndex, area.size);
  m_elementPass.setUniformState(m_texPosIndex, mapping.position);
  m_elementPass.setUniformState(m_texSizeIndex, mapping.size);
  m_elementPass.apply();

  m_context.render(m_range);
}

void Drawer::setDrawingState(vec4 color, bool wireframe)
{
  m_drawPass.setUniformState("color", color);

  if (color.a == 1.f)
    m_drawPass.setBlendFactors(BLEND_ONE, BLEND_ZERO);
  else
    m_drawPass.setBlendFactors(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);

  m_drawPass.setWireframe(wireframe);
  m_drawPass.apply();
}

} /*namespace wendy*/

