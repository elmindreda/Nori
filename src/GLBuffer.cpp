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
      Log::writeError("Invalid index buffer type %u", type);
      return 0;
  }
}

GLenum convertLockType(LockType type)
{
  switch (type)
  {
    case LOCK_READ_ONLY:
      return GL_READ_ONLY_ARB;
    case LOCK_WRITE_ONLY:
      return GL_WRITE_ONLY_ARB;
    case LOCK_READ_WRITE:
      return GL_READ_WRITE_ARB;
    default:
      Log::writeError("Invalid lock type %u", type);
      return 0;
  }
}

GLenum convertUsage(VertexBuffer::Usage usage)
{
  switch (usage)
  {
    case VertexBuffer::STATIC:
      return GL_STATIC_DRAW_ARB;
    case VertexBuffer::STREAM:
      return GL_STREAM_DRAW_ARB;
    case VertexBuffer::DYNAMIC:
      return GL_DYNAMIC_DRAW_ARB;
    default:
      Log::writeError("Invalid vertex buffer usage %u", usage);
      return 0;
  }
}

GLenum convertUsage(IndexBuffer::Usage usage)
{
  switch (usage)
  {
    case IndexBuffer::STATIC:
      return GL_STATIC_DRAW_ARB;
    case IndexBuffer::STREAM:
      return GL_STREAM_DRAW_ARB;
    case IndexBuffer::DYNAMIC:
      return GL_DYNAMIC_DRAW_ARB;
    default:
      Log::writeError("Invalid index buffer usage %u", usage);
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

  if (bufferID)
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferID);
  else
    base = data;

  const VertexComponent* component;

  component = format.findComponent(VertexComponent::POSITION);
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

  unsigned int textureCoords = Context::get()->getLimits().getMaxTextureCoords();
  if (components.size() > textureCoords)
  {
    Log::writeWarning("Applied vertex buffer contains more texture coordinate sets than this context supports");
    components.resize(textureCoords);
  }

  // Apply texture components

  for (unsigned int i = 0;  i < components.size();  i++)
  {
    glClientActiveTextureARB(GL_TEXTURE0_ARB + i);

    component = components[i];

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(component->getElementCount(),
                      component->getType(),
		      (GLsizei) format.getSize(),
		      base + component->getOffset());
  }

  // Disable any remaining texture coordinate sets

  for (unsigned int i = components.size();  i < textureCoords;  i++)
  {
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

void* VertexBuffer::lock(LockType type)
{
  if (locked)
  {
    Log::writeError("Vertex buffer already locked");
    return NULL;
  }

  apply();

  void* mapping = NULL;

  if (bufferID)
  {
    mapping = glMapBufferARB(GL_ARRAY_BUFFER_ARB, convertLockType(type));
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

  apply();

  if (bufferID)
  {
    if (!glUnmapBufferARB(GL_ARRAY_BUFFER_ARB))
      Log::writeWarning("Data for vertex buffer object was corrupted");
  }

  locked = false;
}

void VertexBuffer::copyFrom(const void* source, unsigned int sourceCount, unsigned int start)
{
  if (locked)
  {
    Log::writeError("Cannot copy data into locked vertex buffer");
    return;
  }

  if (start + sourceCount > count)
  {
    Log::writeError("Too many vertices submitted");
    return;
  }

  apply();

  const size_t size = format.getSize();

  if (bufferID)
    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, start * size, sourceCount * size, source);
  else
    data.copyFrom(reinterpret_cast<const Byte*>(source), sourceCount * size, start * size);
}

void VertexBuffer::copyTo(void* target, unsigned int targetCount, unsigned int start)
{
  if (locked)
  {
    Log::writeError("Cannot copy data from locked vertex buffer");
    return;
  }

  if (start + targetCount > count)
  {
    Log::writeError("Too many vertices requested");
    return;
  }

  apply();

  const size_t size = format.getSize();

  if (bufferID)
    glGetBufferSubDataARB(GL_ARRAY_BUFFER_ARB, start * size, targetCount * size, target);
  else
    data.copyTo(reinterpret_cast<Byte*>(target), targetCount * size, start * size);
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

    apply();

    glBufferDataARB(GL_ARRAY_BUFFER_ARB,
		    initCount * initFormat.getSize(),
		    NULL,
		    convertUsage(initUsage));

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

void VertexRange::render(unsigned int mode) const
{
  if (!vertexBuffer || count == 0)
  {
    Log::writeError("Cannot render empty vertex buffer range");
    return;
  }

  vertexBuffer->render(mode, start, count);
}

void* VertexRange::lock(LockType type) const
{
  if (!vertexBuffer || count == 0)
  {
    Log::writeError("Cannot lock empty vertex buffer range");
    return NULL;
  }

  Byte* vertices = (Byte*) vertexBuffer->lock(type);
  if (!vertices)
    return NULL;

  return vertices + start * vertexBuffer->getFormat().getSize();
}

void VertexRange::unlock(void) const
{
  if (!vertexBuffer)
  {
    Log::writeError("Cannot unlock non-locked vertex buffer");
    return;
  }

  vertexBuffer->unlock();
}

void VertexRange::copyFrom(const void* source)
{
  if (!vertexBuffer)
    return;

  vertexBuffer->copyFrom(source, count, start);
}

void VertexRange::copyTo(void* target)
{
  if (!vertexBuffer)
    return;

  vertexBuffer->copyTo(target, count, start);
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

  if (bufferID)
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

  if (!bufferID)
    base = data;

  glDrawElements(mode, count, type, base + getTypeSize(type) * start);
}

void* IndexBuffer::lock(LockType type)
{
  if (locked)
  {
    Log::writeError("Index buffer already locked");
    return NULL;
  }

  apply();

  void* mapping = NULL;

  if (bufferID)
  {
    mapping = glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, convertLockType(type));
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

  apply();

  if (bufferID)
  {
    if (!glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB))
      Log::writeWarning("Data for index buffer object was corrupted");
  }

  locked = false;
}

void IndexBuffer::copyFrom(const void* source, unsigned int sourceCount, unsigned int start)
{
  if (locked)
  {
    Log::writeError("Cannot copy data into locked index buffer");
    return;
  }

  if (start + sourceCount > count)
  {
    Log::writeError("Too many indices submitted");
    return;
  }

  apply();

  const size_t size = getTypeSize(type);

  if (bufferID)
    glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, start * size, sourceCount * size, source);
  else
    data.copyFrom(reinterpret_cast<const Byte*>(source), sourceCount * size, start * size);
}

void IndexBuffer::copyTo(void* target, unsigned int targetCount, unsigned int start)
{
  if (locked)
  {
    Log::writeError("Cannot copy data from locked index buffer");
    return;
  }

  if (start + targetCount > count)
  {
    Log::writeError("Too many indices requested");
    return;
  }

  apply();

  const size_t size = getTypeSize(type);

  if (bufferID)
    glGetBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, start * size, targetCount * size, target);
  else
    data.copyTo(reinterpret_cast<Byte*>(target), targetCount * size, start * size);
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

    apply();

    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
	            initCount * getTypeSize(initType),
		    NULL,
		    convertUsage(initUsage));

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

void IndexRange::render(const VertexBuffer& vertexBuffer, unsigned int mode) const
{
  if (!indexBuffer || count == 0)
  {
    Log::writeError("Cannot render empty index buffer range");
    return;
  }

  indexBuffer->render(vertexBuffer, mode, start, count);
}

void* IndexRange::lock(LockType type) const
{
  if (!indexBuffer || count == 0)
  {
    Log::writeError("Cannot lock empty index buffer range");
    return NULL;
  }

  Byte* indices = (Byte*) indexBuffer->lock(type);
  if (!indices)
    return NULL;

  return indices + start * getTypeSize(indexBuffer->getType());
}

void IndexRange::unlock(void) const
{
  if (!indexBuffer)
  {
    Log::writeError("Cannot unlock non-locked index buffer");
    return;
  }

  indexBuffer->unlock();
}

void IndexRange::copyFrom(const void* source)
{
  if (!indexBuffer)
    return;

  indexBuffer->copyFrom(source, count, start);
}

void IndexRange::copyTo(void* target)
{
  if (!indexBuffer)
    return;

  indexBuffer->copyTo(target, count, start);
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
