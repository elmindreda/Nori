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

#include <wendy/Config.h>

#include <wendy/Bimap.h>

#include <wendy/OpenGL.h>
#include <wendy/GLBuffer.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLConvert.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

namespace
{

GLenum convertToGL(LockType type)
{
  switch (type)
  {
    case LOCK_READ_ONLY:
      return GL_READ_ONLY_ARB;
    case LOCK_WRITE_ONLY:
      return GL_WRITE_ONLY_ARB;
    case LOCK_READ_WRITE:
      return GL_READ_WRITE_ARB;
  }

  Log::writeError("Invalid lock type %u", type);
  return 0;
}

GLenum convertToGL(IndexBuffer::Usage usage)
{
  switch (usage)
  {
    case IndexBuffer::STATIC:
      return GL_STATIC_DRAW_ARB;
    case IndexBuffer::STREAM:
      return GL_STREAM_DRAW_ARB;
    case IndexBuffer::DYNAMIC:
      return GL_DYNAMIC_DRAW_ARB;
  }

  Log::writeError("Invalid index buffer usage %u", usage);
  return 0;
}

GLenum convertToGL(VertexBuffer::Usage usage)
{
  switch (usage)
  {
    case VertexBuffer::STATIC:
      return GL_STATIC_DRAW_ARB;
    case VertexBuffer::STREAM:
      return GL_STREAM_DRAW_ARB;
    case VertexBuffer::DYNAMIC:
      return GL_DYNAMIC_DRAW_ARB;
  }

  Log::writeError("Invalid vertex buffer usage %u", usage);
  return 0;
}

} /*namespace*/

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

void* VertexBuffer::lock(LockType type)
{
  if (locked)
  {
    Log::writeError("Vertex buffer already locked");
    return NULL;
  }

  apply();

  void* mapping = glMapBufferARB(GL_ARRAY_BUFFER_ARB, convertToGL(type));
  if (mapping == NULL)
  {
    Log::writeError("Failed to lock vertex buffer: %s",
                    gluErrorString(glGetError()));
    return NULL;
  }

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

  if (!glUnmapBufferARB(GL_ARRAY_BUFFER_ARB))
    Log::writeWarning("Data for vertex buffer was corrupted");

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
    Log::writeError("Too many vertices submitted to vertex buffer");
    return;
  }

  apply();

  const size_t size = format.getSize();
  glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, start * size, sourceCount * size, source);

#if WENDY_DEBUG
  checkGL("Error during copy to vertex buffer");
#endif
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
    Log::writeError("Too many vertices requested from vertex buffer");
    return;
  }

  apply();

  const size_t size = format.getSize();
  glGetBufferSubDataARB(GL_ARRAY_BUFFER_ARB, start * size, targetCount * size, target);

#if WENDY_DEBUG
  checkGL("Error during copy from vertex buffer");
#endif
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

Ref<VertexBuffer> VertexBuffer::create(Context& context,
                                       unsigned int count,
                                       const VertexFormat& format,
				       Usage usage)
{
  Ref<VertexBuffer> buffer(new VertexBuffer(context));
  if (!buffer->init(format, count, usage))
    return NULL;

  return buffer;
}

VertexBuffer::VertexBuffer(Context& initContext):
  context(initContext),
  locked(false),
  bufferID(0),
  count(0),
  usage(STATIC)
{
}

VertexBuffer::VertexBuffer(const VertexBuffer& source):
  context(source.context)
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
  format = initFormat;
  usage = initUsage;
  count = initCount;

  // Clear any errors
  glGetError();

  glGenBuffersARB(1, &bufferID);

  apply();

  glBufferDataARB(GL_ARRAY_BUFFER_ARB,
		  count * format.getSize(),
		  NULL,
		  convertToGL(usage));

  if (!checkGL("Error during creation of vertex buffer of format \'%s\'",
               format.asString().c_str()))
  {
    return false;
  }

  return true;
}

void VertexBuffer::apply(void) const
{
  if (current == this)
    return;

  glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferID);

#if WENDY_DEBUG
  if (!checkGL("Failed to apply index buffer"))
    return;
#endif

  current = const_cast<VertexBuffer*>(this);
}

void VertexBuffer::invalidateCurrent(void)
{
  current = NULL;
}

VertexBuffer* VertexBuffer::current = NULL;

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

