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

const unsigned int THEME_XML_VERSION = 1;

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

Ref<Theme> Theme::read(render::GeometryPool& pool, const Path& path)
{
  ThemeReader reader(pool);
  return reader.read(path);
}

///////////////////////////////////////////////////////////////////////

ThemeReader::ThemeReader(render::GeometryPool& initPool):
  ResourceReader(initPool.getContext().getCache()),
  pool(initPool)
{
  if (widgetStateMap.isEmpty())
  {
    widgetStateMap["disabled"] = STATE_DISABLED;
    widgetStateMap["normal"] = STATE_NORMAL;
    widgetStateMap["active"] = STATE_ACTIVE;
    widgetStateMap["selected"] = STATE_SELECTED;
  }
}

Ref<Theme> ThemeReader::read(const Path& path)
{
  if (Resource* cached = getCache().findResource(path))
    return dynamic_cast<Theme*>(cached);

  std::ifstream stream;
  if (!getCache().openFile(stream, path))
    return NULL;

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load UI theme \'%s\': %s",
             path.asString().c_str(),
             result.description());
    return NULL;
  }

  pugi::xml_node root = document.child("theme");
  if (!root || root.attribute("version").as_uint() != THEME_XML_VERSION)
  {
    logError("UI theme file format mismatch in \'%s\'",
             path.asString().c_str());
    return NULL;
  }

  Ref<Theme> theme = new Theme(ResourceInfo(getCache(), path));

  const Path texturePath(root.attribute("texture").value());
  if (texturePath.isEmpty())
  {
    logError("Texture path for UI theme \'%s\' is empty",
             path.asString().c_str());
    return NULL;
  }

  theme->texture = GL::Texture::read(pool.getContext(), texturePath);
  if (!theme->texture)
  {
    logError("Failed to load texture for UI theme \'%s\'",
             path.asString().c_str());
    return NULL;
  }

  const Path fontPath(root.attribute("font").value());
  if (fontPath.isEmpty())
  {
    logError("Font path for UI theme \'%s\' is empty",
              path.asString().c_str());
    return NULL;
  }

  theme->font = render::Font::read(pool, fontPath);
  if (!theme->font)
  {
    logError("Failed to load font for UI theme \'%s\'",
             path.asString().c_str());
    return NULL;
  }

  for (pugi::xml_node sn = root.first_child();  sn;  sn = sn.next_sibling())
  {
    if (!widgetStateMap.hasKey(sn.name()))
    {
      logError("Unknown widget state \'%s\' in UI theme \'%s\'",
               sn.name(),
               path.asString().c_str());
      return NULL;
    }

    WidgetState state = widgetStateMap[sn.name()];

    if (pugi::xml_node node = sn.child("text"))
      theme->textColors[state] = vec3Cast(node.attribute("color").value());

    if (pugi::xml_node node = sn.child("back"))
      theme->backColors[state] = vec3Cast(node.attribute("color").value());

    if (pugi::xml_node node = sn.child("button"))
      theme->buttonElements[state] = rectCast(node.attribute("area").value());

    if (pugi::xml_node node = sn.child("handle"))
      theme->handleElements[state] = rectCast(node.attribute("area").value());

    if (pugi::xml_node node = sn.child("frame"))
      theme->frameElements[state] = rectCast(node.attribute("area").value());

    if (pugi::xml_node node = sn.child("well"))
      theme->wellElements[state] = rectCast(node.attribute("area").value());
  }

  return theme;
}

///////////////////////////////////////////////////////////////////////

void Drawer::begin()
{
  GL::Context& context = pool.getContext();

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
  pool.getContext().setCurrentSharedProgramState(NULL);
}

bool Drawer::pushClipArea(const Rect& area)
{
  if (!clipAreaStack.push(area))
    return false;

  const Rect& total = clipAreaStack.getTotal();

  GL::Context& context = pool.getContext();
  context.setScissorArea(Recti(int(total.position.x),
                               int(total.position.y),
                               int(total.size.x),
                               int(total.size.y)));

  return true;
}

