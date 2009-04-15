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
#include <wendy/GLCanvas.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLShader.h>
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
  
size_t getTypeSize(IndexBuffer::Type type)
{
  switch (type)
  {
    case IndexBuffer::UINT:
      return sizeof(GLuint);
    case IndexBuffer::USHORT:
      return sizeof(GLushort);
    case IndexBuffer::UBYTE:
      return sizeof(GLubyte);
    default:
      throw Exception("Invalid index buffer type");
  }
}

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

}

///////////////////////////////////////////////////////////////////////

void Renderer::begin(const Matrix4& newProjection)
{
  Canvas* canvas = Canvas::getCurrent();
  if (!canvas)
  {
    Log::writeError("Cannot begin without a current canvas");
    return;
  }

  if (!modelview.isEmpty())
    throw Exception("Renderer modelview matrix stack not empty at begin");

  projection = newProjection;
}
  
void Renderer::begin2D(float width, float height)
{
  Canvas* canvas = Canvas::getCurrent();
  if (!canvas)
  {
    Log::writeError("Cannot begin without a current canvas");
    return;
  }

  if (!modelview.isEmpty())
    throw Exception("Renderer modelview matrix stack not empty at begin");

  projection.x.x = 2.f / width;
  projection.y.y = 2.f / height;
  projection.z.z = -1.f;
  projection.w.x = -1.f;
  projection.w.y = -1.f;
  projection.w.w = 1.f;
}

void Renderer::begin3D(float FOV, float aspect, float nearZ, float farZ)
{
  Canvas* canvas = Canvas::getCurrent();
  if (!canvas)
  {
    Log::writeError("Cannot begin without a current canvas");
    return;
  }

  if (!modelview.isEmpty())
    throw Exception("Renderer modelview matrix stack not empty at begin");

  if (aspect == 0.f)
    aspect = (float) canvas->getPhysicalWidth() / (float) canvas->getPhysicalHeight();

  const float f = 1.f / tanf((FOV * M_PI / 180.f) / 2.f);

  projection.x.x = f / aspect;
  projection.y.y = f;
  projection.z.z = (farZ + nearZ) / (nearZ - farZ);
  projection.z.w = -1.f;
  projection.w.z = (2.f * farZ * nearZ) / (nearZ - farZ);
  projection.w.w = 0.f;
}

void Renderer::end(void)
{
  if (!modelview.isEmpty())
    throw Exception("Renderer modelview matrix stack not empty after end");

  projection.setIdentity();
}

void Renderer::pushTransform(const Matrix4& transform)
{
  modelview.push(transform);
}

void Renderer::popTransform(void)
{
  modelview.pop();
}

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
    Log::writeError("Shader program has more varying parameters than vertex format has components");
    return;
  }

  for (unsigned int i = 0;  i < program.getVaryingCount();  i++)
  {
    Varying& varying = program.getVarying(i);

    const VertexComponent* component = format.findComponent(varying.getName());
    if (!component)
    {
      Log::writeError("Varying parameter \'%s\' has no corresponding vertex format component",
                      varying.getName().c_str());
      return;
    }

    // TODO: Check type compatibility.

    varying.enable(format.getSize(), component->getOffset());
  }

  if (Uniform* MVP = program.findUniform("MVP"))
  {
    if (MVP->getType() == Uniform::FLOAT_MAT4)
    {
      Matrix4 mvp = projection;
      mvp.concatenate(modelview.getTotal());
      MVP->setValue(mvp);
    }
  }

  if (Uniform* P = program.findUniform("P"))
  {
    if (P->getType() == Uniform::FLOAT_MAT4)
      P->setValue(projection);
  }

  if (Uniform* MV = program.findUniform("MV"))
  {
    if (MV->getType() == Uniform::FLOAT_MAT4)
      MV->setValue(modelview.getTotal());
  }

  if (indexBuffer)
  {
    glDrawElements(convertPrimitiveType(currentRange.getType()),
                   currentRange.getCount(),
		   convertType(indexBuffer->getType()),
		   (GLvoid*) (getTypeSize(indexBuffer->getType()) * currentRange.getStart()));
  }
  else
  {
    glDrawArrays(convertPrimitiveType(currentRange.getType()),
                 currentRange.getStart(),
		 currentRange.getCount());
  }

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
    // TODO: Make configurable or autoconfigured
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
    // TODO: Make configurable or autoconfigured
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

    Log::write("Allocated vertex pool of size %u format %s",
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
  return name == "MVP" || name == "MV" || name == "P";
}

Context& Renderer::getContext(void) const
{
  return context;
}

Canvas* Renderer::getCurrentCanvas(void) const
{
  return currentCanvas;
}

Texture& Renderer::getDefaultTexture(void) const
{
  return *defaultTexture;
}

Program& Renderer::getDefaultProgram(void) const
{
  return *defaultProgram;
}

void Renderer::setCurrentCanvas(Canvas* newCanvas)
{
  currentCanvas = newCanvas;
}

void Renderer::setCurrentProgram(Program* newProgram)
{
  currentProgram = newProgram;
}

void Renderer::setCurrentPrimitiveRange(const PrimitiveRange& newRange)
{
  currentRange = newRange;
}

bool Renderer::create(Context& context)
{
  Ptr<Renderer> renderer = new Renderer(context);
  if (!renderer->init())
    return false;

  set(renderer.detachObject());
  return true;
}

Renderer::Renderer(Context& initContext):
  context(initContext),
  currentCanvas(NULL),
  currentProgram(NULL)
{
}

bool Renderer::init(void)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create renderer without OpenGL context");
    return false;
  }

  CheckerImageGenerator generator;
  generator.setDefaultColor(ColorRGBA(1.f, 0.f, 1.f, 1.f));
  generator.setCheckerColor(ColorRGBA(0.f, 1.f, 0.f, 1.f));
  generator.setCheckerSize(10);

  Ptr<Image> image = generator.generate(ImageFormat::RGB888, 20, 20);
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
  for (IndexBufferList::iterator i = indexBufferPool.begin();  i != indexBufferPool.end();  i++)
    (*i).available = (*i).indexBuffer->getCount();

  for (VertexBufferList::iterator i = vertexBufferPool.begin();  i != vertexBufferPool.end();  i++)
    (*i).available = (*i).vertexBuffer->getCount();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
