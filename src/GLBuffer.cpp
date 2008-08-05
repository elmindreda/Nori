///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/GLStatistics.h>
#include <wendy/GLVertex.h>
#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
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
      return 0;
  }
}

}

///////////////////////////////////////////////////////////////////////

VertexBuffer::~VertexBuffer(void)
{
  if (locked)
    Log::writeWarning("Vertex buffer destroyed while locked");

  if (current == this)
    invalidateCurrent();

  if (bufferID)
    glDeleteBuffersARB(1, &bufferID);
}

void VertexBuffer::apply(void) const
{
  if (current == this)
    return;

  const Byte* base = NULL;

  if (GLEW_ARB_vertex_buffer_object)
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferID);
  else
    base = data;

  const VertexComponent* component;

  component = format.findComponent(VertexComponent::VERTEX);
  if (component)
  {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(component->getElementCount(),
                    component->getType(),
		    (GLsizei) format.getSize(),
		    base + component->getOffset());
  }
  else
    glDisableClientState(GL_VERTEX_ARRAY);

  component = format.findComponent(VertexComponent::COLOR);
  if (component)
  {
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(component->getElementCount(),
                   component->getType(),
		   (GLsizei) format.getSize(),
		   base + component->getOffset());
  }
  else
    glDisableClientState(GL_COLOR_ARRAY);

  component = format.findComponent(VertexComponent::NORMAL);
  if (component)
  {
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(component->getType(),
		    (GLsizei) format.getSize(),
		    base + component->getOffset());
  }
  else
    glDisableClientState(GL_NORMAL_ARRAY);

  // Collect texture coordinate components

  std::vector<const VertexComponent*> components;

  for (unsigned int i = 0;  i < format.getComponentCount();  i++)
  {
    const VertexComponent& component = format[i];
    if (component.getKind() == VertexComponent::TEXCOORD)
      components.push_back(&component);
  }

  // Discard unusable texture components
  // TODO: Make this understand GLSL program limits.

  unsigned int textureUnitCount = TextureLayer::getUnitCount();
  if (components.size() > textureUnitCount)
  {
    Log::writeWarning("Applied vertex buffer contains more texture coordinate sets than there are texture units");
    components.resize(textureUnitCount);
  }

  // Apply texture components

  for (unsigned int i = 0;  i < components.size();  i++)
  {
    if (GLEW_ARB_multitexture)
      glClientActiveTextureARB(GL_TEXTURE0_ARB + i);

    component = components[i];

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(component->getElementCount(),
                      component->getType(),
		      (GLsizei) format.getSize(),
		      base + component->getOffset());
  }

  // Disable any remaining texture coordinate sets

  for (unsigned int i = components.size();  i < textureUnitCount;  i++)
  {
    if (GLEW_ARB_multitexture)
      glClientActiveTextureARB(GL_TEXTURE0_ARB + i);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }

  // TODO: Apply generic attribute components.

  current = const_cast<VertexBuffer*>(this);
}

void VertexBuffer::render(unsigned int mode,
                          unsigned int start,
                          unsigned int count) const
{
  if (getCurrent() != this)
    apply();

  if (!count)
    count = getCount();

  if (Statistics* statistics = Statistics::get())
    statistics->addPrimitives(mode, count);

  glDrawArrays(mode, start, count);
}

void* VertexBuffer::lock(void)
{
  if (locked)
  {
    Log::writeError("Vertex buffer already locked");
    return NULL;
  }

  void* mapping = NULL;

  if (GLEW_ARB_vertex_buffer_object)
  {
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferID);

    mapping = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);

    glPopClientAttrib();

    if (mapping == NULL)
    {
      Log::writeError("Unable to map vertex buffer object: %s", gluErrorString(glGetError()));
      return NULL;
    }
  }
  else
    mapping = data;

  locked = true;
  return mapping;
}

void VertexBuffer::unlock(void)
{
  if (!locked)
  {
    Log::writeWarning("Cannot unlock non-locked vertex buffer");
    return;
  }

  if (GLEW_ARB_vertex_buffer_object)
  {
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferID);

    if (!glUnmapBufferARB(GL_ARRAY_BUFFER_ARB))
      Log::writeWarning("Data for vertex buffer object was corrupted");

    glPopClientAttrib();
  }

  locked = false;
}

VertexBuffer::Usage VertexBuffer::getUsage(void) const
{
  return usage;
}

const VertexFormat& VertexBuffer::getFormat(void) const
{
  return format;
}

unsigned int VertexBuffer::getCount(void) const
{
  return count;
}

