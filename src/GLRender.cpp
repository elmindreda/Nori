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

#include <moira/Config.h>
#include <moira/Portability.h>
#include <moira/Core.h>
#include <moira/Signal.h>
#include <moira/Color.h>
#include <moira/Vector.h>
#include <moira/Matrix.h>
#include <moira/Rectangle.h>
#include <moira/Bezier.h>
#include <moira/Stream.h>
#include <moira/XML.h>
#include <moira/Mesh.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLShader.h>
#include <wendy/GLLight.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLRender.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{

// OMGWTFBBQ!!1!

inline unsigned int max(unsigned int x, unsigned int y)
{
  if (x > y)
    return x;
  else
    return y;
}

}

///////////////////////////////////////////////////////////////////////

RenderOperation::RenderOperation(void):
  vertexBuffer(NULL),
  indexBuffer(NULL),
  shader(NULL),
  start(0),
  count(0)
{
}

bool RenderOperation::operator < (const RenderOperation& other) const
{
  return (*shader) < (*other.shader);
}

///////////////////////////////////////////////////////////////////////

RenderQueue::RenderQueue(void):
  sorted(true)
{
}

void RenderQueue::addLight(Light& light)
{
  lights.push_back(&light);
}

void RenderQueue::addOperation(RenderOperation& operation)
{
  operations.push_back(operation);
  sorted = false;
}

void RenderQueue::removeOperations(void)
{
  operations.clear();
}

void RenderQueue::renderOperations(void)
{
  sortOperations();

  for (LightList::const_iterator i = lights.begin();  i != lights.end();  i++)
    (*i)->setEnabled(true);

  for (OperationList::const_iterator i = operations.begin();  i != operations.end();  i++)
  {
    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf((*i).transform);
    glPopAttrib();

    (*i).vertexBuffer->apply();
    if ((*i).indexBuffer)
      (*i).indexBuffer->apply();

    for (unsigned int pass = 0;  pass < (*i).shader->getPassCount();  pass++)
    {
      (*i).shader->applyPass(pass);

      if ((*i).indexBuffer)
        (*i).indexBuffer->render((*i).renderMode, (*i).start, (*i).count);
      else
        (*i).vertexBuffer->render((*i).renderMode, (*i).start, (*i).count);
    }

    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
  }

  for (LightList::const_iterator i = lights.begin();  i != lights.end();  i++)
    (*i)->setEnabled(false);
}

const RenderQueue::LightList& RenderQueue::getLights(void) const
{
  return lights;
}

const RenderQueue::OperationList& RenderQueue::getOperations(void) const
{
  return operations;
}

void RenderQueue::sortOperations(void)
{
  if (!sorted)
  {
    std::sort(operations.begin(), operations.end());
    sorted = true;
  }
}

///////////////////////////////////////////////////////////////////////

bool Renderer::allocateIndices(IndexBufferRange& range,
		               unsigned int count,
                               IndexBuffer::Type type)
{
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

    slot->indexBuffer = IndexBuffer::createInstance(max(1024, count),
                                                    type,
						    IndexBuffer::DYNAMIC);
    if (!slot->indexBuffer)
    {
      indexBuffers.pop_back();
      return false;
    }

    slot->available = slot->indexBuffer->getCount();
  }

  range = IndexBufferRange(*(slot->indexBuffer), 
			   slot->indexBuffer->getCount() - slot->available,
                           count);

  slot->available -= count;
  return true;
}

bool Renderer::allocateVertices(VertexBufferRange& range,
				unsigned int count,
				const VertexFormat& format)
{
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

    slot->vertexBuffer = VertexBuffer::createInstance(max(1024, count),
                                                      format,
						      VertexBuffer::DYNAMIC);
    if (!slot->vertexBuffer)
    {
      vertexBuffers.pop_back();
      return false;
    }

    slot->available = slot->vertexBuffer->getCount();
  }

  range = VertexBufferRange(*(slot->vertexBuffer), 
			    slot->vertexBuffer->getCount() - slot->available,
                            count);

  slot->available -= count;
  return true;
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
}