void Drawer::popClipArea()
{
  clipAreaStack.pop();

  GL::Context& context = pool.getContext();
  GL::Framebuffer& framebuffer = context.getCurrentFramebuffer();

  Recti area;

  if (clipAreaStack.isEmpty())
    area.set(0, 0, framebuffer.getWidth(), framebuffer.getHeight());
  else
  {
    const Rect& total = clipAreaStack.getTotal();
    area.set(int(total.position.x),
             int(total.position.y),
             int(total.size.x),
             int(total.size.y));
  }

  context.setScissorArea(area);
}

void Drawer::drawPoint(const vec2& point, const vec4& color)
{
  Vertex2fv vertex;
  vertex.position = point;

  GL::VertexRange range;
  if (!pool.allocateVertices(range, 1, Vertex2fv::format))
    return;

  range.copyFrom(&vertex);

  setDrawingState(color, true);

  pool.getContext().render(GL::PrimitiveRange(GL::POINT_LIST, range));
}

void Drawer::drawLine(const Segment2& segment, const vec4& color)
{
  Vertex2fv vertices[2];
  vertices[0].position = segment.start;
  vertices[1].position = segment.end;

  GL::VertexRange range;
  if (!pool.allocateVertices(range, 2, Vertex2fv::format))
    return;

  range.copyFrom(vertices);

  setDrawingState(color, true);

  pool.getContext().render(GL::PrimitiveRange(GL::LINE_LIST, range));
}

void Drawer::drawTriangle(const Triangle2& triangle, const vec4& color)
{
  Vertex2fv vertices[3];
  vertices[0].position = triangle.P[0];
  vertices[1].position = triangle.P[1];
  vertices[2].position = triangle.P[2];

  GL::VertexRange range;
  if (!pool.allocateVertices(range, 3, Vertex2fv::format))
    return;

  range.copyFrom(vertices);

  setDrawingState(color, true);

  pool.getContext().render(GL::PrimitiveRange(GL::TRIANGLE_LIST, range));
}

void Drawer::drawBezier(const BezierCurve2& spline, const vec4& color)
{
  BezierCurve2::PointList points;
  spline.tessellate(points);

  GL::VertexRange range;
  if (!pool.allocateVertices(range, points.size(), Vertex2fv::format))
    return;

  // Realize vertices
  {
    GL::VertexRangeLock<Vertex2fv> vertices(range);

    for (unsigned int i = 0;  i < points.size();  i++)
      vertices[i].position = points[i];
  }

  setDrawingState(color, true);

  pool.getContext().render(GL::PrimitiveRange(GL::LINE_STRIP, range));
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
  if (!pool.allocateVertices(range, 4, Vertex2fv::format))
    return;

  range.copyFrom(vertices);

  setDrawingState(color, true);

  pool.getContext().render(GL::PrimitiveRange(GL::LINE_LOOP, range));
}

void Drawer::fillTriangle(const Triangle2& triangle, const vec4& color)
{
  Vertex2fv vertices[3];
  vertices[0].position = triangle.P[0];
  vertices[1].position = triangle.P[1];
  vertices[2].position = triangle.P[2];

  GL::VertexRange range;
  if (!pool.allocateVertices(range, 3, Vertex2fv::format))
    return;

  range.copyFrom(vertices);

  setDrawingState(color, false);

  pool.getContext().render(GL::PrimitiveRange(GL::TRIANGLE_LIST, range));
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
  if (!pool.allocateVertices(range, 4, Vertex2fv::format))
    return;

  range.copyFrom(vertices);

  setDrawingState(color, false);

  pool.getContext().render(GL::PrimitiveRange(GL::TRIANGLE_FAN, range));
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
  if (!pool.allocateVertices(range, 4, Vertex2ft2fv::format))
    return;

  range.copyFrom(vertices);

  if (texture.getFormat().getSemantic() == PixelFormat::RGBA)
    blitPass.setBlendFactors(GL::BLEND_SRC_ALPHA, GL::BLEND_ONE_MINUS_SRC_ALPHA);
  else
    blitPass.setBlendFactors(GL::BLEND_ONE, GL::BLEND_ZERO);

  blitPass.setSamplerState("image", &texture);
  blitPass.apply();

  pool.getContext().render(GL::PrimitiveRange(GL::TRIANGLE_FAN, range));

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
  drawText(area, text, Alignment(), state);
}

