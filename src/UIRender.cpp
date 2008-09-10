//////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLLight.h>
#include <wendy/GLShader.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderFont.h>

#include <wendy/UIRender.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

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

bool Renderer::pushClipArea(const Rectangle& area)
{
  GL::Canvas* canvas = GL::Canvas::getCurrent();
  if (!canvas)
    throw Exception("Cannot render without a current canvas");

  Vector2 scale;
  scale.x = 1.f / canvas->getPhysicalWidth();
  scale.y = 1.f / canvas->getPhysicalHeight();

  return canvas->pushScissorArea(area * scale);
}

void Renderer::popClipArea(void)
{
  GL::Canvas* canvas = GL::Canvas::getCurrent();
  if (!canvas)
    throw Exception("Cannot render without a current canvas");

  canvas->popScissorArea();
}

void Renderer::drawPoint(const Vector2& point)
{
  drawPass.apply();

  GL::Vertex2fv vertex;
  vertex.position = point;

  GL::VertexRange range;
  if (!GL::Renderer::get()->allocateVertices(range, 1, GL::Vertex2fv::format))
    return;

  range.copyFrom(&vertex);
  range.render(GL_POINTS);
}

void Renderer::drawLine(const Segment2& segment)
{
  drawPass.apply();

  glBegin(GL_LINES);
  glVertex2fv(segment.start);
  glVertex2fv(segment.end);
  glEnd();
}

void Renderer::drawTriangle(const Triangle2& triangle)
{
  drawPass.setPolygonMode(GL_LINE);
  drawPass.apply();

  glBegin(GL_TRIANGLES);
  glVertex2fv(triangle.P[0]);
  glVertex2fv(triangle.P[1]);
  glVertex2fv(triangle.P[2]);
  glEnd();
}

void Renderer::drawBezier(const BezierCurve2& spline)
{
  BezierCurve2::PointList points;
  spline.tessellate(points);
  
  drawPass.apply();

  glBegin(GL_LINE_STRIP);
  for (unsigned int i = 0;  i < points.size();  i++)
    glVertex2fv(points[i]);
  glEnd();
}

void Renderer::drawRectangle(const Rectangle& rectangle)
{
  float minX, minY, maxX, maxY;
  rectangle.getBounds(minX, minY, maxX, maxY);

  if (maxX - minX < 1.f || maxY - minY < 1.f)
    return;

  drawPass.setPolygonMode(GL_LINE);
  drawPass.apply();

  glRectf(minX, minY, maxX - 1.f, maxY - 1.f);
}

void Renderer::fillTriangle(const Triangle2& triangle)
{
  drawPass.setPolygonMode(GL_FILL);
  drawPass.apply();

  glBegin(GL_TRIANGLES);
  glVertex2fv(triangle.P[0]);
  glVertex2fv(triangle.P[1]);
  glVertex2fv(triangle.P[2]);
  glEnd();
}

void Renderer::fillRectangle(const Rectangle& rectangle)
{
  float minX, minY, maxX, maxY;
  rectangle.getBounds(minX, minY, maxX, maxY);

  if (maxX - minX < 1.f || maxY - minY < 1.f)
    return;

  maxX -= 1.f;
  maxY -= 1.f;

  GL::Vertex2fv vertices[4];

  vertices[0].position.set(minX, minY);
  vertices[1].position.set(maxX, minY);
  vertices[2].position.set(maxX, maxY);
  vertices[3].position.set(minX, maxY);

  GL::VertexRange range;
  if (!GL::Renderer::get()->allocateVertices(range, 4, GL::Vertex2fv::format))
    return;

  drawPass.setPolygonMode(GL_FILL);
  drawPass.apply();

  range.copyFrom(vertices);
  range.render(GL_TRIANGLE_FAN);
}

void Renderer::blitTexture(const Rectangle& area, GL::Texture& texture)
{
  float minX, minY, maxX, maxY;
  area.getBounds(minX, minY, maxX, maxY);

  if (maxX - minX < 1.f || maxY - minY < 1.f)
    return;

  maxX -= 1.f;
  maxY -= 1.f;

  GL::Vertex2ft2fv vertices[4];

  vertices[0].mapping.set(0.f, 0.f);
  vertices[0].position.set(minX, minY);
  vertices[1].mapping.set(1.f, 0.f);
  vertices[1].position.set(maxX, minY);
  vertices[2].mapping.set(1.f, 1.f);
  vertices[2].position.set(maxX, maxY);
  vertices[3].mapping.set(0.f, 1.f);
  vertices[3].position.set(minX, maxY);

  GL::VertexRange range;
  if (!GL::Renderer::get()->allocateVertices(range, 4, GL::Vertex2ft2fv::format))
    return;

  GL::TextureLayer& layer = drawPass.createTextureLayer();
  layer.setCombineMode(GL_REPLACE);
  layer.setFilters(GL_NEAREST, GL_NEAREST);
  layer.setTexture(&texture);

  drawPass.setPolygonMode(GL_FILL);
  drawPass.apply();

  range.copyFrom(vertices);
  range.render(GL_TRIANGLE_FAN);

  drawPass.destroyTextureLayers();
}

void Renderer::drawText(const Rectangle& area,
                        const String& text,
		        const Alignment& alignment,
		        const ColorRGB& color)
{
  Rectangle metrics = currentFont->getTextMetrics(text);

  Vector2 penPosition(0.f, 0.f);

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

  currentFont->setColor(ColorRGBA(color, 1.f));
  currentFont->setPenPosition(penPosition);
  currentFont->drawText(text);
}

