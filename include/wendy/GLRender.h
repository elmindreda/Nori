///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_GLRENDER_H
#define WENDY_GLRENDER_H
///////////////////////////////////////////////////////////////////////

#include <list>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @brief The renderer singleton.
 *  @ingroup renderer
 *
 *  This is the central renderer class, although it doesn't actually
 *  render anything.
 */
class Renderer : public Trackable, public Singleton<Renderer>
{
public:
  void begin2D(const Vector2& resolution = Vector2(1.f, 1.f)) const;
  void begin3D(float FOV = 90.f,
               float aspect = 0.f,
	       float nearZ = 0.01f,
	       float farZ = 1000.f) const;
  void begin3D(const Matrix4& projection) const;
  void end(void) const;
  void pushTransform(const Matrix4& transform) const;
  void popTransform(void) const;
  /*! Allocates a range of indices of the specified type.
   *  @param[out] range The newly allocated index range.
   *  @param[in] count The number of indices to allocate.
   *  @param[in] type The type of indices to allocate.
   *  @return @c true if the allocattion succeeded, or @c false if an
   *  error occurred.
   */
  bool allocateIndices(IndexRange& range,
		       unsigned int count,
                       IndexBuffer::Type type = IndexBuffer::UBYTE);
  /*! Allocates a range of vertices of the specified format.
   *  @param[out] range The newly allocated vertex range.
   *  @param[in] count The number of vertices to allocate.
   *  @param[in] format The format of vertices to allocate.
   *  @return @c true if the allocattion succeeded, or @c false if an
   *  error occurred.
   */
  bool allocateVertices(VertexRange& range,
			unsigned int count,
                        const VertexFormat& format);
  /*! Creates the renderer singleton.
   */
  static bool create(void);
private:
  struct IndexBufferSlot
  {
    Ptr<IndexBuffer> indexBuffer;
    unsigned int available;
  };
  struct VertexBufferSlot
  {
    Ptr<VertexBuffer> vertexBuffer;
    unsigned int available;
  };
  Renderer(void);
  bool init(void);
  void onContextFinish(void);
  static void onContextDestroy(void);
  typedef std::list<IndexBufferSlot> IndexBufferList;
  typedef std::list<VertexBufferSlot> VertexBufferList;
  IndexBufferList indexBuffers;
  VertexBufferList vertexBuffers;
  Ref<Texture> defaultTexture;
  Ref<ShaderProgram> defaultProgram;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLRENDER_H*/
///////////////////////////////////////////////////////////////////////
