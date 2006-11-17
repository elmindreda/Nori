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

/*! Memory locking type enumeration.
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
 *
 *  Uses VBO if available, with fallback to vertex arrays.
 */
class VertexBuffer : public Managed<VertexBuffer>
{
public:
  /*! Vertex buffer usage hint enumeration.
   */
  enum Usage
  {
    /*! Data will be specified once and used many times.
     */
    STATIC = GL_STATIC_DRAW_ARB,
    /*! Data will be specified once and used a few times.
     */
    STREAM = GL_STREAM_DRAW_ARB,
    /*! Data will be repeatedly respecified and re-used.
     */
    DYNAMIC = GL_DYNAMIC_DRAW_ARB,
  };
  /*! Destructor.
   */
  ~VertexBuffer(void);
  /*! Makes this vertex buffer current.
   */
  void apply(void) const;
  /*! Renders the specified range of this vertex buffer.
   *  @param mode The desired primitive mode.
   *  @param start The start of the desired vertex range.
   *  @param count The desired number of vertices to use.
   *  @remarks If this is not the current vertex buffer, it will be made
   *  current by a call to VertexBuffer::apply before rendering.
   */
  void render(unsigned int mode,
              unsigned int start = 0,
	      unsigned int count = 0) const;
  /*! Locks this vertex buffer for reading and writing.
   *  @return The base address of the vertices.
   */
  void* lock(void);
  /*! Unlocks this vertex buffer, finalizing any changes.
   */
  void unlock(void);
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
  /*! Invalidates the current vertex buffer.
   */
  static void invalidateCurrent(void);
  /*! @return The current vertex buffer, or @c NULL if no vertex buffer is
   *  current.
   */
  static VertexBuffer* getCurrent(void);
private:
  VertexBuffer(const String& name);
  VertexBuffer(const VertexBuffer& source);
  VertexBuffer& operator = (const VertexBuffer& source);
  bool init(const VertexFormat& format, unsigned int count, Usage usage);
  bool locked;
  VertexFormat format;
  GLuint bufferID;
  unsigned int count;
  Usage usage;
  Block data;
  static VertexBuffer* current;
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
  VertexRange(void);
  VertexRange(VertexBuffer& vertexBuffer, unsigned int start, unsigned int count);
  void render(void) const;
  void* lock(void) const;
  void unlock(void) const;
  VertexBuffer* getVertexBuffer(void) const;
  unsigned int getStart(void) const;
  unsigned int getCount(void) const;
private:
  VertexBuffer* vertexBuffer;
  unsigned int start;
  unsigned int count;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Index (or element) buffer.
 *  @ingroup opengl
 *
 *  Uses VBO if available, with fallback to index arrays.
 */
class IndexBuffer : public Managed<IndexBuffer>
{
public:
  /*! Index buffer element type enumeration.
   */
  enum Type
  {
    /*! Indices are of type unsigned int.
     */
    UINT = GL_UNSIGNED_INT,
    /*! Indices are of type unsigned short.
     */
    USHORT = GL_UNSIGNED_SHORT,
    /*! Indices are of type unsigned char.
     */
    UBYTE = GL_UNSIGNED_BYTE,
  };
  /*! Index buffer usage hint enumeration.
   */
  enum Usage
  {
    /*! Data will be specified once and used many times.
     */
    STATIC = GL_STATIC_DRAW_ARB,
    /*! Data will be specified once and used a few times.
     */ STREAM = GL_STREAM_DRAW_ARB,
    /*! Data will be repeatedly respecified and re-used.
     */
    DYNAMIC = GL_DYNAMIC_DRAW_ARB,
  };
  /*! Destructor.
   */
  ~IndexBuffer(void);
  /*! Makes this the current index buffer.
   */
  void apply(void) const;
  /*! Renders from the specified vertex buffer with the specified range of
   *  index elements in this index buffer.
   *  @param vertexBuffer The desired vertex buffer.
   *  @param mode The desired primitive mode.
   *  @param start The start of the desired index element range.
   *  @param count The desired number of index elements to use.
   *  @remarks If the specified vertex buffer is not current, it will be made
   *  current by a call to VertexBuffer::apply before rendering.
   *  @remarks If this is not the current index buffer, it will be made
   *  current by a call to IndexBuffer::apply before rendering.
   */
  void render(const VertexBuffer& vertexBuffer,
              unsigned int mode,
              unsigned int start = 0,
	      unsigned int count = 0) const;
  /*! Locks this index buffer for reading and writing.
   *  @return The base address of the index elements.
   */
  void* lock(void);
  /*! Unlocks this index buffer, finalizing any changes.
   */
  void unlock(void);
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
  /*! Invalidates the current index buffer.
   */
  static void invalidateCurrent(void);
  /*! @return The current index buffer, or @c NULL if no index buffer is
   *  current.
   */
  static IndexBuffer* getCurrent(void);
private:
  IndexBuffer(const String& name);
  IndexBuffer(const IndexBuffer& source);
  IndexBuffer& operator = (const IndexBuffer& source);
  bool init(unsigned int count, Type type, Usage usage);
  bool locked;
  Type type;
  Usage usage;
  GLuint bufferID;
  unsigned int count;
  Block data;
  static IndexBuffer* current;
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
  IndexRange(void);
  IndexRange(IndexBuffer& indexBuffer, unsigned int start, unsigned int count);
  void render(const VertexBuffer& vertexBuffer) const;
  void* lock(void) const;
  void unlock(void) const;
  IndexBuffer* getIndexBuffer(void) const;
  unsigned int getStart(void) const;
  unsigned int getCount(void) const;
private:
  IndexBuffer* indexBuffer;
  unsigned int start;
  unsigned int count;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLBUFFER_H*/
///////////////////////////////////////////////////////////////////////
