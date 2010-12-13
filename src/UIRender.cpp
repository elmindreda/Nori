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

#include <wendy/UIRender.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

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

bool Renderer::pushClipArea(const Rect& area)
{
  GL::Context& context = pool.getContext();

  GL::Canvas& canvas = context.getCurrentCanvas();

  Vec2 scale;
  scale.x = 1.f / canvas.getWidth();
  scale.y = 1.f / canvas.getHeight();

  if (!clipAreaStack.push(area * scale))
    return false;

  context.setScissorArea(clipAreaStack.getTotal());
  return true;
}

void Renderer::popClipArea(void)
{
  if (clipAreaStack.getCount() == 1)
  {
    Log::writeError("Cannot pop empty clip area stack");
    return;
  }

  clipAreaStack.pop();

  GL::Context& context = pool.getContext();

  context.setScissorArea(clipAreaStack.getTotal());
}

void Renderer::drawPoint(const Vec2& point, const ColorRGBA& color)
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

void Renderer::drawLine(const Segment2& segment, const ColorRGBA& color)
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

void Renderer::drawTriangle(const Triangle2& triangle, const ColorRGBA& color)
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

void Renderer::drawBezier(const BezierCurve2& spline, const ColorRGBA& color)
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

void Renderer::drawRectangle(const Rect& rectangle, const ColorRGBA& color)
{
  float minX, minY, maxX, maxY;
  rectangle.getBounds(minX, minY, maxX, maxY);

  if (maxX - minX < 1.f || maxY - minY < 1.f)
    return;

  maxX -= 1.f;
  maxY -= 1.f;

  Vertex2fv vertices[4];
  vertices[0].position.set(minX, minY);
  vertices[1].position.set(maxX, minY);
  vertices[2].position.set(maxX, maxY);
  vertices[3].position.set(minX, maxY);

  GL::VertexRange range;
  if (!pool.allocateVertices(range, 4, Vertex2fv::format))
    return;

  range.copyFrom(vertices);

  setDrawingState(color, true);

  pool.getContext().render(GL::PrimitiveRange(GL::LINE_LOOP, range));
}

void Renderer::fillTriangle(const Triangle2& triangle, const ColorRGBA& color)
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

void Renderer::fillRectangle(const Rect& rectangle, const ColorRGBA& color)
{
  float minX, minY, maxX, maxY;
  rectangle.getBounds(minX, minY, maxX, maxY);

  if (maxX - minX < 1.f || maxY - minY < 1.f)
    return;

  maxX -= 1.f;
  maxY -= 1.f;

  Vertex2fv vertices[4];
  vertices[0].position.set(minX, minY);
  vertices[1].position.set(maxX, minY);
  vertices[2].position.set(maxX, maxY);
  vertices[3].position.set(minX, maxY);

  GL::VertexRange range;
  if (!pool.allocateVertices(range, 4, Vertex2fv::format))
    return;

  range.copyFrom(vertices);

  setDrawingState(color, false);

  pool.getContext().render(GL::PrimitiveRange(GL::TRIANGLE_FAN, range));
}

void Renderer::blitTexture(const Rect& area, GL::Texture& texture)
{
  float minX, minY, maxX, maxY;
  area.getBounds(minX, minY, maxX, maxY);

  if (maxX - minX < 1.f || maxY - minY < 1.f)
    return;

  maxX -= 1.f;
  maxY -= 1.f;

  Vertex2ft2fv vertices[4];
  vertices[0].mapping.set(0.f, 0.f);
  vertices[0].position.set(minX, minY);
  vertices[1].mapping.set(1.f, 0.f);
  vertices[1].position.set(maxX, minY);
  vertices[2].mapping.set(1.f, 1.f);
  vertices[2].position.set(maxX, maxY);
  vertices[3].mapping.set(0.f, 1.f);
  vertices[3].position.set(minX, maxY);

  GL::VertexRange range;
  if (!pool.allocateVertices(range, 4, Vertex2ft2fv::format))
    return;

  range.copyFrom(vertices);

  blitPass.getSamplerState("image").setTexture(&texture);
  blitPass.apply();

  pool.getContext().render(GL::PrimitiveRange(GL::TRIANGLE_FAN, range));

  blitPass.getSamplerState("image").setTexture(NULL);
}

void Renderer::drawText(const Rect& area,
                        const String& text,
		        const Alignment& alignment,
		        const ColorRGB& color)
{
  if (text.empty())
    return;

  Rect metrics = currentFont->getTextMetrics(text);

  Vec2 penPosition(0.f, 0.f);

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
      Log::writeError("Invalid horizontal alignment");
      return;
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
      Log::writeError("Invalid vertical alignment");
      return;
  }

  currentFont->drawText(penPosition, ColorRGBA(color, 1.f), text);
}

void Renderer::drawText(const Rect& area,
                        const String& text,
		        const Alignment& alignment,
			WidgetState state)
{
  switch (state)
  {
    case STATE_DISABLED:
      drawText(area, text, alignment, textColor);
      break;

    case STATE_NORMAL:
      drawText(area, text, alignment, textColor);
      break;

    case STATE_ACTIVE:
      drawText(area, text, alignment, textColor);
      break;

    case STATE_SELECTED:
      drawText(area, text, alignment, selectedTextColor);
      break;

    default:
      Log::writeError("Invalid widget state %u", state);
      break;
  }
}