const Theme& Drawer::getTheme() const
{
  return *theme;
}

const render::Font& Drawer::getCurrentFont()
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

render::GeometryPool& Drawer::getGeometryPool() const
{
  return pool;
}

Drawer* Drawer::create(render::GeometryPool& pool)
{
  Ptr<Drawer> drawer(new Drawer(pool));
  if (!drawer->init())
    return NULL;

  return drawer.detachObject();
}

Drawer::Drawer(render::GeometryPool& initPool):
  pool(initPool)
{
}

bool Drawer::init()
{
  GL::Context& context = pool.getContext();

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
    Path path("wendy/UIDefault.theme");

    theme = Theme::read(pool, path);
    if (!theme)
    {
      logError("Failed to load default UI theme \'%s\'",
               path.asString().c_str());
      return false;
    }

    currentFont = theme->font;
  }

  // Set up solid pass
  {
    Path path("wendy/UIElement.program");

    Ref<GL::Program> program = GL::Program::read(context, path);
    if (!program)
    {
      logError("Failed to load UI element program \'%s\'",
               path.asString().c_str());
      return false;
    }

    GL::ProgramInterface interface;
    interface.addUniform("elementPos", GL::Uniform::VEC2);
    interface.addUniform("elementSize", GL::Uniform::VEC2);
    interface.addUniform("texPos", GL::Uniform::VEC2);
    interface.addUniform("texSize", GL::Uniform::VEC2);
    interface.addSampler("image", GL::Sampler::SAMPLER_RECT);
    interface.addAttribute("sizeScale", GL::Attribute::VEC2);
    interface.addAttribute("offsetScale", GL::Attribute::VEC2);
    interface.addAttribute("texScale", GL::Attribute::VEC2);

    if (!interface.matches(*program, true))
    {
      logError("UI element program \'%s\' does not conform to the required interface",
               path.asString().c_str());
      return false;
    }

    elementPass.setProgram(program);
    elementPass.setDepthTesting(false);
    elementPass.setDepthWriting(false);
    elementPass.setSamplerState("image", theme->texture);
  }

  // Set up solid pass
  {
    Path path("wendy/UIDrawSolid.program");

    Ref<GL::Program> program = GL::Program::read(context, path);
    if (!program)
    {
      logError("Failed to load UI drawing shader program \'%s\'",
               path.asString().c_str());
      return false;
    }

    GL::ProgramInterface interface;
    interface.addUniform("color", GL::Uniform::VEC4);
    interface.addAttribute("wyPosition", GL::Attribute::VEC2);

    if (!interface.matches(*program, true))
    {
      logError("UI drawing shader program \'%s\' does not conform to the required interface",
               path.asString().c_str());
      return false;
    }

    drawPass.setProgram(program);
    drawPass.setCullMode(GL::CULL_NONE);
    drawPass.setDepthTesting(false);
    drawPass.setDepthWriting(false);
  }

  // Set up blitting pass
  {
    Path path("wendy/UIDrawMapped.program");

    Ref<GL::Program> program = GL::Program::read(context, path);
    if (!program)
    {
      logError("Failed to load UI blitting shader program \'%s\'",
               path.asString().c_str());
      return false;
    }

    GL::ProgramInterface interface;
    interface.addSampler("image", GL::Sampler::SAMPLER_2D);
    interface.addAttribute("wyPosition", GL::Attribute::VEC2);
    interface.addAttribute("wyTexCoord", GL::Attribute::VEC2);

    if (!interface.matches(*program, true))
    {
      logError("UI blitting shader program \'%s\' does not conform to the required interface",
               path.asString().c_str());
      return false;
    }

    blitPass.setProgram(program);
    blitPass.setCullMode(GL::CULL_NONE);
    blitPass.setDepthTesting(false);
    blitPass.setDepthWriting(false);
  }

  return true;
}

void Drawer::drawElement(const Rect& area, const Rect& mapping)
{
  elementPass.setUniformState("elementPos", area.position);
  elementPass.setUniformState("elementSize", area.size);
  elementPass.setUniformState("texPos", mapping.position);
  elementPass.setUniformState("texSize", mapping.size);
  elementPass.apply();

  pool.getContext().render(range);
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
