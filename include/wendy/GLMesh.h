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
#ifndef WENDY_GLMESH_H
#define WENDY_GLMESH_H
///////////////////////////////////////////////////////////////////////

#include <list>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

class VertexBuffer;
class IndexBuffer;
class RenderStyle;

///////////////////////////////////////////////////////////////////////

class Mesh : public DerivedResource<Mesh, moira::Mesh>
{
public:
  class Geometry;
  typedef std::list<Geometry> GeometryList;
  ~Mesh(void);
  void enqueue(RenderQueue& queue, const Matrix4& transform) const;
  void render(void) const;
  GeometryList& getGeometries(void);
  VertexBuffer* getVertexBuffer(void);
  static Mesh* createInstance(const moira::Mesh& mesh, const String& name = "");
private:
  Mesh(const String& name);
  bool init(const moira::Mesh& mesh);
  typedef std::list<IndexBuffer*> IndexBufferList;
  GeometryList geometries;
  Ptr<VertexBuffer> vertexBuffer;
  IndexBufferList indexBuffers;
};

///////////////////////////////////////////////////////////////////////

class Mesh::Geometry
{
public:
  IndexBuffer* indexBuffer;
  GLenum renderMode;
  String styleName;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLMESH_H*/
///////////////////////////////////////////////////////////////////////
