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

#include <wendy/Config.hpp>

#include <wendy/Bimap.hpp>

#include <wendy/GLTexture.hpp>
#include <wendy/GLBuffer.hpp>
#include <wendy/GLProgram.hpp>
#include <wendy/GLContext.hpp>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLHelper.hpp>

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
      return GL_READ_ONLY;
    case LOCK_WRITE_ONLY:
      return GL_WRITE_ONLY;
    case LOCK_READ_WRITE:
      return GL_READ_WRITE;
  }

  panic("Invalid lock type %u", type);
}

GLenum convertToGL(Usage usage)
{
  switch (usage)
  {
    case USAGE_STATIC:
      return GL_STATIC_DRAW;
    case USAGE_STREAM:
      return GL_STREAM_DRAW;
    case USAGE_DYNAMIC:
      return GL_DYNAMIC_DRAW;
  }

  panic("Invalid buffer usage %u", usage);
}

GLenum convertToGL(TextureFramebuffer::Attachment attachment)
{
  switch (attachment)
  {
    case TextureFramebuffer::COLOR_BUFFER0:
      return GL_COLOR_ATTACHMENT0;
    case TextureFramebuffer::COLOR_BUFFER1:
      return GL_COLOR_ATTACHMENT1;
    case TextureFramebuffer::COLOR_BUFFER2:
      return GL_COLOR_ATTACHMENT2;
    case TextureFramebuffer::COLOR_BUFFER3:
      return GL_COLOR_ATTACHMENT3;
    case TextureFramebuffer::DEPTH_BUFFER:
      return GL_DEPTH_ATTACHMENT;
  }

  panic("Invalid framebuffer attachment %u", attachment);
}

const char* asString(TextureFramebuffer::Attachment attachment)
{
  switch (attachment)
  {
    case TextureFramebuffer::COLOR_BUFFER0:
      return "color buffer 0";
    case TextureFramebuffer::COLOR_BUFFER1:
      return "color buffer 1";
    case TextureFramebuffer::COLOR_BUFFER2:
      return "color buffer 2";
    case TextureFramebuffer::COLOR_BUFFER3:
      return "color buffer 3";
    case TextureFramebuffer::DEPTH_BUFFER:
      return "depth buffer";
  }

  panic("Invalid framebuffer attachment %u", attachment);
}

