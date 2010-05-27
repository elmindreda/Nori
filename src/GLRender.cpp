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

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLRender.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <algorithm>
#include <cstdlib>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

namespace
{

GLenum convertPrimitiveType(PrimitiveType type)
{
  switch (type)
  {
    case POINT_LIST:
      return GL_POINTS;
    case LINE_LIST:
      return GL_LINES;
    case LINE_STRIP:
      return GL_LINE_STRIP;
    case LINE_LOOP:
      return GL_LINE_LOOP;
    case TRIANGLE_LIST:
      return GL_TRIANGLES;
    case TRIANGLE_STRIP:
      return GL_TRIANGLE_STRIP;
    case TRIANGLE_FAN:
      return GL_TRIANGLE_FAN;
    default:
      throw Exception("Invalid primitive type");
  }
}

GLenum convertType(VertexComponent::Type type)
{
  switch (type)
  {
    case VertexComponent::DOUBLE:
      return GL_DOUBLE;
    case VertexComponent::FLOAT:
      return GL_FLOAT;
    case VertexComponent::INT:
      return GL_INT;
    default:
      throw Exception("Invalid vertex component type");
  }
}

GLenum convertType(IndexBuffer::Type type)
{
  switch (type)
  {
    case IndexBuffer::UINT8:
      return GL_UNSIGNED_BYTE;
    case IndexBuffer::UINT16:
      return GL_UNSIGNED_SHORT;
    case IndexBuffer::UINT32:
      return GL_UNSIGNED_INT;
    default:
      throw Exception("Invalid index buffer type");
  }
}

bool compatible(const Varying& varying, const VertexComponent& component)
{
  switch (varying.getType())
  {
    case Varying::FLOAT:
    {
      if (component.getType() == VertexComponent::FLOAT &&
          component.getElementCount() == 1)
        return true;

      break;
    }

    case Varying::FLOAT_VEC2:
    {
      if (component.getType() == VertexComponent::FLOAT &&
          component.getElementCount() == 2)
        return true;

      break;
    }

    case Varying::FLOAT_VEC3:
    {
      if (component.getType() == VertexComponent::FLOAT &&
          component.getElementCount() == 3)
        return true;

      break;
    }

    case Varying::FLOAT_VEC4:
    {
      if (component.getType() == VertexComponent::FLOAT &&
          component.getElementCount() == 4)
        return true;

      break;
    }
  }

  return false;
}

void requestModelMatrix(Uniform& uniform)
{
  Renderer* renderer = Renderer::get();
  uniform.setValue(renderer->getModelMatrix());
}

void requestViewMatrix(Uniform& uniform)
{
  Renderer* renderer = Renderer::get();
  uniform.setValue(renderer->getViewMatrix());
}

void requestProjectionMatrix(Uniform& uniform)
{
  Renderer* renderer = Renderer::get();
  uniform.setValue(renderer->getProjectionMatrix());
}

void requestModelViewMatrix(Uniform& uniform)
{
  Renderer* renderer = Renderer::get();
  Mat4 value = renderer->getViewMatrix();
  value *= renderer->getModelMatrix();
  uniform.setValue(value);
}

void requestViewProjectionMatrix(Uniform& uniform)
{
  Renderer* renderer = Renderer::get();
  Mat4 value = renderer->getProjectionMatrix();
  value *= renderer->getViewMatrix();
  uniform.setValue(value);
}

void requestModelViewProjectionMatrix(Uniform& uniform)
{
  Renderer* renderer = Renderer::get();
  Mat4 value = renderer->getProjectionMatrix();
  value *= renderer->getViewMatrix();
  value *= renderer->getModelMatrix();
  uniform.setValue(value);
}

} /*namespace (and Gandalf)*/

///////////////////////////////////////////////////////////////////////

Stats::Stats(void):
  frameCount(0),
  frameRate(0.f)
{
  frames.push_back(Frame());

  timer.start();
}

