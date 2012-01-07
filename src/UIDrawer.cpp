//////////////////////////////////////////////////////////////////////
// Wendy user interface library
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

#include <wendy/Core.h>
#include <wendy/Bimap.h>

#include <wendy/UIDrawer.h>

#include <pugixml.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

namespace
{

Bimap<String, WidgetState> widgetStateMap;

class ElementVertex
{
public:
  inline void set(const vec2& newSizeScale, const vec2& newOffsetScale, const vec2& newTexScale)
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

VertexFormat ElementVertex::format("2f:sizeScale 2f:offsetScale 2f:texScale");

const unsigned int THEME_XML_VERSION = 3;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Alignment::Alignment(HorzAlignment initHorizontal,
                     VertAlignment initVertical):
  horizontal(initHorizontal),
  vertical(initVertical)
{
}

void Alignment::set(HorzAlignment newHorizontal, VertAlignment newVertical)
{
  horizontal = newHorizontal;
  vertical = newVertical;
}

///////////////////////////////////////////////////////////////////////

Theme::Theme(const ResourceInfo& info):
  Resource(info)
{
}

Ref<Theme> Theme::read(render::GeometryPool& pool, const String& name)
{
  ThemeReader reader(pool);
  return reader.read(name);
}

///////////////////////////////////////////////////////////////////////

ThemeReader::ThemeReader(render::GeometryPool& initPool):
  ResourceReader<Theme>(initPool.getContext().getCache()),
  pool(&initPool)
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
  std::ifstream stream(path.asString().c_str());
  if (stream.fail())
  {
    logError("Failed to open animation \'%s\'", name.c_str());
    return NULL;
  }

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load UI theme \'%s\': %s",
             name.c_str(),
             result.description());
    return NULL;
  }

  pugi::xml_node root = document.child("theme");
  if (!root || root.attribute("version").as_uint() != THEME_XML_VERSION)
  {
    logError("UI theme file format mismatch in \'%s\'", name.c_str());
    return NULL;
  }

  Ref<Theme> theme = new Theme(ResourceInfo(cache, name, path));

  const String imageName(root.attribute("image").value());
  if (imageName.empty())
  {
    logError("No image specified for UI theme \'%s\'", name.c_str());
    return NULL;
  }

  theme->texture = GL::Texture::read(pool->getContext(),
                                     GL::TEXTURE_RECT,
                                     imageName);
  if (!theme->texture)
  {
    logError("Failed to create texture for UI theme \'%s\'", name.c_str());
    return NULL;
  }

  const String fontName(root.attribute("font").value());
  if (fontName.empty())
  {
    logError("Font for UI theme \'%s\' is empty", name.c_str());
    return NULL;
  }

  theme->font = render::Font::read(*pool, fontName);
  if (!theme->font)
  {
    logError("Failed to load font for UI theme \'%s\'", name.c_str());
    return NULL;
  }

  const vec3 scale(1.f / 255.f);

  for (pugi::xml_node sn = root.first_child();  sn;  sn = sn.next_sibling())
  {
    if (!widgetStateMap.hasKey(sn.name()))
    {
      logError("Unknown widget state \'%s\' in UI theme \'%s\'",
               sn.name(),
               name.c_str());
      return NULL;
    }

    WidgetState state = widgetStateMap[sn.name()];

    if (pugi::xml_node node = sn.child("text"))
      theme->textColors[state] = vec3Cast(node.attribute("color").value()) * scale;

    if (pugi::xml_node node = sn.child("back"))
      theme->backColors[state] = vec3Cast(node.attribute("color").value()) * scale;

    if (pugi::xml_node node = sn.child("caret"))
      theme->caretColors[state] = vec3Cast(node.attribute("color").value()) * scale;

    if (pugi::xml_node node = sn.child("button"))
      theme->buttonElements[state] = rectCast(node.attribute("area").value());

    if (pugi::xml_node node = sn.child("handle"))
      theme->handleElements[state] = rectCast(node.attribute("area").value());

    if (pugi::xml_node node = sn.child("frame"))
      theme->frameElements[state] = rectCast(node.attribute("area").value());

    if (pugi::xml_node node = sn.child("well"))
      theme->wellElements[state] = rectCast(node.attribute("area").value());

    if (pugi::xml_node node = sn.child("tab"))
      theme->tabElements[state] = rectCast(node.attribute("area").value());
  }

  return theme;
}