bool isColorAttachment(TextureFramebuffer::Attachment attachment)
{
  switch (attachment)
  {
    case TextureFramebuffer::COLOR_BUFFER0:
    case TextureFramebuffer::COLOR_BUFFER1:
    case TextureFramebuffer::COLOR_BUFFER2:
    case TextureFramebuffer::COLOR_BUFFER3:
      return true;
    default:
      return false;
  }
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

VertexBuffer::~VertexBuffer()
{
  if (m_locked)
    logWarning("Vertex buffer destroyed while locked");

  if (m_bufferID)
    glDeleteBuffers(1, &m_bufferID);

  if (Stats* stats = m_context.stats())
    stats->removeVertexBuffer(size());
}

void* VertexBuffer::lock(LockType type)
{
  if (m_locked)
  {
    logError("Vertex buffer already locked");
    return nullptr;
  }

  m_context.setCurrentVertexBuffer(this);

  void* mapping = glMapBuffer(GL_ARRAY_BUFFER, convertToGL(type));
  if (mapping == nullptr)
  {
    checkGL("Failed to lock vertex buffer");
    return nullptr;
  }

  m_locked = true;
  return mapping;
}

void VertexBuffer::unlock()
{
  if (!m_locked)
  {
    logWarning("Cannot unlock non-locked vertex buffer");
    return;
  }

  m_context.setCurrentVertexBuffer(this);

  if (!glUnmapBuffer(GL_ARRAY_BUFFER))
    logWarning("Data for vertex buffer was corrupted");

  m_locked = false;
}

void VertexBuffer::copyFrom(const void* source, size_t sourceCount, size_t start)
{
  if (m_locked)
  {
    logError("Cannot copy data into locked vertex buffer");
    return;
  }

  if (start + sourceCount > m_count)
  {
    logError("Too many vertices submitted to vertex buffer");
    return;
  }

  m_context.setCurrentVertexBuffer(this);

  const size_t size = m_format.size();
  glBufferSubData(GL_ARRAY_BUFFER, start * size, sourceCount * size, source);

#if WENDY_DEBUG
  checkGL("Error during copy to vertex buffer");
#endif
}

void VertexBuffer::copyTo(void* target, size_t targetCount, size_t start)
{
  if (m_locked)
  {
    logError("Cannot copy data from locked vertex buffer");
    return;
  }

  if (start + targetCount > m_count)
  {
    logError("Too many vertices requested from vertex buffer");
    return;
  }

  m_context.setCurrentVertexBuffer(this);

  const size_t size = m_format.size();
  glGetBufferSubData(GL_ARRAY_BUFFER, start * size, targetCount * size, target);

#if WENDY_DEBUG
  checkGL("Error during copy from vertex buffer");
#endif
}

Ref<VertexBuffer> VertexBuffer::create(Context& context,
                                       size_t count,
                                       const VertexFormat& format,
                                       Usage usage)
{
  Ref<VertexBuffer> buffer(new VertexBuffer(context));
  if (!buffer->init(format, count, usage))
    return nullptr;

  return buffer;
}

VertexBuffer::VertexBuffer(Context& context):
  m_context(context),
  m_locked(false),
  m_bufferID(0),
  m_count(0),
  m_usage(USAGE_STATIC)
{
}

bool VertexBuffer::init(const VertexFormat& format, size_t count, Usage usage)
{
  m_format = format;
  m_usage = usage;
  m_count = count;

  glGenBuffers(1, &m_bufferID);

  m_context.setCurrentVertexBuffer(this);

  glBufferData(GL_ARRAY_BUFFER,
               m_count * m_format.size(),
               nullptr,
               convertToGL(m_usage));

  if (!checkGL("Error during creation of vertex buffer of format %s",
               m_format.asString().c_str()))
  {
    m_context.setCurrentVertexBuffer(nullptr);
    return false;
  }

  if (Stats* stats = m_context.stats())
    stats->addVertexBuffer(size());

  return true;
}

///////////////////////////////////////////////////////////////////////

IndexBuffer::~IndexBuffer()
{
  if (m_locked)
    logWarning("Index buffer destroyed while locked");

  if (m_bufferID)
    glDeleteBuffers(1, &m_bufferID);

  if (Stats* stats = m_context.stats())
    stats->removeIndexBuffer(size());
}

void* IndexBuffer::lock(LockType type)
{
  if (m_locked)
  {
    logError("Index buffer already locked");
    return nullptr;
  }

  m_context.setCurrentIndexBuffer(this);

  void* mapping = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, convertToGL(type));
  if (mapping == nullptr)
  {
    checkGL("Failed to lock index buffer");
    return nullptr;
  }

  m_locked = true;
  return mapping;
}

void IndexBuffer::unlock()
{
  if (!m_locked)
  {
    logWarning("Cannot unlock non-locked index buffer");
    return;
  }

  m_context.setCurrentIndexBuffer(this);

  if (!glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER))
    logWarning("Data for index buffer was corrupted");

  m_locked = false;
}

void IndexBuffer::copyFrom(const void* source, size_t sourceCount, size_t start)
{
  if (m_locked)
  {
    logError("Cannot copy data into locked index buffer");
    return;
  }

  if (start + sourceCount > m_count)
  {
    logError("Too many indices submitted to index buffer");
    return;
  }

  m_context.setCurrentIndexBuffer(this);

  const size_t size = typeSize(m_type);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, start * size, sourceCount * size, source);

#if WENDY_DEBUG
  checkGL("Error during copy to index buffer");
#endif
}

void IndexBuffer::copyTo(void* target, size_t targetCount, size_t start)
{
  if (m_locked)
  {
    logError("Cannot copy data from locked index buffer");
    return;
  }

  if (start + targetCount > m_count)
  {
    logError("Too many indices requested from index buffer");
    return;
  }

  m_context.setCurrentIndexBuffer(this);

  const size_t size = typeSize(m_type);
  glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, start * size, targetCount * size, target);

#if WENDY_DEBUG
  checkGL("Error during copy from index buffer");
#endif
}

size_t IndexBuffer::size() const
{
  return m_count * typeSize(m_type);
}

Ref<IndexBuffer> IndexBuffer::create(Context& context,
                                     size_t count,
                                     IndexType type,
                                     Usage usage)
{
  Ref<IndexBuffer> buffer(new IndexBuffer(context));
  if (!buffer->init(count, type, usage))
    return nullptr;

  return buffer;
}