void Stats::addFrame(void)
{
  frameCount++;

  frames.push_front(Frame());
  if (frames.size() > 60)
    frames.pop_back();

  frameRate = 0.f;

  for (unsigned int i = 0;  i < frames.size();  i++)
    frameRate += (float) frames[i].duration / frames.size();
}

void Stats::addPasses(unsigned int count)
{
  Frame& frame = frames.front();
  frame.passCount += count;
}

void Stats::addPrimitives(PrimitiveType type, unsigned int count)
{
  if (!count)
    return;

  Frame& frame = frames.front();
  frame.vertexCount += count;

  switch (type)
  {
    case POINT_LIST:
      frame.pointCount += count;
      break;
    case LINE_LIST:
      frame.lineCount += count / 2;
      break;
    case LINE_STRIP:
      frame.lineCount += count - 1;
      break;
    case TRIANGLE_LIST:
      frame.triangleCount += count / 3;
      break;
    case TRIANGLE_STRIP:
      frame.triangleCount += count - 2;
      break;
    case TRIANGLE_FAN:
      frame.triangleCount += count - 1;
      break;
    default:
      Log::writeError("Invalid primitive type %u", type);
  }
}

float Stats::getFrameRate(void) const
{
  return frameRate;
}

unsigned int Stats::getFrameCount(void) const
{
  return frameCount;
}

const Stats::Frame& Stats::getFrame(void) const
{
  return frames.front();
}

///////////////////////////////////////////////////////////////////////

Stats::Frame::Frame(void):
  passCount(0),
  vertexCount(0),
  pointCount(0),
  lineCount(0),
  triangleCount(0),
  duration(0.0)
{
}

///////////////////////////////////////////////////////////////////////

