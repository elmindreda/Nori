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
 *  This is the central renderer class, although it's rarely used to render
 *  anything.
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
  void drawPoint(const Vector2& point);
  void drawLine(const Segment2& segment);
  void drawTriangle(const Triangle2& triangle);
  void drawBezier(const BezierCurve2& spline);
  void drawRectangle(const Rectangle& rectangle);
  void fillRectangle(const Rectangle& rectangle);
  void fillTriangle(const Triangle2& triangle);
  void blitTexture(const Rectangle& area, GL::Texture& texture);
  /*! Allocates a range of indices of the specified type.
   */
  bool allocateIndices(IndexRange& range,
		       unsigned int count,
                       IndexBuffer::Type type = IndexBuffer::UBYTE);
  /*! Allocates a range of vertices of the specified format.
   */
  bool allocateVertices(VertexRange& range,
			unsigned int count,
                        const VertexFormat& format);
  const ColorRGBA& getColor(void) const;
  void setColor(const ColorRGBA& newColor);
  float getLineWidth(void) const;
  void setLineWidth(float newWidth);
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
  GL::Pass drawPass;
  Ref<Texture> defaultTexture;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLRENDER_H*/
///////////////////////////////////////////////////////////////////////