size_t IndexBuffer::typeSize(IndexType type)
{
  switch (type)
  {
    case INDEX_UINT8:
      return sizeof(GLubyte);
    case INDEX_UINT16:
      return sizeof(GLushort);
    case INDEX_UINT32:
      return sizeof(GLuint);
  }

  panic("Invalid index buffer type %u", type);
}

IndexBuffer::IndexBuffer(Context& context):
  m_context(context),
  m_locked(false),
  m_type(INDEX_UINT8),
  m_usage(USAGE_STATIC),
  m_bufferID(0),
  m_count(0)
{
}

bool IndexBuffer::init(size_t count, IndexType type, Usage usage)
{
  m_type = type;
  m_usage = usage;
  m_count = count;

  glGenBuffers(1, &m_bufferID);

  m_context.setCurrentIndexBuffer(this);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               m_count * typeSize(m_type),
               nullptr,
               convertToGL(m_usage));

  if (!checkGL("Error during creation of index buffer of element size %u",
               (uint) typeSize(m_type)))
  {
    m_context.setCurrentIndexBuffer(nullptr);
    return false;
  }

  if (Stats* stats = m_context.stats())
    stats->addIndexBuffer(size());

  return true;
}

///////////////////////////////////////////////////////////////////////

VertexRange::VertexRange():
  m_buffer(nullptr),
  m_start(0),
  m_count(0)
{
}

VertexRange::VertexRange(VertexBuffer& vertexBuffer):
  m_buffer(&vertexBuffer),
  m_start(0),
  m_count(0)
{
  m_count = m_buffer->count();
}

VertexRange::VertexRange(VertexBuffer& vertexBuffer,
                         size_t start,
                         size_t count):
  m_buffer(&vertexBuffer),
  m_start(start),
  m_count(count)
{
  assert(m_buffer->count() >= m_start + m_count);
}

void* VertexRange::lock(LockType type) const
{
  if (!m_buffer || m_count == 0)
  {
    logError("Cannot lock empty vertex buffer range");
    return nullptr;
  }

  uint8* vertices = (uint8*) m_buffer->lock(type);
  if (!vertices)
    return nullptr;

  return vertices + m_start * m_buffer->format().size();
}

void VertexRange::unlock() const
{
  if (!m_buffer)
  {
    logError("Cannot unlock non-locked vertex buffer");
    return;
  }

  m_buffer->unlock();
}

void VertexRange::copyFrom(const void* source)
{
  if (!m_buffer)
    return;

  m_buffer->copyFrom(source, m_count, m_start);
}

void VertexRange::copyTo(void* target)
{
  if (!m_buffer)
    return;

  m_buffer->copyTo(target, m_count, m_start);
}

///////////////////////////////////////////////////////////////////////

IndexRange::IndexRange():
  m_buffer(nullptr),
  m_start(0),
  m_count(0)
{
}

IndexRange::IndexRange(IndexBuffer& buffer):
  m_buffer(&buffer),
  m_start(0),
  m_count(0)
{
  m_count = m_buffer->count();
}

IndexRange::IndexRange(IndexBuffer& buffer,
                       size_t start,
                       size_t count):
  m_buffer(&buffer),
  m_start(start),
  m_count(count)
{
  assert(m_buffer->count() >= m_start + m_count);
}

void* IndexRange::lock(LockType type) const
{
  if (!m_buffer || m_count == 0)
  {
    logError("Cannot lock empty index buffer range");
    return nullptr;
  }

  uint8* indices = (uint8*) m_buffer->lock(type);
  if (!indices)
    return nullptr;

  return indices + m_start * IndexBuffer::typeSize(m_buffer->type());
}

void IndexRange::unlock() const
{
  if (!m_buffer)
  {
    logError("Cannot unlock non-locked index buffer");
    return;
  }

  m_buffer->unlock();
}

void IndexRange::copyFrom(const void* source)
{
  if (!m_buffer)
    return;

  m_buffer->copyFrom(source, m_count, m_start);
}

void IndexRange::copyTo(void* target)
{
  if (!m_buffer)
    return;

  m_buffer->copyTo(target, m_count, m_start);
}

///////////////////////////////////////////////////////////////////////

