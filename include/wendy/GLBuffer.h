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
#ifndef WENDY_GLBUFFER_H
#define WENDY_GLBUFFER_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Pixel.h>
#include <wendy/Vertex.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

class Context;

///////////////////////////////////////////////////////////////////////

/*! @brief Primitive type enumeration.
 *  @ingroup opengl
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

///////////////////////////////////////////////////////////////////////

/*! @brief Memory locking type enumeration.
 *  @ingroup opengl
 */
enum LockType
{
  /*! Requests read-only access.
   */
  LOCK_READ_ONLY,
  /*! Requests write-only access.
   */
  LOCK_WRITE_ONLY,
  /*! Requests read and write access.
   */
  LOCK_READ_WRITE
};

///////////////////////////////////////////////////////////////////////

/*! @brief Vertex buffer.
 *  @ingroup opengl
 */
class VertexBuffer : public RefObject
{
  friend class Context;
public:
  /*! Vertex buffer usage hint enumeration.
   */
  enum Usage
  {
    /*! Data will be specified once and used many times.
     */
    STATIC,
    /*! Data will be specified once and used a few times.
     */
    STREAM,
    /*! Data will be repeatedly respecified and re-used.
     */
    DYNAMIC
  };
  /*! Destructor.
   */
  ~VertexBuffer();
  /*! Locks this vertex buffer for reading and writing.
   *  @return The base address of the vertices.
   */
  void* lock(LockType type = LOCK_WRITE_ONLY);
  /*! Unlocks this vertex buffer, finalizing any changes.
   */
  void unlock();
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
  Usage getUsage() const;
  /*! @return The format of this vertex buffer.
   */
  const VertexFormat& getFormat() const;
  /*! @return The number of vertices in this vertex buffer.
   */
  size_t getCount() const;
  /*! @return The size, in bytes, of the data in this vertex buffer.
   */
  size_t getSize() const;
  /*! Creates a vertex buffer with the specified properties.
   *  @param count The desired number of vertices.
   *  @param format The desired format of the vertices.
   *  @param usage The desired usage hint.
   *  @return The newly created vertex buffer, or @c NULL if an error occurred.
   */
  static Ref<VertexBuffer> create(Context& context,
                                  size_t count,
                                  const VertexFormat& format,
                                  Usage usage);
private:
  VertexBuffer(Context& context);
  VertexBuffer(const VertexBuffer& source);
  VertexBuffer& operator = (const VertexBuffer& source);
  bool init(const VertexFormat& format, size_t count, Usage usage);
  Context& context;
  bool locked;
  VertexFormat format;
  unsigned int bufferID;
  size_t count;
  Usage usage;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Index (or element) buffer.
 *  @ingroup opengl
 */
class IndexBuffer : public RefObject
{
  friend class Context;
public:
  /*! Index buffer element type enumeration.
   */
  enum Type
  {
    /*! Indices are 8-bit unsigned integers.
     */
    UINT8,
    /*! Indices are 16-bit unsigned integers.
     */
    UINT16,
    /*! Indices are 32-bit unsigned integers.
     */
    UINT32
  };
  /*! Index buffer usage hint enumeration.
   */
  enum Usage
  {
    /*! Data will be specified once and used many times.
     */
    STATIC,
    /*! Data will be specified once and used a few times.
     */
    STREAM,
    /*! Data will be repeatedly respecified and re-used.
     */
    DYNAMIC
  };
  /*! Destructor.
   */
  ~IndexBuffer();
  /*! Locks this index buffer for reading and writing.
   *  @param[in] type The desired type of lock.
   *  @return The base address of the index elements.
   */
  void* lock(LockType type = LOCK_WRITE_ONLY);
  /*! Unlocks this index buffer, finalizing any changes.
   */
  void unlock();
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
  Type getType() const;
  /*! @return The usage hint of this index buffer.
   */
  Usage getUsage() const;
  /*! @return The number of index elements in this index buffer.
   */
  size_t getCount() const;
  /*! @return The size, in bytes, of the data in this index buffer.
   */
  size_t getSize() const;
  /*! Creates an index buffer with the specified properties.
   *  @param count The desired number of index elements.
   *  @param type The desired type of the index elements.
   *  @param usage The desired usage hint.
   *  @return The newly created index buffer, or @c NULL if an error occurred.
   */
  static Ref<IndexBuffer> create(Context& context,
                                 size_t count,
                                 Type type,
                                 Usage usage);
  /*! @return The size, in bytes, of the specified element type.
   */
  static size_t getTypeSize(Type type);
private:
  IndexBuffer(Context& context);
  IndexBuffer(const IndexBuffer& source);
  IndexBuffer& operator = (const IndexBuffer& source);
  bool init(size_t count, Type type, Usage usage);
  Context& context;
  bool locked;
  Type type;
  Usage usage;
  unsigned int bufferID;
  size_t count;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Vertex buffer range.
 *  @ingroup opengl
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
  /*! Locks this vertex range into memory and returns its address.
   *  @return The base address of this vertex range, or @c NULL if an error occurred.
   */
  void* lock(LockType type = LOCK_WRITE_ONLY) const;
  /*! Unlocks this vertex range.
   */
  void unlock() const;
  /*! Copies the specified data into this vertex range.
   *  @param[in] source The base address of the source data.
   */
  void copyFrom(const void* source);
  /*! Copies the specified number of bytes from this vertex range.
   *  @param[in,out] target The base address of the destination buffer.
   */
  void copyTo(void* target);
  /*! @return The vertex buffer underlying this vertex range.
   */
  VertexBuffer* getVertexBuffer() const;
  /*! @return The index of the first vertex in this vertex range.
   */
  size_t getStart() const;
  /*! @return The number of vertices in this vertex range.
   */
  size_t getCount() const;
private:
  VertexBuffer* vertexBuffer;
  size_t start;
  size_t count;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Index buffer range.
 *  @ingroup opengl
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
  IndexRange(IndexBuffer& indexBuffer);
  /*! Creates the specified range within the specified index buffer.
   */
  IndexRange(IndexBuffer& indexBuffer, size_t start, size_t count);
  /*! Locks this index range into memory and returns its address.
   *  @param[in] type The desired type of lock.
   *  @return The base address of this index range, or @c NULL if an error occurred.
   */
  void* lock(LockType type = LOCK_WRITE_ONLY) const;
  /*! Unlocks this index range.
   */
  void unlock() const;
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
  IndexBuffer* getIndexBuffer() const;
  /*! @return The index of the first index in this index range.
   */
  size_t getStart() const;
  /*! @return The number of indices in this index range.
   */
  size_t getCount() const;
private:
  IndexBuffer* indexBuffer;
  size_t start;
  size_t count;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Geometric primitive range.
 *  @ingroup opengl
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
                 IndexBuffer& indexBuffer);
  /*! Creates a primitive range of the specified type, using the specified
   *  range of indices to refer to vertices in the specified vertex buffer.
   */
  PrimitiveRange(PrimitiveType type,
                 VertexBuffer& vertexBuffer,
                 const IndexRange& indexRange);
  /*! Creates a primitive range of the specified type, using the specified
   *  range of the specified vertex buffer.
   */
  PrimitiveRange(PrimitiveType type,
                 VertexBuffer& vertexBuffer,
                 size_t start,
                 size_t count);
  /*! Creates a primitive range of the specified type, using the specified
   *  range of the specified index buffer to reference vertices in the
   *  specified vertex buffer.
   */
  PrimitiveRange(PrimitiveType type,
                 VertexBuffer& vertexBuffer,
                 IndexBuffer& indexBuffer,
                 size_t start,
                 size_t count);
  /*! @return @c true if this primitive range contains zero primitives,
   *  otherwise @c false.
   */
  bool isEmpty() const;
  /*! @return The type of primitives in this range.
   */
  PrimitiveType getType() const;
  /*! @return The vertex buffer used by this primitive range.
   */
  VertexBuffer* getVertexBuffer() const;
  /*! @return The index buffer used by this primitive range, or @c NULL if no
   *  index buffer is used.
   */
  IndexBuffer* getIndexBuffer() const;
  size_t getStart() const;
  size_t getCount() const;
private:
  PrimitiveType type;
  VertexBuffer* vertexBuffer;
  IndexBuffer* indexBuffer;
  size_t start;
  size_t count;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Vertex range scoped lock helper template.
 *  @ingroup opengl
 */
template <typename T>
class VertexRangeLock
{
public:
  /*! Constructor.
   *  @param[in] range The vertex range to lock.
   *  @remarks The vertex range must not already be locked.
   *  @remarks The specified vertex range object is copied, not referenced.
   */
  VertexRangeLock(VertexRange& initRange):
    range(initRange),
    vertices(NULL)
  {
    if (VertexBuffer* vertexBuffer = range.getVertexBuffer())
    {
      const VertexFormat& format = vertexBuffer->getFormat();

      if (T::format != format)
      {
        panic("Vertex buffer format \'%s\' does not match range lock format \'%s\'",
              format.asString().c_str(),
              T::format.asString().c_str());
      }
    }

    vertices = (T*) range.lock();
    if (!vertices)
      panic("Failed to lock vertex buffer");
  }
  /*! Destructor.
   *  Releases any lock held.
   */
  ~VertexRangeLock()
  {
    range.unlock();
  }
  /*! @return The base address of the locked vertex range.
   *  @remarks The vertex range is locked the first time this is called.
   */
  operator T* ()
  {
    return vertices;
  }
private:
  VertexRange range;
  T* vertices;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Index range scoped lock helper template.
 *  @ingroup opengl
 */
template <typename T>
class IndexRangeLock
{
public:
  /*! Constructor.
   *  @param[in] range The index range to lock.
   *  @remarks The index range must not already be locked.
   *  @remarks The specified index range object is copied, not referenced.
   */
  IndexRangeLock(IndexRange& initRange):
    range(initRange),
    indices(NULL)
  {
    panic("Invalid index type");
  }
  /*! Destructor.
   *  Releases any lock held.
   */
  ~IndexRangeLock()
  {
    range.unlock();
  }
  /*! @return The base address of the locked index range.
   *  @remarks The index range is locked the first time this is called.
   */
  operator T* ()
  {
    return indices;
  }
private:
  IndexRange range;
  T* indices;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Interface for images.
 *  @ingoup opengl
 */
class Image : public RefObject
{
  friend class ImageFramebuffer;
public:
  virtual ~Image();
  virtual unsigned int getWidth() const = 0;
  virtual unsigned int getHeight() const = 0;
  virtual unsigned int getDepth() const = 0;
  /*! @return The size, in bytes, of the data in this image.
   */
  size_t getSize() const;
  virtual const PixelFormat& getFormat() const = 0;
protected:
  virtual void attach(int attachment, unsigned int z) = 0;
  virtual void detach(int attachment) = 0;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Render buffer.
 *  @ingroup opengl
 */
class RenderBuffer : public Image
{
public:
  /*! Destructor.
   */
  virtual ~RenderBuffer();
  /*! @return The width, in pixels, of this render buffer.
   */
  unsigned int getWidth() const;
  /*! @return The height, in pixels, of this render buffer.
   */
  unsigned int getHeight() const;
  /*! @return The depth, in pixels, of this render buffer.
   *  @remarks This always returns 1 (one).
   */
  unsigned int getDepth() const;
  /*! @return The pixel format of this render buffer.
   */
  const PixelFormat& getFormat() const;
  /*! Creates a render buffer with the specified properties.
   *  @param[in] format The desired pixel format.
   *  @param[in] width The desired width.
   *  @param[in] height The desired height.
   *  @return The newly created render buffer, or @c NULL if an error occurred.
   */
  static Ref<RenderBuffer> create(Context& context,
                                  const PixelFormat& format,
                                  unsigned int width,
                                  unsigned int height);
private:
  RenderBuffer(Context& context);
  bool init(const PixelFormat& format, unsigned int width, unsigned int height);
  void attach(int attachment, unsigned int z);
  void detach(int attachment);
  Context& context;
  unsigned int bufferID;
  unsigned int width;
  unsigned int height;
  PixelFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Framebuffer.
 *  @ingroup opengl
 *
 *  This class represents a render target, i.e. a framebuffer.
 */
class Framebuffer : public RefObject
{
  friend class Context;
  friend class ImageFramebuffer;
public:
  /*! Destructor.
   */
  virtual ~Framebuffer();
  /*! @return @c true if this framebuffer uses sRGB encoding, or @c false
   *  otherwise.
   */
  bool isSRGB() const;
  /*! Sets whether this framebuffer uses sRGB encoding.
   */
  void setSRGB(bool enabled);
  /*! @return The width, in pixels, of this framebuffer.
   */
  virtual unsigned int getWidth() const = 0;
  /*! @return The height, in pixels, of this framebuffer.
   */
  virtual unsigned int getHeight() const = 0;
  /*! @return The aspect ratio of the dimensions, in pixels, of this framebuffer.
   */
  float getAspectRatio() const;
  /*! @return The context within which this framebuffer was created.
   */
  Context& getContext() const;
protected:
  /*! Constructor.
   */
  Framebuffer(Context& context);
  /*! Called when this framebuffer is to be made current.
   */
  virtual void apply() const = 0;
private:
  Framebuffer(const Framebuffer& source);
  Framebuffer& operator = (const Framebuffer& source);
  Context& context;
  bool sRGB;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Framebuffer for rendering to the screen.
 *  @ingroup opengl
 */
class DefaultFramebuffer : public Framebuffer
{
  friend class Context;
public:
  /*! @return The default framebuffer color depth, in bits.
   */
  unsigned int getColorBits() const;
  /*! @return The default framebuffer depth-buffer depth, in bits.
   */
  unsigned int getDepthBits() const;
  /*! @return The default framebuffer stencil buffer depth, in bits.
   */
  unsigned int getStencilBits() const;
  unsigned int getWidth() const;
  unsigned int getHeight() const;
private:
  DefaultFramebuffer(Context& context);
  void apply() const;
  unsigned int width;
  unsigned int height;
  unsigned int colorBits;
  unsigned int depthBits;
  unsigned int stencilBits;
  unsigned int samples;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Framebuffer for rendering to images.
 *  @ingroup opengl
 */
class ImageFramebuffer : public Framebuffer
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
  ~ImageFramebuffer();
  /*! @copydoc Framebuffer::getWidth
   */
  unsigned int getWidth() const;
  /*! @copydoc Framebuffer::getHeight
   */
  unsigned int getHeight() const;
  /*! @return The image attached to the ImageFramebuffer::COLOR_BUFFER0
   *  attachment point, or @c NULL if no image is attached to it.
   */
  Image* getColorBuffer() const;
  /*! @return The image attached to the ImageFramebuffer::DEPTH_BUFFER
   *  attachment point, or @c NULL if no image is attached to it.
   */
  Image* getDepthBuffer() const;
  /*! @return The image attached to the specified attachment point, or @c NULL
   *  if no image is attached to it.
   */
  Image* getBuffer(Attachment attachment) const;
  /*! Sets the image to use as the default color buffer for this framebuffer.
   *  @param[in] newImage The desired image, or @c NULL to detach the currently
   *  set image.
   *  @return @c true if this framebuffer is complete, or @c false otherwise.
   */
  bool setColorBuffer(Image* newImage);
  /*! sets the image to use as the depth buffer for this framebuffer.
   *  @param[in] newImage The desired image, or @c NULL to detach the currently
   *  set image.
   *  @return @c true if this framebuffer is complete, or @c false otherwise.
   */
  bool setDepthBuffer(Image* newImage);
  /*! sets the image to use for the specified attachment point of this
   *  framebuffer.
   *  @param[in] newImage The desired image, or @c NULL to detach the currently
   *  set image.
   *  @param[in] z The desired Z slice of the specified image to use.  This
   *  only applies to images of 3D textures.
   *  @return @c true if this framebuffer is complete, or @c false otherwise.
   */
  bool setBuffer(Attachment attachment, Image* newImage, unsigned int z = 0);
  /*! Creates an image framebuffer within the specified context.
   */
  static ImageFramebuffer* create(Context& context);
private:
  ImageFramebuffer(Context& context);
  bool init();
  void apply() const;
  unsigned int bufferID;
  Ref<Image> images[5];
};

///////////////////////////////////////////////////////////////////////

template <>
IndexRangeLock<uint8>::IndexRangeLock(IndexRange &range);
template <>
IndexRangeLock<uint16>::IndexRangeLock(IndexRange &range);
template <>
IndexRangeLock<uint32>::IndexRangeLock(IndexRange &range);

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLBUFFER_H*/
///////////////////////////////////////////////////////////////////////