bool Renderer::init(void)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create renderer without OpenGL context");
    return false;
  }

  Context::get()->getFinishSignal().connect(*this, &Renderer::onFinish);

  return true;
}

void Renderer::onFinish(void)
{
  for (IndexBufferList::iterator i = indexBuffers.begin();  i != indexBuffers.end();  i++)
    (*i).available = (*i).indexBuffer->getCount();

  for (VertexBufferList::iterator i = vertexBuffers.begin();  i != vertexBuffers.end();  i++)
    (*i).available = (*i).vertexBuffer->getCount();
}

///////////////////////////////////////////////////////////////////////

/*
void Renderer::begin(void)
{
  stack.push(Context());

  Context& context = getContext();
  context.strokePass.setPolygonMode(GL_LINE);
}

void Renderer::end(void)
{
  if (stack.empty())
    throw Exception("Renderer context stack is empty");

  stack.pop();
}

void Renderer::drawLine(const Vector2& start, const Vector2& end) const
{
  Context& context = getContext();

  if (context.stroking)
  {
    context.strokePass.apply();

    glBegin(GL_LINES);
    glVertex2fv(start);
    glVertex2fv(end);
    glEnd();
  }
}

void Renderer::drawCircle(const Vector2& center, float radius) const
{
}

void Renderer::drawBezier(const BezierCurve2& curve) const
{
  BezierCurve2::PointList points;
  curve.tesselate(points, 0.5f);

  if (context.stroking)
  {
    context.strokePass.apply();

    glBegin(GL_LINE_STRIP);
    for (BezierCurve2::PointList::const_iterator p = points.begin();  p != points.end();  p++)
      glVertex2fv(*p);
    glEnd();
  }
}

void Renderer::drawRectangle(const Rectangle& rectangle) const
{
  Context& context = getContext();
  
  if (context.filling)
  {
    context.fillPass.apply();
    glRectf(rectangle.position.x,
            rectangle.position.y,
	    rectangle.position.x + rectangle.size.x,
	    rectangle.position.y + rectangle.size.y);
  }

  if (context.stroking)
  {
    context.strokePass.apply();
    glRectf(rectangle.position.x,
            rectangle.position.y,
	    rectangle.position.x + rectangle.size.x,
	    rectangle.position.y + rectangle.size.y);
  }
}

bool Renderer::isStroking(void) const
{
  return getContext().stroking;
}

void Renderer::setStroking(bool newState)
{
  Context& context = getContext();
  context.stroking = newState;
}

bool Renderer::isFilling(void) const
{
  return getContext().filling;
}

void Renderer::setFilling(bool newState)
{
  Context& context = getContext();
  context.filling = newState;
}

const ColorRGBA& Renderer::getStrokeColor(void) const
{
  return getContext().strokePass.getDefaultColor();
}

void Renderer::setStrokeColor(const ColorRGBA& newColor)
{
  Context& context = getContext();
  context.strokePass.setDefaultColor(newColor);
}

const ColorRGBA& Renderer::getFillColor(void) const
{
  return getContext().fillPass.getDefaultColor();
}

void Renderer::setFillColor(const ColorRGBA& newColor)
{
  Context& context = getContext();
  context.fillPass.setDefaultColor(newColor);
}

float Renderer::getStrokeWidth(void) const
{
  return getContext().strokePass.getLineWidth() / getLineScale();
}

void Renderer::setStrokeWidth(float newWidth)
{
  Context& context = getContext();
  context.strokePass.setLineWidth(newWidth() * getLineScale());
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
}

bool Renderer::init(void)
{
  return true;
}

float Renderer::getLineScale(void) const
{
  Canvas* Canvas::getCurrent();

  return (float) canvas->getPhysicalHeight() / (float) canvas->getHeight();
}

ShaderPass& Renderer::getContext(void)
{
  if (stack.empty())
    throw Exception("Renderer context stack empty");

  return stack.top();
}

const ShaderPass& Renderer::getContext(void) const
{
  if (stack.empty())
    throw Exception("Renderer context stack empty");

  return stack.top();
}

///////////////////////////////////////////////////////////////////////

Renderer::Context::Context(void):
  stroking(true),
  filling(false)
{
}
*/

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