PrimitiveRange::PrimitiveRange():
  m_type(TRIANGLE_LIST),
  m_vertexBuffer(nullptr),
  m_indexBuffer(nullptr),
  m_start(0),
  m_count(0),
  m_base(0)
{
}

PrimitiveRange::PrimitiveRange(PrimitiveType type,
                               VertexBuffer& vertexBuffer):
  m_type(type),
  m_vertexBuffer(&vertexBuffer),
  m_indexBuffer(nullptr),
  m_start(0),
  m_count(0),
  m_base(0)
{
  m_count = vertexBuffer.count();
}

PrimitiveRange::PrimitiveRange(PrimitiveType type,
                               const VertexRange& vertexRange):
  m_type(type),
  m_vertexBuffer(nullptr),
  m_indexBuffer(nullptr),
  m_start(0),
  m_count(0),
  m_base(0)
{
  m_vertexBuffer = vertexRange.vertexBuffer();
  m_start = vertexRange.start();
  m_count = vertexRange.count();
}

PrimitiveRange::PrimitiveRange(PrimitiveType type,
                               VertexBuffer& vertexBuffer,
                               IndexBuffer& indexBuffer,
                               size_t base):
  m_type(type),
  m_vertexBuffer(&vertexBuffer),
  m_indexBuffer(&indexBuffer),
  m_start(0),
  m_count(0),
  m_base(base)
{
  m_count = indexBuffer.count();
}

PrimitiveRange::PrimitiveRange(PrimitiveType type,
                               VertexBuffer& vertexBuffer,
                               const IndexRange& indexRange,
                               size_t base):
  m_type(type),
  m_vertexBuffer(&vertexBuffer),
  m_indexBuffer(nullptr),
  m_start(0),
  m_count(0),
  m_base(base)
{
  m_indexBuffer = indexRange.indexBuffer();
  m_start = indexRange.start();
  m_count = indexRange.count();
}

PrimitiveRange::PrimitiveRange(PrimitiveType type,
                               VertexBuffer& vertexBuffer,
                               size_t start,
                               size_t count,
                               size_t base):
  m_type(type),
  m_vertexBuffer(&vertexBuffer),
  m_indexBuffer(nullptr),
  m_start(start),
  m_count(count),
  m_base(base)
{
}

PrimitiveRange::PrimitiveRange(PrimitiveType type,
                               VertexBuffer& vertexBuffer,
                               IndexBuffer& indexBuffer,
                               size_t start,
                               size_t count,
                               size_t base):
  m_type(type),
  m_vertexBuffer(&vertexBuffer),
  m_indexBuffer(&indexBuffer),
  m_start(start),
  m_count(count),
  m_base(base)
{
}

bool PrimitiveRange::isEmpty() const
{
  if (m_vertexBuffer == nullptr)
    return true;

  return m_count == 0;
}

///////////////////////////////////////////////////////////////////////

template <>
IndexRangeLock<uint8>::IndexRangeLock(IndexRange& initRange):
  range(initRange),
  indices(nullptr)
{
  if (IndexBuffer* indexBuffer = range.indexBuffer())
  {
    if (indexBuffer->type() != INDEX_UINT8)
      panic("Index buffer is not of type UINT8");
  }

  indices = (uint8*) range.lock();
  if (!indices)
    panic("Failed to lock index buffer");
}

template <>
IndexRangeLock<uint16>::IndexRangeLock(IndexRange& initRange):
  range(initRange),
  indices(nullptr)
{
  if (IndexBuffer* indexBuffer = range.indexBuffer())
  {
    if (indexBuffer->type() != INDEX_UINT16)
      panic("Index buffer is not of type UINT16");
  }

  indices = (uint16*) range.lock();
  if (!indices)
    panic("Failed to lock index buffer");
}

template <>
IndexRangeLock<uint32>::IndexRangeLock(IndexRange& initRange):
  range(initRange),
  indices(nullptr)
{
  if (IndexBuffer* indexBuffer = range.indexBuffer())
  {
    if (indexBuffer->type() != INDEX_UINT32)
      panic("Index buffer is not of type UINT32");
  }

  indices = (uint32*) range.lock();
  if (!indices)
    panic("Failed to lock index buffer");
}

///////////////////////////////////////////////////////////////////////

Framebuffer::~Framebuffer()
{
}