void* IndexBuffer::lock(LockType type)
{
  if (locked)
  {
    Log::writeError("Index buffer already locked");
    return NULL;
  }

  apply();

  void* mapping = glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, convertToGL(type));
  if (mapping == NULL)
  {
    Log::writeError("Failed to lock index buffer: %s",
                    gluErrorString(glGetError()));
    return NULL;
  }

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

  if (!glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB))
    Log::writeWarning("Data for index buffer was corrupted");

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
    Log::writeError("Too many indices submitted to index buffer");
    return;
  }

  apply();

  const size_t size = getTypeSize(type);
  glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, start * size, sourceCount * size, source);

#if WENDY_DEBUG
  checkGL("Error during copy to index buffer");
#endif
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
    Log::writeError("Too many indices requested from index buffer");
    return;
  }

  apply();

  const size_t size = getTypeSize(type);
  glGetBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, start * size, targetCount * size, target);

#if WENDY_DEBUG
  checkGL("Error during copy from index buffer");
#endif
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

Ref<IndexBuffer> IndexBuffer::create(Context& context,
                                     unsigned int count,
				     Type type,
				     Usage usage)
{
  Ref<IndexBuffer> buffer(new IndexBuffer(context));
  if (!buffer->init(count, type, usage))
    return NULL;

  return buffer;
}

size_t IndexBuffer::getTypeSize(Type type)
{
  switch (type)
  {
    case IndexBuffer::UINT8:
      return sizeof(GLubyte);
    case IndexBuffer::UINT16:
      return sizeof(GLushort);
    case IndexBuffer::UINT32:
      return sizeof(GLuint);
    default:
      Log::writeError("Invalid index buffer type %u", type);
      return 0;
  }
}

IndexBuffer::IndexBuffer(Context& initContext):
  context(initContext),
  locked(false),
  type(UINT32),
  usage(STATIC),
  bufferID(0),
  count(0)
{
}

IndexBuffer::IndexBuffer(const IndexBuffer& source):
  context(source.context)
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
  type = initType;
  usage = initUsage;
  count = initCount;

  // Clear any errors
  glGetError();

  glGenBuffersARB(1, &bufferID);

  apply();

  glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
		  count * getTypeSize(type),
		  NULL,
		  convertToGL(usage));

  if (!checkGL("Error during creation of index buffer of element size %u",
               getTypeSize(type)))
  {
    return false;
  }

  return true;
}

void IndexBuffer::apply(void) const
{
  if (current == this)
    return;

  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferID);

#if WENDY_DEBUG
  if (!checkGL("Failed to apply index buffer"))
    return;
#endif

  current = const_cast<IndexBuffer*>(this);
}

void IndexBuffer::invalidateCurrent(void)
{
  current = NULL;
}

IndexBuffer* IndexBuffer::current = NULL;

///////////////////////////////////////////////////////////////////////

VertexRange::VertexRange(void):
  start(0),
  count(0)
{
}

VertexRange::VertexRange(VertexBuffer& initVertexBuffer):
  vertexBuffer(&initVertexBuffer),
  start(0),
  count(0)
{
  count = vertexBuffer->getCount();
}