VertexBuffer* VertexBuffer::createInstance(unsigned int count,
                                           const VertexFormat& format,
					   Usage usage,
					   const String& name)
{
  Ptr<VertexBuffer> buffer = new VertexBuffer(name);
  if (!buffer->init(format, count, usage))
    return NULL;

  return buffer.detachObject();
}

void VertexBuffer::invalidateCurrent(void)
{
  current = NULL;
}

VertexBuffer* VertexBuffer::getCurrent(void)
{
  return current;
}

VertexBuffer::VertexBuffer(const String& name):
  Managed<VertexBuffer>(name),
  locked(false),
  count(0),
  usage(STATIC)
{
}

VertexBuffer::VertexBuffer(const VertexBuffer& source):
  Managed<VertexBuffer>(source)
{
  // NOTE: Not implemented.
}

VertexBuffer& VertexBuffer::operator = (const VertexBuffer& source)
{
  // NOTE: Not implemented.

  return *this;
}

bool VertexBuffer::init(const VertexFormat& initFormat,
			unsigned int initCount,
			Usage initUsage)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create vertex buffer without OpenGL context");
    return false;
  }

  if (GLEW_ARB_vertex_buffer_object)
  {
    // Clear any errors
    glGetError();

    glGenBuffersARB(1, &bufferID);

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferID);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB,
		    initCount * initFormat.getSize(),
		    NULL,
		    initUsage);
    glPopClientAttrib();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
      Log::writeWarning("Error during vertex buffer object creation: %s", gluErrorString(error));
      return false;
    }
  }
  else
    data.resize(initCount * initFormat.getSize());

  format = initFormat;
  usage = initUsage;
  count = initCount;

  return true;
}

VertexBuffer* VertexBuffer::current = NULL;

///////////////////////////////////////////////////////////////////////

VertexRange::VertexRange(void):
  vertexBuffer(NULL),
  start(0),
  count(0)
{
}

VertexRange::VertexRange(VertexBuffer& initVertexBuffer):
  vertexBuffer(&initVertexBuffer),
  start(0),
  count(0)
{
  if (vertexBuffer)
    count = vertexBuffer->getCount();
}

VertexRange::VertexRange(VertexBuffer& initVertexBuffer,
                         unsigned int initStart,
                         unsigned int initCount):
  vertexBuffer(&initVertexBuffer),
  start(initStart),
  count(initCount)
{
  if (vertexBuffer)
  {
    if (vertexBuffer->getCount() < start + count)
      throw Exception("Invalid vertex buffer range");
  }
  else
  {
    if (start > 0 || count > 0)
      throw Exception("Invalid vertex buffer range");
  }
}

void VertexRange::render(void) const
{
  if (!vertexBuffer || count == 0)
  {
    Log::writeError("Cannot render empty vertex buffer range");
    return;
  }

  vertexBuffer->render(start, count);
}

void* VertexRange::lock(void) const
{
  if (!vertexBuffer || count == 0)
  {
    Log::writeError("Cannot lock empty vertex buffer range");
    return NULL;
  }

  Byte* vertices = (Byte*) vertexBuffer->lock();
  if (!vertices)
    return NULL;

  return vertices + start * vertexBuffer->getFormat().getSize();
}

void VertexRange::unlock(void) const
{
  vertexBuffer->unlock();
}

VertexBuffer* VertexRange::getVertexBuffer(void) const
{
  return vertexBuffer;
}

unsigned int VertexRange::getStart(void) const
{
  return start;
}

unsigned int VertexRange::getCount(void) const
{
  return count;
}

///////////////////////////////////////////////////////////////////////

IndexBuffer::~IndexBuffer(void)
{
  if (locked)
    Log::writeWarning("Index buffer destroyed while locked");

  if (current == this)
    invalidateCurrent();

  if (bufferID)
    glDeleteBuffersARB(1, &bufferID);
}

void IndexBuffer::apply(void) const
{
  if (current == this)
    return;

  if (GLEW_ARB_vertex_buffer_object)
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferID);

  current = const_cast<IndexBuffer*>(this);
}

void IndexBuffer::render(const VertexBuffer& vertexBuffer,
                         unsigned int mode,
                         unsigned int start,
			 unsigned int count) const
{
  vertexBuffer.apply();

  if (getCurrent() != this)
    apply();

  if (!count)
    count = getCount();

  if (Statistics* statistics = Statistics::get())
    statistics->addPrimitives(mode, count);

  const Byte* base = NULL;

  if (!GLEW_ARB_vertex_buffer_object)
    base = data;

  glDrawElements(mode, count, type, base + getTypeSize(type) * start);
}