void Renderer::render(const PrimitiveRange& range)
{
  if (range.isEmpty())
  {
    Log::writeWarning("Rendering empty primitive range");
    return;
  }

  if (!currentProgram)
  {
    Log::writeError("Unable to render without a current shader program");
    return;
  }

  // TODO: Optimize this method.

  Program& program = *currentProgram;
  program.apply();

  const VertexBuffer& vertexBuffer = *(range.getVertexBuffer());
  vertexBuffer.apply();

  const IndexBuffer* indexBuffer = range.getIndexBuffer();
  if (indexBuffer)
    indexBuffer->apply();

  const VertexFormat& format = vertexBuffer.getFormat();

  if (program.getVaryingCount() > format.getComponentCount())
  {
    Log::writeError("Shader program \'%s\' has more varying parameters than vertex format has components",
                    program.getName().c_str());
    return;
  }

  for (unsigned int i = 0;  i < program.getVaryingCount();  i++)
  {
    Varying& varying = program.getVarying(i);

    const VertexComponent* component = format.findComponent(varying.getName());
    if (!component)
    {
      Log::writeError("Varying parameter \'%s\' of shader program \'%s\' has no corresponding vertex format component",
                      varying.getName().c_str(),
                      program.getName().c_str());
      return;
    }

    if (!compatible(varying, *component))
    {
      Log::writeError("Varying parameter \'%s\' of shader program \'%s\' has incompatible type",
                      varying.getName().c_str(),
                      program.getName().c_str());
      return;
    }

    varying.enable(format.getSize(), component->getOffset());
  }

  for (UniformList::const_iterator u = reservedUniforms.begin();  u != reservedUniforms.end();  u++)
  {
    if (Uniform* uniform = program.findUniform(u->name))
    {
      if (uniform->getType() == u->type)
        u->signal.emit(*uniform);
    }
  }

  for (SamplerList::const_iterator s = reservedSamplers.begin();  s != reservedSamplers.end();  s++)
  {
    if (Sampler* sampler = program.findSampler(s->name))
    {
      if (sampler->getType() == s->type)
        s->signal.emit(*sampler);
    }
  }

  if (indexBuffer)
  {
    glDrawElements(convertPrimitiveType(range.getType()),
                   range.getCount(),
		   convertType(indexBuffer->getType()),
		   (GLvoid*) (IndexBuffer::getTypeSize(indexBuffer->getType()) * range.getStart()));
  }
  else
  {
    glDrawArrays(convertPrimitiveType(range.getType()),
                 range.getStart(),
		 range.getCount());
  }

  if (stats)
    stats->addPrimitives(range.getType(), range.getCount());

  for (unsigned int i = 0;  i < program.getVaryingCount();  i++)
    program.getVarying(i).disable();
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

  for (IndexBufferList::iterator i = indexBufferPool.begin();  i != indexBufferPool.end();  i++)
  {
    if ((*i).indexBuffer->getType() == type && (*i).available >= count)
    {
      slot = &(*i);
      break;
    }
  }

  if (!slot)
  {
    indexBufferPool.push_back(IndexBufferSlot());
    slot = &(indexBufferPool.back());

    // Granularity of 64K
    // TODO: Make granularity configurable or autoconfigured
    const unsigned int grainSize = 65536;
    const unsigned int actualCount = grainSize * ((count + grainSize - 1) / grainSize);

    slot->indexBuffer = IndexBuffer::createInstance(actualCount,
                                                    type,
						    IndexBuffer::DYNAMIC);
    if (!slot->indexBuffer)
    {
      indexBufferPool.pop_back();
      return false;
    }

    Log::write("Allocated index pool of size %u", actualCount);

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

  for (VertexBufferList::iterator i = vertexBufferPool.begin();  i != vertexBufferPool.end();  i++)
  {
    if ((*i).vertexBuffer->getFormat() == format && (*i).available >= count)
    {
      slot = &(*i);
      break;
    }
  }

  if (!slot)
  {
    vertexBufferPool.push_back(VertexBufferSlot());
    slot = &(vertexBufferPool.back());

    // Granularity of 64K
    // TODO: Make granularity configurable or autoconfigured
    const unsigned int grainSize = 65536;
    const unsigned int actualCount = grainSize * ((count + grainSize - 1) / grainSize);

    slot->vertexBuffer = VertexBuffer::createInstance(actualCount,
                                                      format,
						      VertexBuffer::DYNAMIC);
    if (!slot->vertexBuffer)
    {
      vertexBufferPool.pop_back();
      return false;
    }

    Log::write("Allocated vertex pool of size %u format \'%s\'",
               actualCount,
	       format.asString().c_str());

    slot->available = slot->vertexBuffer->getCount();
  }

  range = VertexRange(*(slot->vertexBuffer),
		      slot->vertexBuffer->getCount() - slot->available,
                      count);

  slot->available -= count;
  return true;
}

SignalProxy1<void, Uniform&> Renderer::reserveUniform(const String& name, Uniform::Type type)
{
  if (isReservedUniform(name))
    throw Exception("Uniform already reserved");

  reservedUniforms.push_back(ReservedUniform());

  ReservedUniform& slot = reservedUniforms.back();
  slot.name = name;
  slot.type = type;

  return slot.signal;
}

SignalProxy1<void, Sampler&> Renderer::reserveSampler(const String& name, Sampler::Type type)
{
  if (isReservedSampler(name))
    throw Exception("Sampler already reserved");

  reservedSamplers.push_back(ReservedSampler());

  ReservedSampler& slot = reservedSamplers.back();
  slot.name = name;
  slot.type = type;

  return slot.signal;
}

bool Renderer::isReservedUniform(const String& name) const
{
  for (UniformList::const_iterator u = reservedUniforms.begin();  u != reservedUniforms.end();  u++)
  {
    if (u->name == name)
      return true;
  }

  return false;
}

bool Renderer::isReservedSampler(const String& name) const
{
  for (SamplerList::const_iterator s = reservedSamplers.begin();  s != reservedSamplers.end();  s++)
  {
    if (s->name == name)
      return true;
  }

  return false;
}

Context& Renderer::getContext(void) const
{
  return context;
}

Program* Renderer::getCurrentProgram(void) const
{
  return currentProgram;
}

const Renderer::PlaneList& Renderer::getClipPlanes(void) const
{
  return planes;
}

const Mat4& Renderer::getModelMatrix(void) const
{
  return modelMatrix;
}

const Mat4& Renderer::getViewMatrix(void) const
{
  return viewMatrix;
}

const Mat4& Renderer::getProjectionMatrix(void) const
{
  return projectionMatrix;
}

void Renderer::setModelMatrix(const Mat4& newMatrix)
{
  modelMatrix = newMatrix;
}

void Renderer::setViewMatrix(const Mat4& newMatrix)
{
  viewMatrix = newMatrix;
}

void Renderer::setProjectionMatrix(const Mat4& newMatrix)
{
  projectionMatrix = newMatrix;
}

void Renderer::setProjectionMatrix2D(float width, float height)
{
  projectionMatrix.setProjection2D(width, height);
}

void Renderer::setProjectionMatrix3D(float FOV, float aspect, float nearZ, float farZ)
{
  if (aspect == 0.f)
  {
    aspect = (context.getCurrentCanvas().getWidth() * context.getViewportArea().size.x) /
             (context.getCurrentCanvas().getHeight() * context.getViewportArea().size.y);
  }

  projectionMatrix.setProjection3D(FOV, aspect, nearZ, farZ);
}

void Renderer::setCurrentProgram(Program* newProgram)
{
  currentProgram = newProgram;
}

bool Renderer::setClipPlanes(const PlaneList& newPlanes)
{
  if (planes.size() > context.getLimits().getMaxClipPlanes())
    return false;

  planes = newPlanes;
  unsigned int index = 0;

  for (PlaneList::const_iterator p = planes.begin();  p != planes.end();  p++)
  {
    // TODO: Verify this.

    const double equation[4] = { (*p).normal.x, (*p).normal.y, (*p).normal.z, (*p).distance };

    glEnable(GL_CLIP_PLANE0 + index);
    glClipPlane(GL_CLIP_PLANE0 + index, equation);

    index++;
  }

  for ( ;  index < context.getLimits().getMaxClipPlanes();  index++)
  {
    glDisable(GL_CLIP_PLANE0 + index);
  }

  return true;
}

Stats* Renderer::getStats(void) const
{
  return stats;
}

void Renderer::setStats(Stats* newStats)
{
  stats = newStats;
}

bool Renderer::create(Context& context)
{
  Ptr<Renderer> renderer(new Renderer(context));
  if (!renderer->init())
    return false;

  set(renderer.detachObject());
  return true;
}

Renderer::Renderer(Context& initContext):
  context(initContext),
  currentProgram(NULL),
  stats(NULL)
{
}

bool Renderer::init(void)
{
  reserveUniform("M", Uniform::FLOAT_MAT4).connect(requestModelMatrix);
  reserveUniform("V", Uniform::FLOAT_MAT4).connect(requestViewMatrix);
  reserveUniform("P", Uniform::FLOAT_MAT4).connect(requestProjectionMatrix);
  reserveUniform("MV", Uniform::FLOAT_MAT4).connect(requestModelViewMatrix);
  reserveUniform("VP", Uniform::FLOAT_MAT4).connect(requestViewProjectionMatrix);
  reserveUniform("MVP", Uniform::FLOAT_MAT4).connect(requestModelViewProjectionMatrix);

  context.getFinishSignal().connect(*this, &Renderer::onContextFinish);
  return true;
}

void Renderer::onContextFinish(void)
{
  for (IndexBufferList::iterator i = indexBufferPool.begin();  i != indexBufferPool.end();  i++)
    (*i).available = (*i).indexBuffer->getCount();

  for (VertexBufferList::iterator i = vertexBufferPool.begin();  i != vertexBufferPool.end();  i++)
    (*i).available = (*i).vertexBuffer->getCount();

  if (stats)
    stats->addFrame();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