///////////////////////////////////////////////////////////////////////

void Drawer::begin()
{
  GL::Context& context = getContext();

  GL::Framebuffer& framebuffer = context.getCurrentFramebuffer();
  const unsigned int width = framebuffer.getWidth();
  const unsigned int height = framebuffer.getHeight();

  context.setCurrentSharedProgramState(state);
  context.setViewportArea(Recti(0, 0, width, height));
  context.setScissorArea(Recti(0, 0, width, height));

  state->setOrthoProjectionMatrix(float(width), float(height));
}

void Drawer::end()
{
  getContext().setCurrentSharedProgramState(NULL);
}

bool Drawer::pushClipArea(const Rect& area)
{
  if (!clipAreaStack.push(area))
    return false;

  const Rect& total = clipAreaStack.getTotal();

  GL::Context& context = getContext();
  context.setScissorArea(Recti(ivec2(total.position), ivec2(total.size)));

  return true;
}

void Drawer::popClipArea()
{
  clipAreaStack.pop();

  GL::Context& context = getContext();
  GL::Framebuffer& framebuffer = context.getCurrentFramebuffer();

  Recti area;

  if (clipAreaStack.isEmpty())
    area.set(0, 0, framebuffer.getWidth(), framebuffer.getHeight());
  else
  {
    const Rect& total = clipAreaStack.getTotal();
    area.set(ivec2(total.position), ivec2(total.size));
  }

  context.setScissorArea(area);
}

void Drawer::drawPoint(const vec2& point, const vec4& color)
{
  Vertex2fv vertex;
  vertex.position = point;

  GL::VertexRange range;
  if (!getGeometryPool().allocateVertices(range, 1, Vertex2fv::format))
    return;

  range.copyFrom(&vertex);

  setDrawingState(color, true);

  getContext().render(GL::PrimitiveRange(GL::POINT_LIST, range));
}

void Drawer::drawLine(const Segment2& segment, const vec4& color)
{
  Vertex2fv vertices[2];
  vertices[0].position = segment.start;
  vertices[1].position = segment.end;

  GL::VertexRange range;
  if (!getGeometryPool().allocateVertices(range, 2, Vertex2fv::format))
    return;

  range.copyFrom(vertices);

  setDrawingState(color, true);

  getContext().render(GL::PrimitiveRange(GL::LINE_LIST, range));
}

void Drawer::drawTriangle(const Triangle2& triangle, const vec4& color)
{
  Vertex2fv vertices[3];
  vertices[0].position = triangle.P[0];
  vertices[1].position = triangle.P[1];
  vertices[2].position = triangle.P[2];

  GL::VertexRange range;
  if (!getGeometryPool().allocateVertices(range, 3, Vertex2fv::format))
    return;

  range.copyFrom(vertices);

  setDrawingState(color, true);

  getContext().render(GL::PrimitiveRange(GL::TRIANGLE_LIST, range));
}

void Drawer::drawBezier(const BezierCurve2& spline, const vec4& color)
{
  BezierCurve2::PointList points;
  spline.tessellate(points);

  GL::VertexRange range;
  if (!getGeometryPool().allocateVertices(range, points.size(), Vertex2fv::format))
    return;

  // Realize vertices
  {
    GL::VertexRangeLock<Vertex2fv> vertices(range);

    for (unsigned int i = 0;  i < points.size();  i++)
      vertices[i].position = points[i];
  }

  setDrawingState(color, true);

  getContext().render(GL::PrimitiveRange(GL::LINE_STRIP, range));
}

