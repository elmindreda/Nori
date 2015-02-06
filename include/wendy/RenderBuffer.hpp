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

#pragma once

#include <wendy/Core.hpp>
#include <wendy/Pixel.hpp>
#include <wendy/Vertex.hpp>

namespace wendy
{

class RenderContext;

/*! @brief Primitive type enumeration.
 */
enum PrimitiveType
{
  POINT_LIST,
  LINE_LIST,
  LINE_STRIP,
  LINE_LOOP,
  TRIANGLE_LIST,
  TRIANGLE_STRIP,
  TRIANGLE_FAN
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
  INDEX_UINT32
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

/*! @brief Vertex buffer.
 */
class VertexBuffer : public RefObject
{
  friend class RenderContext;
public:
  /*! Destructor.
   */
  ~VertexBuffer();
  /*! Discards the current data.
   */
  void discard();
  /*! Copies the specified data into this vertex buffer, starting at the
   *  specified offset.
   *  @param[in] source The base address of the source data.
   *  @param[in] count The number of vertices to copy.
   *  @param[in] start The index of the first vertex to be written to.
   */
  void copyFrom(const void* source, size_t count, size_t start = 0);
  /*! Copies the specified number of bytes from this vertex buffer, starting
   *  at the specified offset.
   *  @param[in,out] target The base address of the destination buffer.
   *  @param[in] count The number of vertices to copy.
   *  @param[in] start The index of the first vertex to read from.
   */
  void copyTo(void* target, size_t count, size_t start = 0);
  /*! @return The usage hint of this vertex buffer.
   */
  BufferUsage usage() const;
  /*! @return The format of this vertex buffer.
   */
  const VertexFormat& format() const { return m_format; }
  /*! @return The number of vertices in this vertex buffer.
   */
  size_t count() const { return m_count; }
  /*! @return The size, in bytes, of the data in this vertex buffer.
   */
  size_t size() const { return m_count * m_format.size(); }
  /*! Creates a vertex buffer with the specified properties.
   *  @param count The desired number of vertices.
   *  @param format The desired format of the vertices.
   *  @param usage The desired usage hint.
   *  @return The newly created vertex buffer, or @c nullptr if an error occurred.
   */
  static Ref<VertexBuffer> create(RenderContext& context,
                                  size_t count,
                                  const VertexFormat& format,
                                  BufferUsage usage);
private:
  VertexBuffer(RenderContext& context);
  VertexBuffer(const VertexBuffer&) = delete;
  bool init(const VertexFormat& format, size_t count, BufferUsage usage);
  VertexBuffer& operator = (const VertexBuffer&) = delete;
  RenderContext& m_context;
  VertexFormat m_format;
  uint m_bufferID;
  size_t m_count;
  BufferUsage m_usage;
};

/*! @brief Index (or element) buffer.
 */
class IndexBuffer : public RefObject
{
  friend class RenderContext;
public:
  /*! Destructor.
   */
  ~IndexBuffer();
  /*! Copies the specified data into this index buffer, starting at the
   *  specified offset.
   *  @param[in] source The base address of the source data.
   *  @param[in] count The number of indices to copy.
   *  @param[in] start The index of the first index to be written to.
   */
  void copyFrom(const void* source, size_t count, size_t start = 0);
  /*! Copies the specified number of bytes from this index buffer, starting
   *  at the specified offset.
   *  @param[in,out] target The base address of the destination buffer.
   *  @param[in] count The number of indices to copy.
   *  @param[in] start The index of the first index to read from.
   */
  void copyTo(void* target, size_t count, size_t start = 0);
  /*! @return The type of the index elements in this index buffer.
   */
  IndexType type() const { return m_type; }
  /*! @return The usage hint of this index buffer.
   */
  BufferUsage usage() const { return m_usage; }
  /*! @return The number of index elements in this index buffer.
   */
  size_t count() const { return m_count; }
  /*! @return The size, in bytes, of the data in this index buffer.
   */
  size_t size() const;
  /*! Creates an index buffer with the specified properties.
   *  @param count The desired number of index elements.
   *  @param type The desired type of the index elements.
   *  @param usage The desired usage hint.
   *  @return The newly created index buffer, or @c nullptr if an error
   *  occurred.
   */
  static Ref<IndexBuffer> create(RenderContext& context,
                                 size_t count,
                                 IndexType type,
                                 BufferUsage usage);
  /*! @return The size, in bytes, of the specified element type.
   */
  static size_t typeSize(IndexType type);
private:
  IndexBuffer(RenderContext& context);
  IndexBuffer(const IndexBuffer&) = delete;
  bool init(size_t count, IndexType type, BufferUsage usage);
  IndexBuffer& operator = (const IndexBuffer&) = delete;
  RenderContext& m_context;
  IndexType m_type;
  BufferUsage m_usage;
  uint m_bufferID;
  size_t m_count;
};

/*! @brief Vertex buffer range.
 *
 *  This class represents a contigous range of an vertex buffer object.
 *  This is useful for allocation schemes where many smaller objects
 *  are fitted into a single vertex buffer for performance reasons.
 */
class VertexRange
{
public:
  /*! Constructor.
   */
  VertexRange();
  /*! Constructor.
   */
  VertexRange(VertexBuffer& vertexBuffer);
  /*! Constructor.
   */
  VertexRange(VertexBuffer& vertexBuffer, size_t start, size_t count);
  /*! Copies the specified data into this vertex range.
   *  @param[in] source The base address of the source data.
   */
  void copyFrom(const void* source);
  /*! Copies the specified number of bytes from this vertex range.
   *  @param[in,out] target The base address of the destination buffer.
   */
  void copyTo(void* target);
  /*! @return @c true if this vertex range is empty, or @c false otherwise.
   */
  bool isEmpty() const { return m_count == 0; }
  /*! @return The vertex buffer underlying this vertex range.
   */
  VertexBuffer* vertexBuffer() const { return m_buffer; }
  /*! @return The index of the first vertex in this vertex range.
   */
  size_t start() const { return m_start; }
  /*! @return The number of vertices in this vertex range.
   */
  size_t count() const { return m_count; }
private:
  VertexBuffer* m_buffer;
  size_t m_start;
  size_t m_count;
};

/*! @brief Index buffer range.
 *
 *  This class represents a contigous range of an index buffer object.
 *  This is useful for allocation schemes where many smaller objects
 *  are fitted into a single index buffer for performance reasons.
 */
class IndexRange
{
public:
  /*! Creates an empty range not referencing any index buffer.
   */
  IndexRange();
  /*! Creates a range spanning the entire specified index buffer.
   *  @param[in] indexBuffer The index buffer this range will refer to.
   */
  IndexRange(IndexBuffer& buffer);
  /*! Creates the specified range within the specified index buffer.
   */
  IndexRange(IndexBuffer& buffer, size_t start, size_t count);
  /*! Copies the specified data into this index range.
   *  @param[in] source The base address of the source data.
   */
  void copyFrom(const void* source);
  /*! Copies the specified number of bytes from this index range.
   *  @param[in,out] target The base address of the destination buffer.
   */
  void copyTo(void* target);
  /*! @return The index buffer underlying this index range.
   */
  IndexBuffer* indexBuffer() const { return m_buffer; }
  /*! @return The index of the first index in this index range.
   */
  size_t start() const { return m_start; }
  /*! @return The number of indices in this index range.
   */
  size_t count() const { return m_count; }
private:
  IndexBuffer* m_buffer;
  size_t m_start;
  size_t m_count;
};

/*! @brief Geometric primitive range.
 */
class PrimitiveRange
{
public:
  /*! Creates an empty primitive range not referencing any buffers.
   */
  PrimitiveRange();
  /*! Creates a primitive range of the specified type, using the entire
   *  specified vertex buffer.
   */
  PrimitiveRange(PrimitiveType type, VertexBuffer& vertexBuffer);
  /*! Creates a primitive range of the specified type, using the specified
   *  range of vertices.
   */
  PrimitiveRange(PrimitiveType type, const VertexRange& vertexRange);
  /*! Creates a primitive range of the specified type, using the entire
   *  specified index buffer to reference vertices in the specified vertex
   *  buffer.
   */
  PrimitiveRange(PrimitiveType type,
                 VertexBuffer& vertexBuffer,
                 IndexBuffer& indexBuffer,
                 size_t base = 0);
  /*! Creates a primitive range of the specified type, using the specified
   *  range of indices to refer to vertices in the specified vertex buffer.
   */
  PrimitiveRange(PrimitiveType type,
                 VertexBuffer& vertexBuffer,
                 const IndexRange& indexRange,
                 size_t base = 0);
  /*! Creates a primitive range of the specified type, using the specified
   *  range of the specified vertex buffer.
   */
  PrimitiveRange(PrimitiveType type,
                 VertexBuffer& vertexBuffer,
                 size_t start,
                 size_t count,
                 size_t base = 0);
  /*! Creates a primitive range of the specified type, using the specified
   *  range of the specified index buffer to reference vertices in the
   *  specified vertex buffer.
   */
  PrimitiveRange(PrimitiveType type,
                 VertexBuffer& vertexBuffer,
                 IndexBuffer& indexBuffer,
                 size_t start,
                 size_t count,
                 size_t base = 0);
  /*! @return @c true if this primitive range contains zero primitives,
   *  otherwise @c false.
   */
  bool isEmpty() const;
  /*! @return The type of primitives in this range.
   */
  PrimitiveType type() const { return m_type; }
  /*! @return The vertex buffer used by this primitive range.
   */
  VertexBuffer* vertexBuffer() const { return m_vertexBuffer; }
  /*! @return The index buffer used by this primitive range, or @c nullptr if no
   *  index buffer is used.
   */
  IndexBuffer* indexBuffer() const { return m_indexBuffer; }
  size_t start() const { return m_start; }
  size_t count() const { return m_count; }
  size_t base() const { return m_base; }
private:
  PrimitiveType m_type;
  VertexBuffer* m_vertexBuffer;
  IndexBuffer* m_indexBuffer;
  size_t m_start;
  size_t m_count;
  size_t m_base;
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

} /*namespace wendy*/

