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

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

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
  LOCK_READ_WRITE,
};

///////////////////////////////////////////////////////////////////////

/*! @brief Vertex buffer.
 *  @ingroup opengl
 */
class VertexBuffer : public Managed<VertexBuffer>, public RefObject
{
  friend class Renderer;
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
    DYNAMIC,
  };
  /*! Destructor.
   */
  ~VertexBuffer(void);
  /*! Locks this vertex buffer for reading and writing.
   *  @return The base address of the vertices.
   */
  void* lock(LockType type = LOCK_WRITE_ONLY);
  /*! Unlocks this vertex buffer, finalizing any changes.
   */
  void unlock(void);
  /*! Copies the specified data into this vertex buffer, starting at the
   *  specified offset.
   *  @param[in] source The base address of the source data.
   *  @param[in] count The number of vertices to copy.
   *  @param[in] start The index of the first vertex to be written to.
   */
  void copyFrom(const void* source, unsigned int count, unsigned int start = 0);
  /*! Copies the specified number of bytes from this vertex buffer, starting
   *  at the specified offset.
   *  @param[in] target The base address of the destination buffer.
   *  @param[in] count The number of vertices to copy.
   *  @param[in] start The index of the first vertex to read from.
   */
  void copyTo(void* target, unsigned int count, unsigned int start = 0);
  /*! @return The usage hint of this vertex buffer.
   */
  Usage getUsage(void) const;
  /*! @return The format of this vertex buffer.
   */
  const VertexFormat& getFormat(void) const;
  /*! @return The number of vertices in this vertex buffer.
   */
  unsigned int getCount(void) const;
  /*! Creates a vertex buffer with the specified properties.
   *  @param count The desired number of vertices.
   *  @param format The desired format of the vertices.
   *  @param usage The desired usage hint.
   *  @param name The desired name of the vertex buffer.
   *  @return The newly created vertex buffer, or @c NULL if an error occurred.
   */
  static VertexBuffer* createInstance(unsigned int count,
                                      const VertexFormat& format,
				      Usage usage = STATIC,
				      const String& name = ""); 
private:
  VertexBuffer(const String& name);
  VertexBuffer(const VertexBuffer& source);
  VertexBuffer& operator = (const VertexBuffer& source);
  bool init(const VertexFormat& format, unsigned int count, Usage usage);
  void apply(void) const;
  static void invalidateCurrent(void);
  bool locked;
  VertexFormat format;
  unsigned int bufferID;
  unsigned int count;
  Usage usage;
  static VertexBuffer* current;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Index (or element) buffer.
 *  @ingroup opengl
 */