void Drawer::drawRectangle(const Rect& rectangle, const vec4& color)
{
  float minX, minY, maxX, maxY;
  rectangle.getBounds(minX, minY, maxX, maxY);

  if (maxX - minX < 1.f || maxY - minY < 1.f)
    return;

  maxX -= 1.f;
  maxY -= 1.f;

  Vertex2fv vertices[4];
  vertices[0].position = vec2(minX, minY);
  vertices[1].position = vec2(maxX, minY);
  vertices[2].position = vec2(maxX, maxY);
  vertices[3].position = vec2(minX, maxY);

  GL::VertexRange range;
  if (!getGeometryPool().allocateVertices(range, 4, Vertex2fv::format))
    return;

  range.copyFrom(vertices);

  setDrawingState(color, true);

  getContext().render(GL::PrimitiveRange(GL::LINE_LOOP, range));
}

void Drawer::fillTriangle(const Triangle2& triangle, const vec4& color)
{
  Vertex2fv vertices[3];
  vertices[0].position = triangle.P[0];
  vertices[1].position = triangle.P[1];
  vertices[2].position = triangle.P[2];

  GL::VertexRange range;
  if (!getGeometryPool().allocateVertices(range, 3, Vertex2fv::format))
    return;

  range.copyFrom(vertices);

  setDrawingState(color, false);

  getContext().render(GL::PrimitiveRange(GL::TRIANGLE_LIST, range));
}

void Drawer::fillRectangle(const Rect& rectangle, const vec4& color)
{
  float minX, minY, maxX, maxY;
  rectangle.getBounds(minX, minY, maxX, maxY);

  if (maxX - minX < 1.f || maxY - minY < 1.f)
    return;

  maxX -= 1.f;
  maxY -= 1.f;

  Vertex2fv vertices[4];
  vertices[0].position = vec2(minX, minY);
  vertices[1].position = vec2(maxX, minY);
  vertices[2].position = vec2(maxX, maxY);
  vertices[3].position = vec2(minX, maxY);

  GL::VertexRange range;
  if (!getGeometryPool().allocateVertices(range, 4, Vertex2fv::format))
    return;

  range.copyFrom(vertices);

  setDrawingState(color, false);

  getContext().render(GL::PrimitiveRange(GL::TRIANGLE_FAN, range));
}

void Drawer::blitTexture(const Rect& area, GL::Texture& texture)
{
  float minX, minY, maxX, maxY;
  area.getBounds(minX, minY, maxX, maxY);

  if (maxX - minX < 1.f || maxY - minY < 1.f)
    return;

  maxX -= 1.f;
  maxY -= 1.f;

  Vertex2ft2fv vertices[4];
  vertices[0].texCoord = vec2(0.f, 0.f);
  vertices[0].position = vec2(minX, minY);
  vertices[1].texCoord = vec2(1.f, 0.f);
  vertices[1].position = vec2(maxX, minY);
  vertices[2].texCoord = vec2(1.f, 1.f);
  vertices[2].position = vec2(maxX, maxY);
  vertices[3].texCoord = vec2(0.f, 1.f);
  vertices[3].position = vec2(minX, maxY);

  GL::VertexRange range;
  if (!getGeometryPool().allocateVertices(range, 4, Vertex2ft2fv::format))
    return;

  range.copyFrom(vertices);

  if (texture.getFormat().getSemantic() == PixelFormat::RGBA)
    blitPass.setBlendFactors(GL::BLEND_SRC_ALPHA, GL::BLEND_ONE_MINUS_SRC_ALPHA);
  else
    blitPass.setBlendFactors(GL::BLEND_ONE, GL::BLEND_ZERO);

  blitPass.setSamplerState("image", &texture);
  blitPass.apply();

  getContext().render(GL::PrimitiveRange(GL::TRIANGLE_FAN, range));

  blitPass.setSamplerState("image", NULL);
}