void Renderer::drawText(const Rectangle& area,
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

void Renderer::drawWell(const Rectangle& area, WidgetState state)
{
  float minX, minY, maxX, maxY;
  area.getBounds(minX, minY, maxX, maxY);

  if (maxX - minX < 1.f || maxY - minY < 1.f)
    return;

  GL::Pass pass;
  pass.setLineWidth(1.f / GL::Canvas::getCurrent()->getPhysicalHeight());

  switch (state)
  {
    case STATE_ACTIVE:
      pass.setDefaultColor(ColorRGBA(wellColor * 1.2f, 1.f));
      break;
    case STATE_DISABLED:
      pass.setDefaultColor(ColorRGBA(wellColor * 0.8f, 1.f));
      break;
    default:
      pass.setDefaultColor(ColorRGBA(wellColor, 1.f));
      break;
  }
      
  pass.setDepthTesting(false);
  pass.setDepthWriting(false);
  pass.apply();

  glRectf(minX, minY, maxX - 1.f, maxY - 1.f);
      
  pass.setPolygonMode(GL_LINE);
  pass.setDefaultColor(ColorRGBA::BLACK);
  pass.apply();

  glRectf(minX, minY, maxX - 1.f, maxY - 1.f);
}

void Renderer::drawFrame(const Rectangle& area, WidgetState state)
{
  float minX, minY, maxX, maxY;
  area.getBounds(minX, minY, maxX, maxY);

  if (maxX - minX < 1.f || maxY - minY < 1.f)
    return;

  GL::Pass pass;
  pass.setLineWidth(1.f / GL::Canvas::getCurrent()->getPhysicalHeight());

  switch (state)
  {
    case STATE_ACTIVE:
      pass.setDefaultColor(ColorRGBA(widgetColor * 1.2f, 1.f));
      break;
    case STATE_DISABLED:
      pass.setDefaultColor(ColorRGBA(widgetColor * 0.8f, 1.f));
      break;
    default:
      pass.setDefaultColor(ColorRGBA(widgetColor, 1.f));
      break;
  }
      
  pass.setDepthTesting(false);
  pass.setDepthWriting(false);
  pass.apply();

  glRectf(minX, minY, maxX - 1.f, maxY - 1.f);
      
  pass.setPolygonMode(GL_LINE);
  pass.setDefaultColor(ColorRGBA::BLACK);
  pass.apply();

  glRectf(minX, minY, maxX - 1.f, maxY - 1.f);
}

void Renderer::drawHandle(const Rectangle& area, WidgetState state)
{
  drawFrame(area, state);
}

void Renderer::drawButton(const Rectangle& area, WidgetState state, const String& text)
{
  drawFrame(area, state);

  if (text.length())
    drawText(area, text);
}

const ColorRGBA& Renderer::getColor(void) const
{
  return drawPass.getDefaultColor();
}

void Renderer::setColor(const ColorRGBA& newColor)
{
  drawPass.setDefaultColor(newColor);
  if (newColor.a == 1.f)
    drawPass.setBlendFactors(GL_ONE, GL_ZERO);
  else
    drawPass.setBlendFactors(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

float Renderer::getLineWidth(void) const
{
  return drawPass.getLineWidth();
}

void Renderer::setLineWidth(float newWidth)
{
  drawPass.setLineWidth(newWidth);
}

const ColorRGB& Renderer::getWidgetColor(void)
{
  return widgetColor;
}

const ColorRGB& Renderer::getTextColor(void)
{
  return textColor;
}

const ColorRGB& Renderer::getWellColor(void)
{
  return wellColor;
}

const ColorRGB& Renderer::getSelectedTextColor(void)
{
  return selectedTextColor;
}

const ColorRGB& Renderer::getSelectionColor(void)
{
  return selectionColor;
}

render::Font* Renderer::getCurrentFont(void)
{
  return currentFont;
}

render::Font* Renderer::getDefaultFont(void)
{
  return defaultFont;
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

bool Renderer::create(void)
{
  Ptr<Renderer> renderer = new Renderer();
  if (!renderer->init())
    return false;

  set(renderer.detachObject());
  return true;
}

Renderer::Renderer(void)
{
  static bool initialized = false;

  if (!initialized)
  {
    GL::Context::getDestroySignal().connect(onContextDestroy);
    initialized = true;
  }

  drawPass.setCullMode(GL::CULL_NONE);
  drawPass.setDepthTesting(false);
  drawPass.setDepthWriting(false);
  drawPass.setDefaultColor(ColorRGBA::BLACK);
}

bool Renderer::init(void)
{
  if (!GL::Context::get())
  {
    Log::writeError("Cannot create the widget renderer without an OpenGL context");
    return false;
  }

  defaultFont = render::Font::readInstance("default");
  if (!defaultFont)
  {
    Log::writeError("Failed to load default font");
    return false;
  }

  currentFont = defaultFont;

  widgetColor.set(0.7f, 0.7f, 0.7f);
  textColor = ColorRGB::BLACK;
  wellColor = widgetColor * 1.2f;
  selectionColor.set(0.3f, 0.3f, 0.3f);
  selectedTextColor = ColorRGB::WHITE;

  return true;
}

void Renderer::onContextDestroy(void)
{
  if (Renderer::get())
  {
    Log::writeWarning("Renderer not explicitly destroyed before context destruction");
    Renderer::destroy();
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