void Renderer::drawWell(const Rect& area, WidgetState state)
{
  ColorRGB fillColor;

  switch (state)
  {
    case STATE_ACTIVE:
      fillColor = wellColor * 1.2f;
      break;
    case STATE_DISABLED:
      fillColor = wellColor * 0.8f;
      break;
    default:
      fillColor = wellColor;
      break;
  }

  fillRectangle(area, fillColor);
  drawRectangle(area, ColorRGBA::BLACK);
}

void Renderer::drawFrame(const Rect& area, WidgetState state)
{
  ColorRGB fillColor;

  switch (state)
  {
    case STATE_ACTIVE:
      fillColor = widgetColor * 1.2f;
      break;
    case STATE_DISABLED:
      fillColor = widgetColor * 0.8f;
      break;
    default:
      fillColor = widgetColor;
      break;
  }

  fillRectangle(area, fillColor);
  drawRectangle(area, ColorRGBA::BLACK);
}

void Renderer::drawHandle(const Rect& area, WidgetState state)
{
  drawFrame(area, state);
}

void Renderer::drawButton(const Rect& area, WidgetState state, const String& text)
{
  drawFrame(area, state);

  if (text.length())
    drawText(area, text);
}

const ColorRGB& Renderer::getWidgetColor(void)
{
  return widgetColor;
}

void Renderer::setWidgetColor(const ColorRGB& newColor)
{
  widgetColor = newColor;
}

const ColorRGB& Renderer::getTextColor(void)
{
  return textColor;
}

void Renderer::setTextColor(const ColorRGB& newColor)
{
  textColor = newColor;
}

const ColorRGB& Renderer::getWellColor(void)
{
  return wellColor;
}

void Renderer::setWellColor(const ColorRGB& newColor)
{
  wellColor = newColor;
}

const ColorRGB& Renderer::getSelectionColor(void)
{
  return selectionColor;
}

void Renderer::setSelectionColor(const ColorRGB& newColor)
{
  selectionColor = newColor;
}

const ColorRGB& Renderer::getSelectedTextColor(void)
{
  return selectedTextColor;
}

void Renderer::setSelectedTextColor(const ColorRGB& newColor)
{
  selectedTextColor = newColor;
}

render::Font& Renderer::getCurrentFont(void)
{
  return *currentFont;
}

render::Font& Renderer::getDefaultFont(void)
{
  return *defaultFont;
}

float Renderer::getDefaultEM(void) const
{
  return defaultFont->getHeight();
}

float Renderer::getCurrentEM(void) const
{
  return currentFont->getHeight();
}

void Renderer::setCurrentFont(render::Font* newFont)
{
  if (newFont)
    currentFont = newFont;
  else
    currentFont = defaultFont;
}

render::GeometryPool& Renderer::getGeometryPool(void) const
{
  return pool;
}

Renderer* Renderer::create(render::GeometryPool& pool)
{
  Ptr<Renderer> renderer(new Renderer(pool));
  if (!renderer->init())
    return NULL;

  return renderer.detachObject();
}

Renderer::Renderer(render::GeometryPool& initPool):
  pool(initPool)
{
}

bool Renderer::init(void)
{
  widgetColor.set(0.7f, 0.7f, 0.7f);
  textColor = ColorRGB::BLACK;
  wellColor = widgetColor * 1.2f;
  selectionColor.set(0.3f, 0.3f, 0.3f);
  selectedTextColor = ColorRGB::WHITE;

  clipAreaStack.push(Rect(0.f, 0.f, 1.f, 1.f));

  // Load default font
  {
    Path path("wendy/default.font");

    defaultFont = render::Font::read(pool, path);
    if (!defaultFont)
    {
      Log::writeError("Failed to load default UI font \'%s\'",
                      path.asString().c_str());
      return false;
    }

    currentFont = defaultFont;
  }

  // Set up drawing render pass
  {
    Path path("wendy/UIRenderSolid.program");

    Ref<GL::Program> program = GL::Program::read(pool.getContext(), path);
    if (!program)
    {
      Log::writeError("Failed to load UI drawing shader program \'%s\'",
                      path.asString().c_str());
      return false;
    }

    GL::ProgramInterface interface;
    interface.addUniform("color", GL::Uniform::FLOAT_VEC4);
    interface.addVarying("position", GL::Varying::FLOAT_VEC2);

    if (!interface.matches(*program, true))
    {
      Log::writeError("UI drawing shader program \'%s\' does not conform to the required interface",
                      path.asString().c_str());
      return false;
    }

    drawPass.setProgram(program);
    drawPass.setCullMode(GL::CULL_NONE);
    drawPass.setDepthTesting(false);
    drawPass.setDepthWriting(false);
  }

  // Set up blitting render pass
  {
    Path path("wendy/UIRenderMapped.program");

    Ref<GL::Program> program = GL::Program::read(pool.getContext(), path);
    if (!program)
    {
      Log::writeError("Failed to load UI blitting shader program \'%s\'",
                      path.asString().c_str());
      return false;
    }

    GL::ProgramInterface interface;
    interface.addSampler("image", GL::Sampler::SAMPLER_2D);
    interface.addVarying("position", GL::Varying::FLOAT_VEC2);
    interface.addVarying("mapping", GL::Varying::FLOAT_VEC2);

    if (!interface.matches(*program, true))
    {
      Log::writeError("UI blitting shader program \'%s\' does not conform to the required interface",
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

void Renderer::setDrawingState(const ColorRGBA& color, bool wireframe)
{
  drawPass.getUniformState("color").setValue(Vec4(color.r, color.g, color.b, color.a));

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