void Drawer::drawText(const Rect& area,
                      const char* text,
                      const Alignment& alignment,
                      const vec3& color)
{
  Rect metrics = currentFont->getTextMetrics(text);

  vec2 penPosition;

  switch (alignment.horizontal)
  {
    case LEFT_ALIGNED:
      penPosition.x = area.position.x - metrics.position.x;
      break;
    case CENTERED_ON_X:
      penPosition.x = area.getCenter().x - metrics.getCenter().x;
      break;
    case RIGHT_ALIGNED:
      penPosition.x = (area.position.x + area.size.x) -
                      (metrics.position.x + metrics.size.x);
      break;
    default:
      panic("Invalid horizontal alignment");
  }

  switch (alignment.vertical)
  {
    case BOTTOM_ALIGNED:
      penPosition.y = area.position.y - metrics.position.y;
      break;
    case CENTERED_ON_Y:
      penPosition.y = area.getCenter().y - metrics.getCenter().y;
      break;
    case TOP_ALIGNED:
      penPosition.y = (area.position.y + area.size.y) -
                      (metrics.position.y + metrics.size.y);
      break;
    default:
      panic("Invalid vertical alignment");
  }

  currentFont->drawText(penPosition, vec4(color, 1.f), text);
}

void Drawer::drawText(const Rect& area,
                      const char* text,
                      const Alignment& alignment,
                      WidgetState state)
{
  drawText(area, text, alignment, theme->textColors[state]);
}

void Drawer::drawWell(const Rect& area, WidgetState state)
{
  drawElement(area, theme->wellElements[state]);
}

void Drawer::drawFrame(const Rect& area, WidgetState state)
{
  drawElement(area, theme->frameElements[state]);
}

void Drawer::drawHandle(const Rect& area, WidgetState state)
{
  drawElement(area, theme->handleElements[state]);
}

