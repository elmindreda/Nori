///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/GLLight.h>
#include <wendy/GLShader.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <algorithm>
#include <cstdlib>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

void Renderer::begin2D(const Vector2& resolution) const
{
  Canvas* canvas = Canvas::getCurrent();
  if (!canvas)
  {
    Log::writeError("Cannot begin without a current canvas");
    return;
  }

  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0.f, resolution.x, 0.f, resolution.y);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glPopAttrib();
}

void Renderer::begin3D(float FOV, float aspect, float nearZ, float farZ) const
{
  Canvas* canvas = Canvas::getCurrent();
  if (!canvas)
  {
    Log::writeError("Cannot begin without a current canvas");
    return;
  }

  if (aspect == 0.f)
    aspect = (float) canvas->getPhysicalWidth() / (float) canvas->getPhysicalHeight();

  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPerspective(FOV, aspect, nearZ, farZ);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glPopAttrib();
}

void Renderer::begin3D(const Matrix4& projection) const
{
  Canvas* canvas = Canvas::getCurrent();
  if (!canvas)
  {
    Log::writeError("Cannot begin without a current canvas");
    return;
  }

  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMultMatrixf(projection);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glPopAttrib();
}
  
void Renderer::end(void) const
{
  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();
}

void Renderer::pushTransform(const Matrix4& transform) const
{
  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glMultMatrixf(transform);
  glPopAttrib();
}

void Renderer::popTransform(void) const
{
  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();
}

void Renderer::drawPoint(const Vector2& point)
{
  drawPass.apply();

  GL::Vertex2fv vertex;
  vertex.position = point;

  GL::VertexRange range;
  if (!allocateVertices(range, 1, GL::Vertex2fv::format))
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
  if (!allocateVertices(range, 4, GL::Vertex2fv::format))
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
  if (!allocateVertices(range, 4, GL::Vertex2ft2fv::format))
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

bool Renderer::allocateIndices(IndexRange& range,
		               unsigned int count,
                               IndexBuffer::Type type)
{
  if (!count)
  {
    range = IndexRange();
    return true;
  }

  IndexBufferSlot* slot = NULL;

  for (IndexBufferList::iterator i = indexBuffers.begin();  i != indexBuffers.end();  i++)
  {
    if ((*i).indexBuffer->getType() == type && (*i).available >= count)
    {
      slot = &(*i);
      break;
    }
  }

  if (!slot)
  {
    indexBuffers.push_back(IndexBufferSlot());
    slot = &(indexBuffers.back());

    // Granularity of 1K
    const unsigned int actualCount = 1024 * ((count + 1023) / 1024);

    slot->indexBuffer = IndexBuffer::createInstance(actualCount,
                                                    type,
						    IndexBuffer::DYNAMIC);
    if (!slot->indexBuffer)
    {
      indexBuffers.pop_back();
      return false;
    }

    slot->available = slot->indexBuffer->getCount();
  }

  range = IndexRange(*(slot->indexBuffer), 
		     slot->indexBuffer->getCount() - slot->available,
                     count);

  slot->available -= count;
  return true;
}

bool Renderer::allocateVertices(VertexRange& range,
				unsigned int count,
				const VertexFormat& format)
{
  if (!count)
  {
    range = VertexRange();
    return true;
  }

  VertexBufferSlot* slot = NULL;

  for (VertexBufferList::iterator i = vertexBuffers.begin();  i != vertexBuffers.end();  i++)
  {
    if ((*i).vertexBuffer->getFormat() == format && (*i).available >= count)
    {
      slot = &(*i);
      break;
    }
  }

  if (!slot)
  {
    vertexBuffers.push_back(VertexBufferSlot());
    slot = &(vertexBuffers.back());

    // Granularity of 1K
    const unsigned int actualCount = 1024 * ((count + 1023) / 1024);
    
    slot->vertexBuffer = VertexBuffer::createInstance(actualCount,
                                                      format,
						      VertexBuffer::DYNAMIC);
    if (!slot->vertexBuffer)
    {
      vertexBuffers.pop_back();
      return false;
    }

    slot->available = slot->vertexBuffer->getCount();
  }

  range = VertexRange(*(slot->vertexBuffer), 
		      slot->vertexBuffer->getCount() - slot->available,
                      count);

  slot->available -= count;
  return true;
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
    Context::getDestroySignal().connect(onContextDestroy);
    initialized = true;
  }
}

bool Renderer::init(void)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create renderer without OpenGL context");
    return false;
  }

  drawPass.setCullMode(CULL_NONE);
  drawPass.setDepthTesting(false);
  drawPass.setDepthWriting(false);
  drawPass.setDefaultColor(ColorRGBA::BLACK);

  CheckerImageGenerator generator;
  generator.setDefaultColor(ColorRGBA(1.f, 0.f, 1.f, 1.f));
  generator.setCheckerColor(ColorRGBA(0.f, 1.f, 0.f, 1.f));
  generator.setCheckerSize(1);

  Ptr<Image> image = generator.generate(ImageFormat::RGBX8888, 2, 2);
  if (!image)
  {
    Log::writeError("Failed to create image data for default texture");
    return false;
  }

  defaultTexture = Texture::createInstance(*image, Texture::DEFAULT, "default");
  if (!defaultTexture)
  {
    Log::writeError("Failed to create default texture");
    return false;
  }

  Context::get()->getFinishSignal().connect(*this, &Renderer::onContextFinish);
  return true;
}

void Renderer::onContextFinish(void)
{
  for (IndexBufferList::iterator i = indexBuffers.begin();  i != indexBuffers.end();  i++)
    (*i).available = (*i).indexBuffer->getCount();

  for (VertexBufferList::iterator i = vertexBuffers.begin();  i != vertexBuffers.end();  i++)
    (*i).available = (*i).vertexBuffer->getCount();
}

void Renderer::onContextDestroy(void)
{
  if (get())
  {
    Log::writeWarning("Renderer not explicitly destroyed before context destruction");
    destroy();
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