void Framebuffer::setSRGB(bool enabled)
{
  if (m_sRGB == enabled)
    return;

  Framebuffer& previous = m_context.currentFramebuffer();
  apply();

  if (enabled)
  {
    glEnable(GL_FRAMEBUFFER_SRGB);
    checkGL("Failed to enable framebuffer sRGB encoding");
  }
  else
  {
    glDisable(GL_FRAMEBUFFER_SRGB);
    checkGL("Failed to disable framebuffer sRGB encoding");
  }

  m_sRGB = enabled;

  previous.apply();
}

Framebuffer::Framebuffer(Context& context):
  m_context(context),
  m_sRGB(false)
{
}

///////////////////////////////////////////////////////////////////////

DefaultFramebuffer::DefaultFramebuffer(Context& context):
  Framebuffer(context)
{
}

void DefaultFramebuffer::apply() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

#if WENDY_DEBUG
  checkGL("Error when applying default framebuffer");
#endif
}

uint DefaultFramebuffer::width() const
{
  return context().window().width();
}

uint DefaultFramebuffer::height() const
{
  context().window().height();
}

///////////////////////////////////////////////////////////////////////

TextureFramebuffer::~TextureFramebuffer()
{
  if (m_bufferID)
    glDeleteFramebuffers(1, &m_bufferID);
}

uint TextureFramebuffer::width() const
{
  uint width = 0;

  for (size_t i = 0;  i < 5;  i++)
  {
    if (m_images[i])
    {
      if (width && width != m_images[i]->width())
        return 0;

      width = m_images[i]->width();
    }
  }

  return width;
}

uint TextureFramebuffer::height() const
{
  uint height = 0;

  for (size_t i = 0;  i < 5;  i++)
  {
    if (m_images[i])
    {
      if (height && height != m_images[i]->height())
        return 0;

      height = m_images[i]->height();
    }
  }

  return height;
}

TextureImage* TextureFramebuffer::buffer(Attachment attachment) const
{
  return m_images[attachment];
}

bool TextureFramebuffer::setDepthBuffer(TextureImage* newImage)
{
  return setBuffer(DEPTH_BUFFER, newImage);
}

bool TextureFramebuffer::setColorBuffer(TextureImage* newImage)
{
  return setBuffer(COLOR_BUFFER0, newImage);
}

bool TextureFramebuffer::setBuffer(Attachment attachment, TextureImage* newImage, uint z)
{
  if (isColorAttachment(attachment))
  {
    const Limits& limits = m_context.limits();
    const uint index = attachment - COLOR_BUFFER0;

    if (index >= limits.maxColorAttachments)
    {
      logError("OpenGL context supports at most %u FBO color attachments",
               limits.maxColorAttachments);
      return false;
    }

    if (index >= limits.maxDrawBuffers)
    {
      logError("OpenGL context supports at most %u draw buffers",
               limits.maxDrawBuffers);
      return false;
    }
  }

  Framebuffer& previous = m_context.currentFramebuffer();
  apply();

  if (m_images[attachment])
    m_images[attachment]->detach(convertToGL(attachment));

  m_images[attachment] = newImage;

  if (m_images[attachment])
    m_images[attachment]->attach(convertToGL(attachment), z);

  previous.apply();
  return true;
}

Ref<TextureFramebuffer> TextureFramebuffer::create(Context& context)
{
  Ref<TextureFramebuffer> framebuffer(new TextureFramebuffer(context));
  if (!framebuffer->init())
    return nullptr;

  return framebuffer;
}

TextureFramebuffer::TextureFramebuffer(Context& context):
  Framebuffer(context),
  m_bufferID(0)
{
}

bool TextureFramebuffer::init()
{
  glGenFramebuffers(1, &m_bufferID);

#if WENDY_DEBUG
  if (!checkGL("Error during image framebuffer creation"))
    return false;
#endif

  return true;
}

void TextureFramebuffer::apply() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_bufferID);

  GLenum enables[5];
  size_t count = 0;

  for (size_t i = 0;  i < sizeof(enables) / sizeof(enables[0]);  i++)
  {
    Attachment attachment = (Attachment) i;

    if (m_images[i] && isColorAttachment(attachment))
      enables[count++] = convertToGL(attachment);
  }

  if (count)
    glDrawBuffers(count, enables);
  else
    glDrawBuffer(GL_NONE);

#if WENDY_DEBUG
  checkGL("Error when applying image framebuffer");
#endif
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
