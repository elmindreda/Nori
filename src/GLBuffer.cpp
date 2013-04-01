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

#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLHelper.h>

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
  if (locked)
    logWarning("Vertex buffer destroyed while locked");

  if (bufferID)
    glDeleteBuffers(1, &bufferID);

  if (Stats* stats = context.getStats())
    stats->removeVertexBuffer(getSize());
}

void* VertexBuffer::lock(LockType type)
{
  if (locked)
  {
    logError("Vertex buffer already locked");
    return NULL;
  }

  context.setCurrentVertexBuffer(this);

  void* mapping = glMapBuffer(GL_ARRAY_BUFFER, convertToGL(type));
  if (mapping == NULL)
  {
    checkGL("Failed to lock vertex buffer");
    return NULL;
  }

  locked = true;
  return mapping;
}

void VertexBuffer::unlock()
{
  if (!locked)
  {
    logWarning("Cannot unlock non-locked vertex buffer");
    return;
  }

  context.setCurrentVertexBuffer(this);

  if (!glUnmapBuffer(GL_ARRAY_BUFFER))
    logWarning("Data for vertex buffer was corrupted");

  locked = false;
}

void VertexBuffer::copyFrom(const void* source, size_t sourceCount, size_t start)
{
  if (locked)
  {
    logError("Cannot copy data into locked vertex buffer");
    return;
  }

  if (start + sourceCount > count)
  {
    logError("Too many vertices submitted to vertex buffer");
    return;
  }

  context.setCurrentVertexBuffer(this);

  const size_t size = format.getSize();
  glBufferSubData(GL_ARRAY_BUFFER, start * size, sourceCount * size, source);

#if WENDY_DEBUG
  checkGL("Error during copy to vertex buffer");
#endif
}

void VertexBuffer::copyTo(void* target, size_t targetCount, size_t start)
{
  if (locked)
  {
    logError("Cannot copy data from locked vertex buffer");
    return;
  }

  if (start + targetCount > count)
  {
    logError("Too many vertices requested from vertex buffer");
    return;
  }

  context.setCurrentVertexBuffer(this);

  const size_t size = format.getSize();
  glGetBufferSubData(GL_ARRAY_BUFFER, start * size, targetCount * size, target);

#if WENDY_DEBUG
  checkGL("Error during copy from vertex buffer");
#endif
}

Usage VertexBuffer::getUsage() const
{
  return usage;
}

const VertexFormat& VertexBuffer::getFormat() const
{
  return format;
}

size_t VertexBuffer::getCount() const
{
  return count;
}

size_t VertexBuffer::getSize() const
{
  return count * format.getSize();
}

Ref<VertexBuffer> VertexBuffer::create(Context& context,
                                       size_t count,
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
  usage(USAGE_STATIC)
{
}

VertexBuffer::VertexBuffer(const VertexBuffer& source):
  context(source.context)
{
  panic("Vertex buffers may not be copied");
}

bool VertexBuffer::init(const VertexFormat& initFormat,
                        size_t initCount,
                        Usage initUsage)
{
  format = initFormat;
  usage = initUsage;
  count = initCount;

  glGenBuffers(1, &bufferID);

  context.setCurrentVertexBuffer(this);

  glBufferData(GL_ARRAY_BUFFER,
               count * format.getSize(),
               NULL,
               convertToGL(usage));

  if (!checkGL("Error during creation of vertex buffer of format \'%s\'",
               format.asString().c_str()))
  {
    context.setCurrentVertexBuffer(NULL);
    return false;
  }

  if (Stats* stats = context.getStats())
    stats->addVertexBuffer(getSize());

  return true;
}

VertexBuffer& VertexBuffer::operator = (const VertexBuffer& source)
{
  panic("Vertex buffers may not be assigned");
}

///////////////////////////////////////////////////////////////////////

IndexBuffer::~IndexBuffer()
{
  if (locked)
    logWarning("Index buffer destroyed while locked");

  if (bufferID)
    glDeleteBuffers(1, &bufferID);

  if (Stats* stats = context.getStats())
    stats->removeIndexBuffer(getSize());
}

void* IndexBuffer::lock(LockType type)
{
  if (locked)
  {
    logError("Index buffer already locked");
    return NULL;
  }

  context.setCurrentIndexBuffer(this);

  void* mapping = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, convertToGL(type));
  if (mapping == NULL)
  {
    checkGL("Failed to lock index buffer");
    return NULL;
  }

  locked = true;
  return mapping;
}

