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

using namespace moira;

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
    case IndexBuffer::UINT:
      return GL_UNSIGNED_INT;
    case IndexBuffer::USHORT:
      return GL_UNSIGNED_SHORT;
    case IndexBuffer::UBYTE:
      return GL_UNSIGNED_BYTE;
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

Bimap<PixelFormat, GLenum> colorFormatMap;
Bimap<PixelFormat, GLenum> depthFormatMap;

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

void Renderer::render(void)
{
  if (currentRange.isEmpty())
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

  const VertexBuffer& vertexBuffer = *(currentRange.getVertexBuffer());
  vertexBuffer.apply();

  const IndexBuffer* indexBuffer = currentRange.getIndexBuffer();
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

  if (Uniform* M = program.findUniform("M"))
  {
    if (M->getType() == Uniform::FLOAT_MAT4)
      M->setValue(modelMatrix);
  }

  if (Uniform* V = program.findUniform("V"))
  {
    if (V->getType() == Uniform::FLOAT_MAT4)
      V->setValue(viewMatrix);
  }

  if (Uniform* P = program.findUniform("P"))
  {
    if (P->getType() == Uniform::FLOAT_MAT4)
      P->setValue(projectionMatrix);
  }

  if (Uniform* MV = program.findUniform("MV"))
  {
    if (MV->getType() == Uniform::FLOAT_MAT4)
    {
      Mat4 mv = viewMatrix;
      mv *= modelMatrix;
      MV->setValue(mv);
    }
  }

  if (Uniform* VP = program.findUniform("VP"))
  {
    if (VP->getType() == Uniform::FLOAT_MAT4)
    {
      Mat4 vp = projectionMatrix;
      vp *= viewMatrix;
      VP->setValue(vp);
    }
  }

  if (Uniform* MVP = program.findUniform("MVP"))
  {
    if (MVP->getType() == Uniform::FLOAT_MAT4)
    {
      Mat4 mvp = projectionMatrix;
      mvp *= viewMatrix;
      mvp *= modelMatrix;
      MVP->setValue(mvp);
    }
  }

  if (indexBuffer)
  {
    glDrawElements(convertPrimitiveType(currentRange.getType()),
                   currentRange.getCount(),
		   convertType(indexBuffer->getType()),
		   (GLvoid*) (IndexBuffer::getTypeSize(indexBuffer->getType()) * currentRange.getStart()));
  }
  else
  {
    glDrawArrays(convertPrimitiveType(currentRange.getType()),
                 currentRange.getStart(),
		 currentRange.getCount());
  }

  if (stats)
    stats->addPrimitives(currentRange.getType(), currentRange.getCount());

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

    // Granularity of 1K
    // TODO: Make granularity configurable or autoconfigured
    const unsigned int actualCount = 1024 * ((count + 1023) / 1024);

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

    // Granularity of 1K
    // TODO: Make granularity configurable or autoconfigured
    const unsigned int actualCount = 1024 * ((count + 1023) / 1024);
    
    slot->vertexBuffer = VertexBuffer::createInstance(actualCount,
                                                      format,
						      VertexBuffer::DYNAMIC);
    if (!slot->vertexBuffer)
    {
      vertexBufferPool.pop_back();
      return false;
    }

    String specification;
    format.getSpecification(specification);

    Log::write("Allocated vertex pool of size %u format \'%s\'",
               actualCount,
	       specification.c_str());

    slot->available = slot->vertexBuffer->getCount();
  }

  range = VertexRange(*(slot->vertexBuffer), 
		      slot->vertexBuffer->getCount() - slot->available,
                      count);

  slot->available -= count;
  return true;
}

bool Renderer::isReservedUniform(const String& name) const
{
  return name == "M" || name == "V" || name == "P" ||
         name == "MV" || name == "VP" ||
	 name == "MVP";
}

Context& Renderer::getContext(void) const
{
  return context;
}

Texture& Renderer::getDefaultTexture(void) const
{
  return *defaultTexture;
}

Program& Renderer::getDefaultProgram(void) const
{
  return *defaultProgram;
}

Program* Renderer::getCurrentProgram(void) const
{
  return currentProgram;
}

const PrimitiveRange& Renderer::getCurrentPrimitiveRange(void) const
{
  return currentRange;
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
  projectionMatrix.setIdentity();

  projectionMatrix.x.x = 2.f / width;
  projectionMatrix.y.y = 2.f / height;
  projectionMatrix.z.z = -1.f;
  projectionMatrix.w.x = -1.f;
  projectionMatrix.w.y = -1.f;
  projectionMatrix.w.w = 1.f;
}

void Renderer::setProjectionMatrix3D(float FOV, float aspect, float nearZ, float farZ)
{
  if (aspect == 0.f)
  {
    aspect = (context.getCurrentCanvas().getWidth() * context.getViewportArea().size.x) /
             (context.getCurrentCanvas().getHeight() * context.getViewportArea().size.y);
  }

  const float f = 1.f / tanf((FOV * (float) M_PI / 180.f) / 2.f);

  projectionMatrix.setIdentity();

  projectionMatrix.x.x = f / aspect;
  projectionMatrix.y.y = f;
  projectionMatrix.z.z = (farZ + nearZ) / (nearZ - farZ);
  projectionMatrix.z.w = -1.f;
  projectionMatrix.w.z = (2.f * farZ * nearZ) / (nearZ - farZ);
  projectionMatrix.w.w = 0.f;
}

void Renderer::setCurrentProgram(Program* newProgram)
{
  currentProgram = newProgram;
}

void Renderer::setCurrentPrimitiveRange(const PrimitiveRange& newRange)
{
  currentRange = newRange;
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
  defaultTexture = Texture::findInstance("default");
  if (!defaultTexture)
  {
    // TODO: Create DXM-colored source image.

    defaultTexture = Texture::createInstance(context,
                                             moira::Image(PixelFormat::RGB8, 20, 20),
                                             Texture::DEFAULT,
                                             "default");
    if (!defaultTexture)
    {
      Log::writeError("Failed to create default texture");
      return false;
    }
  }

  // TODO: Create default shader program(s).

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
