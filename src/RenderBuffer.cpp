///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#include <wendy/Texture.hpp>
#include <wendy/RenderBuffer.hpp>
#include <wendy/Program.hpp>
#include <wendy/RenderContext.hpp>

#include <GREG/greg.h>

#include <internal/OpenGL.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

GLenum convertToGL(BufferUsage usage)
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
  if (m_bufferID)
    glDeleteBuffers(1, &m_bufferID);

  if (RenderStats* stats = m_context.stats())
    stats->removeVertexBuffer(size());
}

void VertexBuffer::discard()
{
  m_context.setCurrentVertexBuffer(this);

  glBufferData(GL_ARRAY_BUFFER,
               m_count * m_format.size(),
               nullptr,
               convertToGL(m_usage));

#if WENDY_DEBUG
  checkGL("Error during vertex buffer discard");
#endif
}

void VertexBuffer::copyFrom(const void* source, size_t sourceCount, size_t start)
{
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

Ref<VertexBuffer> VertexBuffer::create(RenderContext& context,
                                       size_t count,
                                       const VertexFormat& format,
                                       BufferUsage usage)
{
  Ref<VertexBuffer> buffer(new VertexBuffer(context));
  if (!buffer->init(format, count, usage))
    return nullptr;

  return buffer;
}

VertexBuffer::VertexBuffer(RenderContext& context):
  m_context(context),
  m_bufferID(0),
  m_count(0),
  m_usage(USAGE_STATIC)
{
}

bool VertexBuffer::init(const VertexFormat& format, size_t count, BufferUsage usage)
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

  if (RenderStats* stats = m_context.stats())
    stats->addVertexBuffer(size());

  return true;
}

///////////////////////////////////////////////////////////////////////

IndexBuffer::~IndexBuffer()
{
  if (m_bufferID)
    glDeleteBuffers(1, &m_bufferID);

  if (RenderStats* stats = m_context.stats())
    stats->removeIndexBuffer(size());
}

void IndexBuffer::copyFrom(const void* source, size_t sourceCount, size_t start)
{
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

Ref<IndexBuffer> IndexBuffer::create(RenderContext& context,
                                     size_t count,
                                     IndexBufferType type,
                                     BufferUsage usage)
{
  Ref<IndexBuffer> buffer(new IndexBuffer(context));
  if (!buffer->init(count, type, usage))
    return nullptr;

  return buffer;
}

size_t IndexBuffer::typeSize(IndexBufferType type)
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

IndexBuffer::IndexBuffer(RenderContext& context):
  m_context(context),
  m_type(INDEX_UINT8),
  m_usage(USAGE_STATIC),
  m_bufferID(0),
  m_count(0)
{
}

bool IndexBuffer::init(size_t count, IndexBufferType type, BufferUsage usage)
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

  if (RenderStats* stats = m_context.stats())
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

Framebuffer::~Framebuffer()
{
}

Ref<Image> Framebuffer::data() const
{
  Ref<Image> image = Image::create(m_context.cache(),
                                   PixelFormat::RGB8,
                                   width(), height());
  if (!image)
    return nullptr;

  Framebuffer& previous = m_context.currentFramebuffer();
  apply();

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, image->width(), image->height(),
               GL_RGB, GL_UNSIGNED_BYTE,
               image->pixels());

  previous.apply();

  if (!checkGL("Error when reading framebuffer data"))
    return nullptr;

  return image;
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

Framebuffer::Framebuffer(RenderContext& context):
  m_context(context),
  m_sRGB(false)
{
}

///////////////////////////////////////////////////////////////////////

DefaultFramebuffer::DefaultFramebuffer(RenderContext& context):
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
  return context().window().height();
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
    const RenderLimits& limits = m_context.limits();
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

Ref<TextureFramebuffer> TextureFramebuffer::create(RenderContext& context)
{
  Ref<TextureFramebuffer> framebuffer(new TextureFramebuffer(context));
  if (!framebuffer->init())
    return nullptr;

  return framebuffer;
}

TextureFramebuffer::TextureFramebuffer(RenderContext& context):
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
  GLsizei count = 0;

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

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