void IndexBuffer::unlock()
{
  if (!locked)
  {
    logWarning("Cannot unlock non-locked index buffer");
    return;
  }

  context.setCurrentIndexBuffer(this);

  if (!glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER))
    logWarning("Data for index buffer was corrupted");

  locked = false;
}

void IndexBuffer::copyFrom(const void* source, size_t sourceCount, size_t start)
{
  if (locked)
  {
    logError("Cannot copy data into locked index buffer");
    return;
  }

  if (start + sourceCount > count)
  {
    logError("Too many indices submitted to index buffer");
    return;
  }

  context.setCurrentIndexBuffer(this);

  const size_t size = getTypeSize(type);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, start * size, sourceCount * size, source);

#if WENDY_DEBUG
  checkGL("Error during copy to index buffer");
#endif
}

void IndexBuffer::copyTo(void* target, size_t targetCount, size_t start)
{
  if (locked)
  {
    logError("Cannot copy data from locked index buffer");
    return;
  }

  if (start + targetCount > count)
  {
    logError("Too many indices requested from index buffer");
    return;
  }

  context.setCurrentIndexBuffer(this);

  const size_t size = getTypeSize(type);
  glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, start * size, targetCount * size, target);

#if WENDY_DEBUG
  checkGL("Error during copy from index buffer");
#endif
}

IndexType IndexBuffer::getType() const
{
  return type;
}

Usage IndexBuffer::getUsage() const
{
  return usage;
}

size_t IndexBuffer::getCount() const
{
  return count;
}

size_t IndexBuffer::getSize() const
{
  return count * getTypeSize(type);
}

Ref<IndexBuffer> IndexBuffer::create(Context& context,
                                     size_t count,
                                     IndexType type,
                                     Usage usage)
{
  Ref<IndexBuffer> buffer(new IndexBuffer(context));
  if (!buffer->init(count, type, usage))
    return NULL;

  return buffer;
}

size_t IndexBuffer::getTypeSize(IndexType type)
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

IndexBuffer::IndexBuffer(Context& initContext):
  context(initContext),
  locked(false),
  type(INDEX_UINT8),
  usage(USAGE_STATIC),
  bufferID(0),
  count(0)
{
}

IndexBuffer::IndexBuffer(const IndexBuffer& source):
  context(source.context)
{
  panic("Index buffers may not be copied");
}

bool IndexBuffer::init(size_t initCount, IndexType initType, Usage initUsage)
{
  type = initType;
  usage = initUsage;
  count = initCount;

  glGenBuffers(1, &bufferID);

  context.setCurrentIndexBuffer(this);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               count * getTypeSize(type),
               NULL,
               convertToGL(usage));

  if (!checkGL("Error during creation of index buffer of element size %u",
               (uint) getTypeSize(type)))
  {
    context.setCurrentIndexBuffer(NULL);
    return false;
  }

  if (Stats* stats = context.getStats())
    stats->addIndexBuffer(getSize());

  return true;
}

IndexBuffer& IndexBuffer::operator = (const IndexBuffer& source)
{
  panic("Index buffers may not be assigned");
}

///////////////////////////////////////////////////////////////////////

VertexRange::VertexRange():
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
  count = vertexBuffer->getCount();
}

VertexRange::VertexRange(VertexBuffer& initVertexBuffer,
                         size_t initStart,
                         size_t initCount):
  vertexBuffer(&initVertexBuffer),
  start(initStart),
  count(initCount)
{
  assert(vertexBuffer->getCount() >= start + count);
}

void* VertexRange::lock(LockType type) const
{
  if (!vertexBuffer || count == 0)
  {
    logError("Cannot lock empty vertex buffer range");
    return NULL;
  }

  uint8* vertices = (uint8*) vertexBuffer->lock(type);
  if (!vertices)
    return NULL;

  return vertices + start * vertexBuffer->getFormat().getSize();
}

