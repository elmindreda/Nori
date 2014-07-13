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

#pragma once

#include <nori/Core.hpp>
#include <nori/Pixel.hpp>
#include <nori/Vertex.hpp>

namespace nori
{

class RenderContext;

/*! @brief Primitive type enumeration.
 */
enum PrimitiveMode
{
  POINT_LIST,
  LINE_LIST,
  LINE_STRIP,
  LINE_LOOP,
  TRIANGLE_LIST,
  TRIANGLE_STRIP,
  TRIANGLE_FAN
};

enum BufferType
{
  VERTEX_BUFFER,
  INDEX_BUFFER
};

/*! Index buffer element type enumeration.
 */
enum IndexType
{
  /*! Indices are 8-bit unsigned integers.
   */
  INDEX_UINT8,
  /*! Indices are 16-bit unsigned integers.
   */
  INDEX_UINT16,
  /*! Indices are 32-bit unsigned integers.
   */
  INDEX_UINT32,
  NO_INDICES
};

/*! Index buffer usage hint enumeration.
 */
enum BufferUsage
{
  /*! Data will be specified once and used many times.
   */
  USAGE_STATIC,
  /*! Data will be specified once and used a few times.
   */
  USAGE_STREAM,
  /*! Data will be repeatedly respecified and re-used.
   */
  USAGE_DYNAMIC
};

size_t getIndexTypeSize(IndexType type);

/*! @brief Vertex buffer.
 */
class Buffer
{
  friend class VertexArray;
  friend class RenderContext;
public:
  /*! Destructor.
   */
  ~Buffer();
  /*! Discards the current data.
   */
  void discard();
  /*! Copies the specified data into this buffer, starting at the
   *  specified offset.
   *  @param[in] source The base address of the source data.
   *  @param[in] size The size, in bytes, of the data to copy.
   *  @param[in] offset The offset, in bytes, of the data to copy.
   */
  void copyFrom(const void* source, size_t size, size_t offset = 0);
  /*! Copies the specified data from this buffer, starting at the specified
   *  offset.
   *  @param[in,out] target The base address of the destination buffer.
   *  @param[in] size The size, in bytes, of the data to copy.
   *  @param[in] offset The offset, in bytes, of the data to copy.
   */
  void copyTo(void* target, size_t size, size_t start = 0);
  /*! @return The size, in bytes, of the data in this vertex buffer.
   */
  size_t size() const { return m_size; }
  static std::unique_ptr<Buffer> create(RenderContext& context,
                                        BufferType type,
                                        size_t count,
                                        BufferUsage usage);
protected:
  Buffer(RenderContext& context, BufferType type);
private:
  Buffer(const Buffer&) = delete;
  bool init(size_t size, BufferUsage usage);
  Buffer& operator = (const Buffer&) = delete;
  RenderContext& m_context;
  BufferType m_type;
  uint m_bufferID;
  size_t m_size;
  BufferUsage m_usage;
};

/*! @brief Buffer range.
 *
 *  This class represents a contigous range of a buffer object.
 */
class BufferRange
{
public:
  /*! Constructor.
   */
  BufferRange();
  /*! Constructor.
   */
  BufferRange(Buffer& buffer);
  /*! Constructor.
   */
  BufferRange(Buffer& buffer, size_t size, size_t offset = 0);
  /*! Copies the specified data into this buffer range.
   *  @param[in] source The base address of the source data.
   */
  void copyFrom(const void* source);
  /*! Copies the specified number of bytes from this buffer range.
   *  @param[in,out] target The base address of the destination buffer.
   */
  void copyTo(void* target);
  bool isEmpty() const;
  /*! The buffer to use.
   */
  Buffer* buffer;
  /*! The size, in bytes, of this range.
   */
  size_t size;
  /*! The offset, in bytes, of this range within the buffer.
   */
  size_t offset;
};

/*! @brief Geometric primitive range.
 */
class PrimitiveRange
{
public:
  /*! Creates an empty primitive range.
   */
  PrimitiveRange();
  PrimitiveRange(PrimitiveMode mode,
                 IndexType type,
                 size_t start,
                 size_t count,
                 size_t base = 0);
  PrimitiveMode mode;
  IndexType type;
  size_t start;
  size_t count;
  size_t base;
};

/*! @brief Framebuffer.
 *
 *  This class represents a render target, i.e. a framebuffer.
 */
class Framebuffer : public RefObject
{
  friend class RenderContext;
  friend class TextureFramebuffer;
public:
  /*! Destructor.
   */
  virtual ~Framebuffer();
  Ref<Image> data() const;
  /*! @return @c true if this framebuffer uses sRGB encoding, or @c false
   *  otherwise.
   */
  bool isSRGB() const { return m_sRGB; }
  /*! Sets whether this framebuffer uses sRGB encoding.
   */
  void setSRGB(bool enabled);
  /*! @return The width, in pixels, of this framebuffer.
   */
  virtual uint width() const = 0;
  /*! @return The height, in pixels, of this framebuffer.
   */
  virtual uint height() const = 0;
  /*! @return The aspect ratio of the dimensions, in pixels, of this framebuffer.
   */
  float aspectRatio() const { return width() / (float) height(); }
  /*! @return The context within which this framebuffer was created.
   */
  RenderContext& context() const { return m_context; }
protected:
  /*! Constructor.
   */
  Framebuffer(RenderContext& context);
  /*! Called when this framebuffer is to be made current.
   */
  virtual void apply() const = 0;
private:
  Framebuffer(const Framebuffer&) = delete;
  Framebuffer& operator = (const Framebuffer&) = delete;
  RenderContext& m_context;
  bool m_sRGB;
};

/*! @brief %Framebuffer for rendering to the screen.
 */
class WindowFramebuffer : public Framebuffer
{
  friend class RenderContext;
public:
  /*! @return The default framebuffer color depth, in bits.
   */
  uint colorBits() const { return m_colorBits; }
  /*! @return The default framebuffer depth-buffer depth, in bits.
   */
  uint depthBits() const { return m_depthBits; }
  /*! @return The default framebuffer stencil buffer depth, in bits.
   */
  uint stencilBits() const { return m_stencilBits; }
  uint samples() const { return m_samples; }
  uint width() const;
  uint height() const;
private:
  WindowFramebuffer(RenderContext& context);
  void apply() const;
  uint m_colorBits;
  uint m_depthBits;
  uint m_stencilBits;
  uint m_samples;
};

/*! @brief %Framebuffer for rendering to images.
 */
class TextureFramebuffer : public Framebuffer
{
public:
  /*! Framebuffer image attachment point enumeration.
   */
  enum Attachment
  {
    /*! The first (default) color buffer, referenced by @c gl_FragColor or @c
     *  gl_FragData[0].
     */
    COLOR_BUFFER0,
    /*! The second color buffer, referenced in GLSL by @c gl_FragData[1].
     */
    COLOR_BUFFER1,
    /*! The third color buffer, referenced in GLSL by @c gl_FragData[2].
     */
    COLOR_BUFFER2,
    /*! The fourth color buffer, referenced in GLSL by @c gl_FragData[3].
     */
    COLOR_BUFFER3,
    /*! The depth buffer, referenced in GLSL by @c gl_FragDepth.
     */
    DEPTH_BUFFER
  };
  /*! Destructor.
   */
  ~TextureFramebuffer();
  /*! @copydoc Framebuffer::width
   */
  uint width() const;
  /*! @copydoc Framebuffer::height
   */
  uint height() const;
  /*! @return The image attached to the TextureFramebuffer::COLOR_BUFFER0
   *  attachment point, or @c nullptr if no image is attached to it.
   */
  Texture* colorBuffer() const { return m_textures[COLOR_BUFFER0]; }
  /*! @return The image attached to the TextureFramebuffer::DEPTH_BUFFER
   *  attachment point, or @c nullptr if no image is attached to it.
   */
  Texture* depthBuffer() const { return m_textures[DEPTH_BUFFER]; }
  /*! @return The image attached to the specified attachment point, or @c
   *  nullptr if no image is attached to it.
   */
  Texture* buffer(Attachment attachment) const;
  /*! Sets the image to use as the default color buffer for this framebuffer.
   *  @param[in] newTexture The desired image, or @c nullptr to detach the
   *  currently set image.
   *  @return @c true if this framebuffer is complete, or @c false otherwise.
   */
  bool setColorBuffer(Texture* newTexture,
                      const TextureImage& image = TextureImage(),
                      uint z = 0);
  /*! sets the image to use as the depth buffer for this framebuffer.
   *  @param[in] newTexture The desired image, or @c nullptr to detach the
   *  currently set image.
   *  @return @c true if this framebuffer is complete, or @c false otherwise.
   */
  bool setDepthBuffer(Texture* newTexture,
                      const TextureImage& image = TextureImage(),
                      uint z = 0);
  /*! sets the image to use for the specified attachment point of this
   *  framebuffer.
   *  @param[in] newTexture The desired image, or @c nullptr to detach the
   *  currently set image.
   *  @param[in] z The desired Z slice of the specified image to use.  This
   *  only applies to 3D textures.
   *  @return @c true if this framebuffer is complete, or @c false otherwise.
   */
  bool setBuffer(Attachment attachment,
                 Texture* newTexture,
                 const TextureImage& image = TextureImage(),
                 uint z = 0);
  /*! Creates an image framebuffer within the specified context.
   */
  static Ref<TextureFramebuffer> create(RenderContext& context);
private:
  TextureFramebuffer(RenderContext& context);
  bool init();
  void apply() const;
  uint m_bufferID;
  Ref<Texture> m_textures[5];
};

} /*namespace nori*/