VertexRange::VertexRange(VertexBuffer& initVertexBuffer,
                         unsigned int initStart,
                         unsigned int initCount):
  vertexBuffer(&initVertexBuffer),
  start(initStart),
  count(initCount)
{
  if (vertexBuffer->getCount() < start + count)
    throw Exception("Vertex range is partially or completely outside the specified vertex buffer");
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

IndexRange::IndexRange(void):
  start(0),
  count(0)
{
}

IndexRange::IndexRange(IndexBuffer& initIndexBuffer):
  indexBuffer(&initIndexBuffer),
  start(0),
  count(0)
{
  count = indexBuffer->getCount();
}

IndexRange::IndexRange(IndexBuffer& initIndexBuffer,
                       unsigned int initStart,
                       unsigned int initCount):
  indexBuffer(&initIndexBuffer),
  start(initStart),
  count(initCount)
{
  if (indexBuffer->getCount() < start + count)
    throw Exception("Index range is partially or completely outside the specified index buffer");
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

  return indices + start * IndexBuffer::getTypeSize(indexBuffer->getType());
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

PrimitiveRange::PrimitiveRange(void):
  type(TRIANGLE_LIST),
  start(0),
  count(0)
{
}

PrimitiveRange::PrimitiveRange(PrimitiveType initType,
	                       VertexBuffer& initVertexBuffer):
  type(initType),
  vertexBuffer(&initVertexBuffer),
  start(0),
  count(0)
{
  count = vertexBuffer->getCount();
}

PrimitiveRange::PrimitiveRange(PrimitiveType initType,
                               const VertexRange& vertexRange):
  type(initType),
  start(0),
  count(0)
{
  vertexBuffer = vertexRange.getVertexBuffer();
  start = vertexRange.getStart();
  count = vertexRange.getCount();
}

PrimitiveRange::PrimitiveRange(PrimitiveType initType,
	                       VertexBuffer& initVertexBuffer,
	                       IndexBuffer& initIndexBuffer):
  type(initType),
  vertexBuffer(&initVertexBuffer),
  indexBuffer(&initIndexBuffer),
  start(0),
  count(0)
{
  count = indexBuffer->getCount();
}

PrimitiveRange::PrimitiveRange(PrimitiveType initType,
	                       VertexBuffer& initVertexBuffer,
	                       const IndexRange& indexRange):
  type(initType),
  vertexBuffer(&initVertexBuffer),
  start(0),
  count(0)
{
  indexBuffer = indexRange.getIndexBuffer();
  start = indexRange.getStart();
  count = indexRange.getCount();
}

PrimitiveRange::PrimitiveRange(PrimitiveType initType,
	                       VertexBuffer& initVertexBuffer,
	                       unsigned int initStart,
	                       unsigned int initCount):
  type(initType),
  vertexBuffer(&initVertexBuffer),
  start(initStart),
  count(initCount)
{
}

PrimitiveRange::PrimitiveRange(PrimitiveType initType,
	                       VertexBuffer& initVertexBuffer,
			       IndexBuffer& initIndexBuffer,
	                       unsigned int initStart,
	                       unsigned int initCount):
  type(initType),
  vertexBuffer(&initVertexBuffer),
  indexBuffer(&initIndexBuffer),
  start(initStart),
  count(initCount)
{
}

bool PrimitiveRange::isEmpty(void) const
{
  if (vertexBuffer == NULL)
    return true;

  return count == 0;
}

PrimitiveType PrimitiveRange::getType(void) const
{
  return type;
}

VertexBuffer* PrimitiveRange::getVertexBuffer(void) const
{
  return vertexBuffer;
}

IndexBuffer* PrimitiveRange::getIndexBuffer(void) const
{
  return indexBuffer;
}

unsigned int PrimitiveRange::getStart(void) const
{
  return start;
}

unsigned int PrimitiveRange::getCount(void) const
{
  return count;
}

///////////////////////////////////////////////////////////////////////

RenderBuffer::~RenderBuffer(void)
{
  if (bufferID)
    glDeleteRenderbuffersEXT(1, &bufferID);
}

unsigned int RenderBuffer::getWidth(void) const
{
  return width;
}

unsigned int RenderBuffer::getHeight(void) const
{
  return height;
}

const PixelFormat& RenderBuffer::getFormat(void) const
{
  return format;
}

Ref<RenderBuffer> RenderBuffer::create(const PixelFormat& format,
                                       unsigned int width,
                                       unsigned int height)
{
  Ref<RenderBuffer> buffer(new RenderBuffer());
  if (!buffer->init(format, width, height))
    return NULL;

  return buffer;
}

RenderBuffer::RenderBuffer(void):
  bufferID(0)
{
}

bool RenderBuffer::init(const PixelFormat& initFormat,
                        unsigned int initWidth,
                        unsigned int initHeight)
{
  format = initFormat;
  width = initWidth;
  height = initHeight;

  glGenRenderbuffersEXT(1, &bufferID);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, bufferID);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
                           convertToGL(format.getSemantic()),
                           width,
                           height);

  if (!checkGL("Error during creation of render buffer of format \'%s\'",
               format.asString().c_str()))
  {
    return false;
  }

  return true;
}

void RenderBuffer::attach(int attachment)
{
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                               attachment,
                               GL_RENDERBUFFER_EXT,
                               bufferID);
}

void RenderBuffer::detach(int attachment)
{
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                               attachment,
                               GL_RENDERBUFFER_EXT,
                               0);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