void VertexRange::unlock() const
{
  if (!vertexBuffer)
  {
    logError("Cannot unlock non-locked vertex buffer");
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

VertexBuffer* VertexRange::getVertexBuffer() const
{
  return vertexBuffer;
}

size_t VertexRange::getStart() const
{
  return start;
}

size_t VertexRange::getCount() const
{
  return count;
}

///////////////////////////////////////////////////////////////////////

IndexRange::IndexRange():
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
  count = indexBuffer->getCount();
}

IndexRange::IndexRange(IndexBuffer& initIndexBuffer,
                       size_t initStart,
                       size_t initCount):
  indexBuffer(&initIndexBuffer),
  start(initStart),
  count(initCount)
{
  assert(indexBuffer->getCount() >= start + count);
}

void* IndexRange::lock(LockType type) const
{
  if (!indexBuffer || count == 0)
  {
    logError("Cannot lock empty index buffer range");
    return NULL;
  }

  uint8* indices = (uint8*) indexBuffer->lock(type);
  if (!indices)
    return NULL;

  return indices + start * IndexBuffer::getTypeSize(indexBuffer->getType());
}

void IndexRange::unlock() const
{
  if (!indexBuffer)
  {
    logError("Cannot unlock non-locked index buffer");
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

IndexBuffer* IndexRange::getIndexBuffer() const
{
  return indexBuffer;
}

size_t IndexRange::getStart() const
{
  return start;
}

size_t IndexRange::getCount() const
{
  return count;
}

///////////////////////////////////////////////////////////////////////

PrimitiveRange::PrimitiveRange():
  type(TRIANGLE_LIST),
  vertexBuffer(NULL),
  indexBuffer(NULL),
  start(0),
  count(0),
  base(0)
{
}

PrimitiveRange::PrimitiveRange(PrimitiveType initType,
                               VertexBuffer& initVertexBuffer):
  type(initType),
  vertexBuffer(&initVertexBuffer),
  indexBuffer(NULL),
  start(0),
  count(0),
  base(0)
{
  count = vertexBuffer->getCount();
}

PrimitiveRange::PrimitiveRange(PrimitiveType initType,
                               const VertexRange& vertexRange):
  type(initType),
  vertexBuffer(NULL),
  indexBuffer(NULL),
  start(0),
  count(0),
  base(0)
{
  vertexBuffer = vertexRange.getVertexBuffer();
  start = vertexRange.getStart();
  count = vertexRange.getCount();
}

PrimitiveRange::PrimitiveRange(PrimitiveType initType,
                               VertexBuffer& initVertexBuffer,
                               IndexBuffer& initIndexBuffer,
                               size_t initBase):
  type(initType),
  vertexBuffer(&initVertexBuffer),
  indexBuffer(&initIndexBuffer),
  start(0),
  count(0),
  base(initBase)
{
  count = indexBuffer->getCount();
}

PrimitiveRange::PrimitiveRange(PrimitiveType initType,
                               VertexBuffer& initVertexBuffer,
                               const IndexRange& indexRange,
                               size_t initBase):
  type(initType),
  vertexBuffer(&initVertexBuffer),
  indexBuffer(NULL),
  start(0),
  count(0),
  base(initBase)
{
  indexBuffer = indexRange.getIndexBuffer();
  start = indexRange.getStart();
  count = indexRange.getCount();
}

PrimitiveRange::PrimitiveRange(PrimitiveType initType,
                               VertexBuffer& initVertexBuffer,
                               size_t initStart,
                               size_t initCount,
                               size_t initBase):
  type(initType),
  vertexBuffer(&initVertexBuffer),
  indexBuffer(NULL),
  start(initStart),
  count(initCount),
  base(initBase)
{
}

PrimitiveRange::PrimitiveRange(PrimitiveType initType,
                               VertexBuffer& initVertexBuffer,
                               IndexBuffer& initIndexBuffer,
                               size_t initStart,
                               size_t initCount,
                               size_t initBase):
  type(initType),
  vertexBuffer(&initVertexBuffer),
  indexBuffer(&initIndexBuffer),
  start(initStart),
  count(initCount),
  base(initBase)
{
}

bool PrimitiveRange::isEmpty() const
{
  if (vertexBuffer == NULL)
    return true;

  return count == 0;
}

PrimitiveType PrimitiveRange::getType() const
{
  return type;
}

VertexBuffer* PrimitiveRange::getVertexBuffer() const
{
  return vertexBuffer;
}

IndexBuffer* PrimitiveRange::getIndexBuffer() const
{
  return indexBuffer;
}

size_t PrimitiveRange::getStart() const
{
  return start;
}

size_t PrimitiveRange::getCount() const
{
  return count;
}

size_t PrimitiveRange::getBase() const
{
  return base;
}

///////////////////////////////////////////////////////////////////////

template <>
IndexRangeLock<uint8>::IndexRangeLock(IndexRange& initRange):
  range(initRange),
  indices(NULL)
{
  if (IndexBuffer* indexBuffer = range.getIndexBuffer())
  {
    if (indexBuffer->getType() != INDEX_UINT8)
      panic("Index buffer is not of type UINT8");
  }

  indices = (uint8*) range.lock();
  if (!indices)
    panic("Failed to lock index buffer");
}

template <>
IndexRangeLock<uint16>::IndexRangeLock(IndexRange& initRange):
  range(initRange),
  indices(NULL)
{
  if (IndexBuffer* indexBuffer = range.getIndexBuffer())
  {
    if (indexBuffer->getType() != INDEX_UINT16)
      panic("Index buffer is not of type UINT16");
  }

  indices = (uint16*) range.lock();
  if (!indices)
    panic("Failed to lock index buffer");
}

template <>
IndexRangeLock<uint32>::IndexRangeLock(IndexRange& initRange):
  range(initRange),
  indices(NULL)
{
  if (IndexBuffer* indexBuffer = range.getIndexBuffer())
  {
    if (indexBuffer->getType() != INDEX_UINT32)
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

bool Framebuffer::isSRGB() const
{
  return sRGB;
}

void Framebuffer::setSRGB(bool enabled)
{
  if (sRGB == enabled)
    return;

  Framebuffer& previous = context.getCurrentFramebuffer();
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

  sRGB = enabled;

  previous.apply();
}

float Framebuffer::getAspectRatio() const
{
  return getWidth() / (float) getHeight();
}

Context& Framebuffer::getContext() const
{
  return context;
}

Framebuffer::Framebuffer(Context& initContext):
  context(initContext),
  sRGB(false)
{
}

Framebuffer::Framebuffer(const Framebuffer& source):
  context(source.context)
{
  panic("Framebuffers may not be copied");
}

Framebuffer& Framebuffer::operator = (const Framebuffer& source)
{
  panic("Framebuffers may not be assigned");
}

///////////////////////////////////////////////////////////////////////

uint DefaultFramebuffer::getColorBits() const
{
  return colorBits;
}

uint DefaultFramebuffer::getDepthBits() const
{
  return depthBits;
}

uint DefaultFramebuffer::getStencilBits() const
{
  return stencilBits;
}

uint DefaultFramebuffer::getWidth() const
{
  return getContext().getWindow().getWidth();
}

uint DefaultFramebuffer::getHeight() const
{
  return getContext().getWindow().getHeight();
}

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

///////////////////////////////////////////////////////////////////////

TextureFramebuffer::~TextureFramebuffer()
{
  if (bufferID)
    glDeleteFramebuffers(1, &bufferID);
}

uint TextureFramebuffer::getWidth() const
{
  uint width = 0;

  for (size_t i = 0;  i < 5;  i++)
  {
    if (images[i])
    {
      if (width && width != images[i]->getWidth())
        return 0;

      width = images[i]->getWidth();
    }
  }

  return width;
}

uint TextureFramebuffer::getHeight() const
{
  uint height = 0;

  for (size_t i = 0;  i < 5;  i++)
  {
    if (images[i])
    {
      if (height && height != images[i]->getHeight())
        return 0;

      height = images[i]->getHeight();
    }
  }

  return height;
}

TextureImage* TextureFramebuffer::getColorBuffer() const
{
  return images[COLOR_BUFFER0];
}

TextureImage* TextureFramebuffer::getDepthBuffer() const
{
  return images[DEPTH_BUFFER];
}

TextureImage* TextureFramebuffer::getBuffer(Attachment attachment) const
{
  return images[attachment];
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
    const Limits& limits = context.getLimits();
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

  Framebuffer& previous = context.getCurrentFramebuffer();
  apply();

  if (images[attachment])
    images[attachment]->detach(convertToGL(attachment));

  images[attachment] = newImage;

  if (images[attachment])
    images[attachment]->attach(convertToGL(attachment), z);

  previous.apply();
  return true;
}

Ref<TextureFramebuffer> TextureFramebuffer::create(Context& context)
{
  Ref<TextureFramebuffer> framebuffer(new TextureFramebuffer(context));
  if (!framebuffer->init())
    return NULL;

  return framebuffer;
}

TextureFramebuffer::TextureFramebuffer(Context& context):
  Framebuffer(context),
  bufferID(0)
{
}

bool TextureFramebuffer::init()
{
  glGenFramebuffers(1, &bufferID);

#if WENDY_DEBUG
  if (!checkGL("Error during image framebuffer creation"))
    return false;
#endif

  return true;
}

void TextureFramebuffer::apply() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, bufferID);

  GLenum enables[5];
  size_t count = 0;

  for (size_t i = 0;  i < sizeof(enables) / sizeof(enables[0]);  i++)
  {
    Attachment attachment = (Attachment) i;

    if (images[i] && isColorAttachment(attachment))
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
