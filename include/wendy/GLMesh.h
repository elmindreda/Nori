///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@home.se>
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
#ifndef WEGLMESH_H
#define WEGLMESH_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class SimpleMesh : public Managed<SimpleMesh>
{
public:
  class Geometry;
  typedef std::list<Geometry> GeometryList;
  ~SimpleMesh(void);
  void enqueue(RenderQueue& queue, const Matrix4& transform) const;
  void render(void) const;
  GeometryList& getGeometries(void);
  VertexBuffer* getVertexBuffer(void);
  static SimpleMesh* createInstance(const Path& path);
  static SimpleMesh* createInstance(const Mesh& mesh, const std::string& name = "");
private:
  SimpleMesh(const std::string& name);
  bool init(const Mesh& mesh);
  typedef std::list<IndexBuffer*> IndexBufferList;
  GeometryList geometries;
  Ptr<VertexBuffer> vertexBuffer;
  IndexBufferList indexBuffers;
};

///////////////////////////////////////////////////////////////////////

class SimpleMesh::Geometry
{
public:
  IndexBuffer* indexBuffer;
  GLenum renderMode;
  std::string shaderName;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WEGLMESH*/
///////////////////////////////////////////////////////////////////////