void* IndexBuffer::lock(void)
{
  if (locked)
  {
    Log::writeError("Index buffer already locked");
    return NULL;
  }

  void* mapping = NULL;

  if (GLEW_ARB_vertex_buffer_object)
  {
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferID);

    mapping = glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);

    glPopClientAttrib();

    if (mapping == NULL)
    {
      Log::writeError("Unable to map index buffer object: %s", gluErrorString(glGetError()));
      return NULL;
    }
  }
  else
    mapping = data;

  locked = true;
  return mapping;
}

void IndexBuffer::unlock(void)
{
  if (!locked)
  {
    Log::writeWarning("Cannot unlock non-locked index buffer");
    return;
  }

  if (GLEW_ARB_vertex_buffer_object)
  {
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferID);

    if (!glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB))
      Log::writeWarning("Data for index buffer object was corrupted");

    glPopClientAttrib();
  }

  locked = false;
}

IndexBuffer::Type IndexBuffer::getType(void) const
{
  return type;
}

IndexBuffer::Usage IndexBuffer::getUsage(void) const
{
  return usage;
}

unsigned int IndexBuffer::getCount(void) const
{
  return count;
}

IndexBuffer* IndexBuffer::createInstance(unsigned int count,
					 Type type,
					 Usage usage,
					 const String& name)
{
  Ptr<IndexBuffer> buffer = new IndexBuffer(name);
  if (!buffer->init(count, type, usage))
    return NULL;

  return buffer.detachObject();
}

void IndexBuffer::invalidateCurrent(void)
{
  current = NULL;
}

IndexBuffer* IndexBuffer::getCurrent(void)
{
  return current;
}

IndexBuffer::IndexBuffer(const String& name):
  Managed<IndexBuffer>(name),
  locked(false),
  type(UINT),
  usage(STATIC),
  count(0)
{
}

IndexBuffer::IndexBuffer(const IndexBuffer& source):
  Managed<IndexBuffer>(source)
{
  // NOTE: Not implemented.
}

IndexBuffer& IndexBuffer::operator = (const IndexBuffer& source)
{
  // NOTE: Not implemented.

  return *this;
}

bool IndexBuffer::init(unsigned int initCount, Type initType, Usage initUsage)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create index buffer without OpenGL context");
    return false;
  }

  if (GLEW_ARB_vertex_buffer_object)
  {
    // Clear any errors
    glGetError();

    glGenBuffersARB(1, &bufferID);

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferID);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
	            initCount * getTypeSize(initType),
		    NULL,
		    initUsage);
    glPopClientAttrib();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
      Log::writeWarning("Error during vertex buffer object creation: %s", gluErrorString(error));
      return false;
    }
  }
  else
    data.resize(initCount * getTypeSize(initType));

  type = initType;
  usage = initUsage;
  count = initCount;
  return true;
}

IndexBuffer* IndexBuffer::current = NULL;

///////////////////////////////////////////////////////////////////////

IndexRange::IndexRange(void):
  indexBuffer(NULL),
  start(0),
  count(0)
{
}

IndexRange::IndexRange(IndexBuffer& initIndexBuffer):
  indexBuffer(&initIndexBuffer),
  start(0),
  count(0)
{
  if (indexBuffer)
    count = indexBuffer->getCount();
}

IndexRange::IndexRange(IndexBuffer& initIndexBuffer,
                       unsigned int initStart,
                       unsigned int initCount):
  indexBuffer(&initIndexBuffer),
  start(initStart),
  count(initCount)
{
  if (indexBuffer)
  {
    if (indexBuffer->getCount() < start + count)
      throw Exception("Invalid index buffer range");
  }
  else
  {
    if (start > 0 || count > 0)
      throw Exception("Invalid index buffer range");
  }
}

void IndexRange::render(const VertexBuffer& vertexBuffer) const
{
  if (!indexBuffer || count == 0)
  {
    Log::writeError("Cannot render empty index buffer range");
    return;
  }

  indexBuffer->render(vertexBuffer, start, count);
}

void* IndexRange::lock(void) const
{
  if (!indexBuffer || count == 0)
  {
    Log::writeError("Cannot lock empty index buffer range");
    return NULL;
  }

  Byte* indices = (Byte*) indexBuffer->lock();
  if (!indices)
    return NULL;

  return indices + start * getTypeSize(indexBuffer->getType());
}

void IndexRange::unlock(void) const
{
  indexBuffer->unlock();
}

IndexBuffer* IndexRange::getIndexBuffer(void) const
{
  return indexBuffer;
}

unsigned int IndexRange::getStart(void) const
{
  return start;
}

unsigned int IndexRange::getCount(void) const
{
  return count;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