class IndexBuffer : public Managed<IndexBuffer>, public RefObject
{
  friend class Renderer;
public:
  /*! Index buffer element type enumeration.
   */
  enum Type
  {
    /*! Indices are of type unsigned int.
     */
    UINT,
    /*! Indices are of type unsigned short.
     */
    USHORT,
    /*! Indices are of type unsigned char.
     */
    UBYTE,
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
    DYNAMIC,
  };
  /*! Destructor.
   */
  ~IndexBuffer(void);
  /*! Locks this index buffer for reading and writing.
   *  @param[in] type The desired type of lock.
   *  @return The base address of the index elements.
   */
  void* lock(LockType type = LOCK_WRITE_ONLY);
  /*! Unlocks this index buffer, finalizing any changes.
   */
  void unlock(void);
  /*! Copies the specified data into this index buffer, starting at the
   *  specified offset.
   *  @param[in] source The base address of the source data.
   *  @param[in] count The number of indices to copy.
   *  @param[in] start The index of the first index to be written to.
   */
  void copyFrom(const void* source, unsigned int count, unsigned int start = 0);
  /*! Copies the specified number of bytes from this index buffer, starting
   *  at the specified offset.
   *  @param[in] target The base address of the destination buffer.
   *  @param[in] count The number of indices to copy.
   *  @param[in] start The index of the first index to read from.
   */
  void copyTo(void* target, unsigned int count, unsigned int start = 0);
  /*! @return The type of the index elements in this index buffer.
   */
  Type getType(void) const;
  /*! @return The usage hint of this index buffer.
   */
  Usage getUsage(void) const;
  /*! @return The number of index elements in this index buffer.
   */
  unsigned int getCount(void) const;
  /*! Creates an index buffer with the specified properties.
   *  @param count The desired number of index elements.
   *  @param type The desired type of the index elements.
   *  @param usage The desired usage hint.
   *  @param name The desired name of the index buffer.
   *  @return The newly created index buffer, or @c NULL if an error occurred.
   */
  static IndexBuffer* createInstance(unsigned int count,
				     Type type = UINT,
				     Usage usage = STATIC,
				     const String& name = "");
  /*! @return The size, in bytes, of the specified element type.
   */
  static size_t getTypeSize(Type type);
private:
  IndexBuffer(const String& name);
  IndexBuffer(const IndexBuffer& source);
  IndexBuffer& operator = (const IndexBuffer& source);
  bool init(unsigned int count, Type type, Usage usage);
  void apply(void) const;
  static void invalidateCurrent(void);
  bool locked;
  Type type;
  Usage usage;
  unsigned int bufferID;
  unsigned int count;
  static IndexBuffer* current;
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
  VertexRange(void);
  /*! Constructor.
   */
  VertexRange(VertexBuffer& vertexBuffer);
  /*! Constructor.
   */
  VertexRange(VertexBuffer& vertexBuffer, unsigned int start, unsigned int count);
  /*! Locks this vertex range into memory and returns its address.
   *  @return The base address of this vertex range, or @c NULL if an error occurred.
   */
  void* lock(LockType type = LOCK_WRITE_ONLY) const;
  /*! Unlocks this vertex range.
   */
  void unlock(void) const;
  /*! Copies the specified data into this vertex range.
   *  @param[in] source The base address of the source data.
   */
  void copyFrom(const void* source);
  /*! Copies the specified number of bytes from this vertex range.
   *  @param[in] target The base address of the destination buffer.
   */
  void copyTo(void* target);
  /*! @return The vertex buffer underlying this vertex range.
   */
  VertexBuffer* getVertexBuffer(void) const;
  /*! @return The index of the first vertex in this vertex range.
   */
  unsigned int getStart(void) const;
  /*! @return The number of vertices in this vertex range.
   */
  unsigned int getCount(void) const;
private:
  Ref<VertexBuffer> vertexBuffer;
  unsigned int start;
  unsigned int count;
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
  /*! Constructor.
   */
  IndexRange(void);
  /*! Constructor.
   */
  IndexRange(IndexBuffer& indexBuffer);
  /*! Constructor.
   */
  IndexRange(IndexBuffer& indexBuffer, unsigned int start, unsigned int count);
  /*! Locks this index range into memory and returns its address.
   *  @param[in] type The desired type of lock.
   *  @return The base address of this index range, or @c NULL if an error occurred.
   */
  void* lock(LockType type = LOCK_WRITE_ONLY) const;
  /*! Unlocks this index range.
   */
  void unlock(void) const;
  /*! Copies the specified data into this index range.
   *  @param[in] source The base address of the source data.
   */
  void copyFrom(const void* source);
  /*! Copies the specified number of bytes from this index range.
   *  @param[in] target The base address of the destination buffer.
   */
  void copyTo(void* target);
  /*! @return The index buffer underlying this index range.
   */
  IndexBuffer* getIndexBuffer(void) const;
  /*! @return The index of the first index in this index range.
   */
  unsigned int getStart(void) const;
  /*! @return The number of indices in this index range.
   */
  unsigned int getCount(void) const;
private:
  Ref<IndexBuffer> indexBuffer;
  unsigned int start;
  unsigned int count;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Geometric primitive range.
 *  @ingroup opengl
 */
class PrimitiveRange
{
public:
  PrimitiveRange(void);
  PrimitiveRange(PrimitiveType type, VertexBuffer& vertexBuffer);
  PrimitiveRange(PrimitiveType type, const VertexRange& vertexRange);
  PrimitiveRange(PrimitiveType type,
                 VertexBuffer& vertexBuffer,
                 IndexBuffer& indexBuffer);
  PrimitiveRange(PrimitiveType type,
                 VertexBuffer& vertexBuffer,
                 const IndexRange& indexRange);
  PrimitiveRange(PrimitiveType type,
                 VertexBuffer& vertexBuffer,
		 unsigned int start,
		 unsigned int count);
  PrimitiveRange(PrimitiveType type,
                 VertexBuffer& vertexBuffer,
                 IndexBuffer& indexBuffer,
	         unsigned int start,
	         unsigned int count);
  /*! @return @c true if this primitive range contains zero primitives,
   *  otherwise @c false.
   */
  bool isEmpty(void) const;
  /*! @return The type of primitives in this range.
   */
  PrimitiveType getType(void) const;
  /*! @return The vertex buffer used by this primitive range.
   */
  VertexBuffer* getVertexBuffer(void) const;
  /*! @return The index buffer used by this primitive range, or @c NULL if no
   *  index buffer is used.
   */
  IndexBuffer* getIndexBuffer(void) const;
  unsigned int getStart(void) const;
  unsigned int getCount(void) const;
private:
  PrimitiveType type;
  Ref<VertexBuffer> vertexBuffer;
  Ref<IndexBuffer> indexBuffer;
  unsigned int start;
  unsigned int count;
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
   */
  inline VertexRangeLock(VertexRange& range);
  /*! Destructor.
   *  Releases any lock held.
   */
  inline ~VertexRangeLock(void);
  /*! @return The base address of the locked vertex range.
   *  @remarks The vertex range is locked the first time this is called.
   */
  inline operator T* (void);
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
   */
  inline IndexRangeLock(IndexRange& range);
  /*! Destructor.
   *  Releases any lock held.
   */
  inline ~IndexRangeLock(void);
  /*! @return The base address of the locked index range.
   *  @remarks The index range is locked the first time this is called.
   */
  inline operator T* (void);
private:
  IndexRange range;
  T* indices;
};

///////////////////////////////////////////////////////////////////////

class RenderBuffer : public Image, public Managed<RenderBuffer>
{
public:
  virtual ~RenderBuffer(void);
  unsigned int getWidth(void) const;
  unsigned int getHeight(void) const;
  const PixelFormat& getFormat(void) const;
  static RenderBuffer* createInstance(const PixelFormat& format,
                                      unsigned int width,
                                      unsigned int height,
                                      const String& name = "");
private:
  RenderBuffer(const String& name);
  bool init(const PixelFormat& format, unsigned int width, unsigned int height);
  void attach(int attachment);
  void detach(void);
  unsigned int bufferID;
  unsigned int width;
  unsigned int height;
  PixelFormat format;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
inline VertexRangeLock<T>::VertexRangeLock(VertexRange& initRange):
  range(initRange),
  vertices(NULL)
{
  // TODO: Type matching.
}

template <typename T>
inline VertexRangeLock<T>::~VertexRangeLock(void)
{
  if (vertices)
    range.unlock();
}

template <typename T>
inline VertexRangeLock<T>::operator T* (void)
{
  if (vertices)
    return vertices;

  vertices = (T*) range.lock();

  return vertices;
}

///////////////////////////////////////////////////////////////////////

template <typename T>
inline IndexRangeLock<T>::IndexRangeLock(IndexRange& initRange):
  range(initRange),
  indices(NULL)
{
  // TODO: Type matching.
}

template <typename T>
inline IndexRangeLock<T>::~IndexRangeLock(void)
{
  if (indices)
    range.unlock();
}

template <typename T>
inline IndexRangeLock<T>::operator T* (void)
{
  if (indices)
    return indices;

  indices = (T*) range.lock();

  return indices;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLBUFFER_H*/
///////////////////////////////////////////////////////////////////////
