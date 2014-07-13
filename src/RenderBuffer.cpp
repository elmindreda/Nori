///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

#include <nori/Config.hpp>

#include <nori/Texture.hpp>
#include <nori/RenderBuffer.hpp>
#include <nori/Program.hpp>
#include <nori/RenderContext.hpp>

#include <GREG/greg.h>

#include <internal/OpenGL.hpp>

namespace nori
{

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

size_t getIndexTypeSize(IndexType type)
{
  switch (type)
  {
    case INDEX_UINT8:
      return 1;
    case INDEX_UINT16:
      return 2;
    case INDEX_UINT32:
      return 4;
    default:
      panic("Invalid index type");
  }
}

Buffer::~Buffer()
{
  if (m_bufferID)
    glDeleteBuffers(1, &m_bufferID);

  if (RenderStats* stats = m_context.stats())
    stats->removeVertexBuffer(m_size);
}

void Buffer::discard()
{
  m_context.setVertexArray(0);
  glBindBuffer(convertToGL(m_type), m_bufferID);
  glBufferData(convertToGL(m_type), m_size, nullptr, convertToGL(m_usage));

#if WENDY_DEBUG
  checkGL("Error during buffer discard");
#endif
}

void Buffer::copyFrom(const void* source, size_t size, size_t offset)
{
  if (offset + size > m_size)
  {
    logError("Too much data submitted to buffer");
    return;
  }

  m_context.setVertexArray(0);
  glBindBuffer(convertToGL(m_type), m_bufferID);
  glBufferSubData(convertToGL(m_type), offset, size, source);

#if WENDY_DEBUG
  checkGL("Error during copy to buffer");
#endif
}

void Buffer::copyTo(void* target, size_t size, size_t offset)
{
  if (offset + size > m_size)
  {
    logError("Too much data requested from buffer");
    return;
  }

  m_context.setVertexArray(0);
  glBindBuffer(convertToGL(m_type), m_bufferID);
  glGetBufferSubData(convertToGL(m_type), offset, size, target);

#if WENDY_DEBUG
  checkGL("Error during copy from buffer");
#endif
}

std::unique_ptr<Buffer> Buffer::create(RenderContext& context,
                                       BufferType type,
                                       size_t size,
                                       BufferUsage usage)
{
  std::unique_ptr<Buffer> buffer(new Buffer(context, type));
  if (!buffer->init(size, usage))
    return nullptr;

  return buffer;
}

Buffer::Buffer(RenderContext& context, BufferType type):
  m_context(context),
  m_type(type),
  m_bufferID(0),
  m_size(0),
  m_usage(USAGE_STATIC)
{
}

bool Buffer::init(size_t size, BufferUsage usage)
{
  m_usage = usage;
  m_size = size;

  m_context.setVertexArray(0);
  glGenBuffers(1, &m_bufferID);
  glBindBuffer(convertToGL(m_type), m_bufferID);
  glBufferData(convertToGL(m_type), m_size, nullptr, convertToGL(m_usage));

  if (!checkGL("Error during creation of buffer"))
    return false;

  if (RenderStats* stats = m_context.stats())
    stats->addVertexBuffer(m_size);

  return true;
}

BufferRange::BufferRange():
  buffer(nullptr),
  size(0),
  offset(0)
{
}

BufferRange::BufferRange(Buffer& buffer):
  buffer(&buffer),
  size(buffer.size()),
  offset(0)
{
}

BufferRange::BufferRange(Buffer& buffer,
                         size_t size,
                         size_t offset):
  buffer(&buffer),
  size(size),
  offset(offset)
{
  assert(buffer.size() >= offset + size);
}

void BufferRange::copyFrom(const void* source)
{
  assert(buffer);
  buffer->copyFrom(source, size, offset);
}

void BufferRange::copyTo(void* target)
{
  assert(buffer);
  buffer->copyTo(target, size, offset);
}

bool BufferRange::isEmpty() const
{
  return buffer == nullptr || size == 0;
}

PrimitiveRange::PrimitiveRange():
  mode(TRIANGLE_LIST),
  type(NO_INDICES),
  start(0),
  count(0),
  base(0)
{
}

PrimitiveRange::PrimitiveRange(PrimitiveMode mode,
                               IndexType type,
                               size_t start,
                               size_t count,
                               size_t base):
  mode(mode),
  type(type),
  start(start),
  count(count),
  base(base)
{
}

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

  Framebuffer& previous = m_context.framebuffer();
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

  Framebuffer& previous = m_context.framebuffer();
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

WindowFramebuffer::WindowFramebuffer(RenderContext& context):
  Framebuffer(context),
  m_colorBits(getInteger(GL_RED_BITS) +
              getInteger(GL_GREEN_BITS) +
              getInteger(GL_BLUE_BITS)),
  m_depthBits(getInteger(GL_DEPTH_BITS)),
  m_stencilBits(getInteger(GL_STENCIL_BITS)),
  m_samples(getInteger(GL_SAMPLES))
{
}

void WindowFramebuffer::apply() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

#if NORI_DEBUG
  checkGL("Error when applying default framebuffer");
#endif
}

uint WindowFramebuffer::width() const
{
  return context().window().width();
}

uint WindowFramebuffer::height() const
{
  return context().window().height();
}

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
    if (m_textures[i])
    {
      if (width && width != m_textures[i]->width())
        return 0;

      width = m_textures[i]->width();
    }
  }

  return width;
}

uint TextureFramebuffer::height() const
{
  uint height = 0;

  for (size_t i = 0;  i < 5;  i++)
  {
    if (m_textures[i])
    {
      if (height && height != m_textures[i]->height())
        return 0;

      height = m_textures[i]->height();
    }
  }

  return height;
}

Texture* TextureFramebuffer::buffer(Attachment attachment) const
{
  return m_textures[attachment];
}

bool TextureFramebuffer::setDepthBuffer(Texture* newTexture, const TextureImage& image, uint z)
{
  return setBuffer(DEPTH_BUFFER, newTexture, image, z);
}

bool TextureFramebuffer::setColorBuffer(Texture* newTexture, const TextureImage& image, uint z)
{
  return setBuffer(COLOR_BUFFER0, newTexture, image, z);
}

bool TextureFramebuffer::setBuffer(Attachment attachment, Texture* newTexture,
                                   const TextureImage& image, uint z)
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

  Framebuffer& previous = m_context.framebuffer();
  apply();

  if (m_textures[attachment])
    m_textures[attachment]->detach(convertToGL(attachment));

  m_textures[attachment] = newTexture;

  if (m_textures[attachment])
    m_textures[attachment]->attach(convertToGL(attachment), image, z);

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

#if NORI_DEBUG
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

    if (m_textures[i] && isColorAttachment(attachment))
      enables[count++] = convertToGL(attachment);
  }

  if (count)
    glDrawBuffers(count, enables);
  else
    glDrawBuffer(GL_NONE);

#if NORI_DEBUG
  checkGL("Error when applying image framebuffer");
#endif
}

} /*namespace nori*/