void Drawer::drawButton(const Rect& area, WidgetState state, const char* text)
{
  drawElement(area, theme->buttonElements[state]);

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

void Drawer::drawTab(const Rect& area, WidgetState state, const char* text)
{
  drawElement(area, theme->tabElements[state]);
  drawText(area, text, Alignment(), state);
}

const Theme& Drawer::getTheme() const
{
  return *theme;
}

GL::Context& Drawer::getContext()
{
  return pool->getContext();
}

render::GeometryPool& Drawer::getGeometryPool()
{
  return *pool;
}

render::Font& Drawer::getCurrentFont()
{
  return *currentFont;
}

void Drawer::setCurrentFont(render::Font* newFont)
{
  if (newFont)
    currentFont = newFont;
  else
    currentFont = theme->font;
}

float Drawer::getCurrentEM() const
{
  return currentFont->getHeight();
}

Ref<Drawer> Drawer::create(render::GeometryPool& pool)
{
  Ptr<Drawer> drawer(new Drawer(pool));
  if (!drawer->init())
    return NULL;

  return drawer.detachObject();
}

Drawer::Drawer(render::GeometryPool& initPool):
  pool(&initPool)
{
}

bool Drawer::init()
{
  GL::Context& context = getContext();

  state = new render::SharedProgramState();
  if (!state->reserveSupported(context))
    return false;

  // Set up element geometry
  {
    vertexBuffer = GL::VertexBuffer::create(context, 16, ElementVertex::format, GL::VertexBuffer::STATIC);
    if (!vertexBuffer)
      return false;

    ElementVertex* vertices = (ElementVertex*) vertexBuffer->lock();

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

    vertexBuffer->unlock();

    indexBuffer = GL::IndexBuffer::create(context, 54, GL::IndexBuffer::UINT8, GL::IndexBuffer::STATIC);
    if (!indexBuffer)
      return false;

    uint8* indices = (uint8*) indexBuffer->lock();

    // This is a perfectly normal indexed triangle list using the vertices above

    for (int y = 0;  y < 3;  y++)
    {
      for (int x = 0;  x < 3;  x++)
      {
        *indices++ = x + y * 4;
        *indices++ = (x + 1) + (y + 1) * 4;
        *indices++ = x + (y + 1) * 4;

        *indices++ = x + y * 4;
        *indices++ = (x + 1) + y * 4;
        *indices++ = (x + 1) + (y + 1) * 4;
      }
    }

    indexBuffer->unlock();

    range = GL::PrimitiveRange(GL::TRIANGLE_LIST, *vertexBuffer, *indexBuffer);
  }

  // Load default theme
  {
    const String themeName("wendy/UIDefault.theme");

    theme = Theme::read(getGeometryPool(), themeName);
    if (!theme)
    {
      logError("Failed to load default UI theme \'%s\'", themeName.c_str());
      return false;
    }

    currentFont = theme->font;
  }

  // Set up solid pass
  {
    Ref<GL::Program> program = GL::Program::read(context,
                                                 "wendy/UIElement.vs",
                                                 "wendy/UIElement.fs");
    if (!program)
    {
      logError("Failed to load UI element program");
      return false;
    }

    GL::ProgramInterface interface;
    interface.addUniform("elementPos", GL::UNIFORM_VEC2);
    interface.addUniform("elementSize", GL::UNIFORM_VEC2);
    interface.addUniform("texPos", GL::UNIFORM_VEC2);
    interface.addUniform("texSize", GL::UNIFORM_VEC2);
    interface.addSampler("image", GL::SAMPLER_RECT);
    interface.addAttributes(ElementVertex::format);

    if (!interface.matches(*program, true))
    {
      logError("UI element program \'%s\' does not conform to the required interface",
               program->getName().c_str());
      return false;
    }

    elementPass.setProgram(program);
    elementPass.setDepthTesting(false);
    elementPass.setDepthWriting(false);
    elementPass.setSamplerState("image", theme->texture);
    elementPass.setBlendFactors(GL::BLEND_SRC_ALPHA, GL::BLEND_ONE_MINUS_SRC_ALPHA);
    elementPass.setMultisampling(false);

    elementPosIndex = elementPass.getUniformStateIndex("elementPos");
    elementSizeIndex = elementPass.getUniformStateIndex("elementSize");
    texPosIndex = elementPass.getUniformStateIndex("texPos");
    texSizeIndex = elementPass.getUniformStateIndex("texSize");
  }

  // Set up solid pass
  {
    Ref<GL::Program> program = GL::Program::read(context,
                                                 "wendy/UIDrawSolid.vs",
                                                 "wendy/UIDrawSolid.fs");
    if (!program)
    {
      logError("Failed to load UI drawing shader program");
      return false;
    }

    GL::ProgramInterface interface;
    interface.addUniform("color", GL::UNIFORM_VEC4);
    interface.addAttributes(Vertex2fv::format);

    if (!interface.matches(*program, true))
    {
      logError("UI drawing shader program \'%s\' does not conform to the required interface",
               program->getName().c_str());
      return false;
    }

    drawPass.setProgram(program);
    drawPass.setCullMode(GL::CULL_NONE);
    drawPass.setDepthTesting(false);
    drawPass.setDepthWriting(false);
    drawPass.setMultisampling(false);
  }

  // Set up blitting pass
  {
    Ref<GL::Program> program = GL::Program::read(context,
                                                 "wendy/UIDrawMapped.vs",
                                                 "wendy/UIDrawMapped.fs");
    if (!program)
    {
      logError("Failed to load UI blitting shader program");
      return false;
    }

    GL::ProgramInterface interface;
    interface.addSampler("image", GL::SAMPLER_2D);
    interface.addAttributes(Vertex2ft2fv::format);

    if (!interface.matches(*program, true))
    {
      logError("UI blitting shader program \'%s\' does not conform to the required interface",
               program->getName().c_str());
      return false;
    }

    blitPass.setProgram(program);
    blitPass.setCullMode(GL::CULL_NONE);
    blitPass.setDepthTesting(false);
    blitPass.setDepthWriting(false);
    blitPass.setMultisampling(false);
  }

  return true;
}

void Drawer::drawElement(const Rect& area, const Rect& mapping)
{
  elementPass.setUniformState(elementPosIndex, area.position);
  elementPass.setUniformState(elementSizeIndex, area.size);
  elementPass.setUniformState(texPosIndex, mapping.position);
  elementPass.setUniformState(texSizeIndex, mapping.size);
  elementPass.apply();

  getContext().render(range);
}

void Drawer::setDrawingState(const vec4& color, bool wireframe)
{
  drawPass.setUniformState("color", color);

  if (color.a == 1.f)
    drawPass.setBlendFactors(GL::BLEND_ONE, GL::BLEND_ZERO);
  else
    drawPass.setBlendFactors(GL::BLEND_SRC_ALPHA, GL::BLEND_ONE_MINUS_SRC_ALPHA);

  drawPass.setWireframe(wireframe);
  drawPass.apply();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
