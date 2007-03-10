///////////////////////////////////////////////////////////////////////
// Wendy default renderer
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
#ifndef WENDY_RENDERMESH_H
#define WENDY_RENDERMESH_H
///////////////////////////////////////////////////////////////////////

#include <list>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

/*! @brief Geometry mesh object.
 *  @ingroup renderer
 *
 *  This class represents a single static mesh, consisting of one or more
 *  Mesh::Geometry objects. Each geometry is a part of the mesh using the
 *  same render style and primitive mode.
 */
class Mesh : public Renderable,
             public DerivedResource<Mesh, moira::Mesh>,
	     public RefObject<Mesh>
{
public:
  class Geometry;
  void enqueue(Queue& queue, const Transform3& transform) const;
  const Sphere& getBounds(void) const;
  static Mesh* createInstance(const moira::Mesh& mesh, const String& name = "");
private:
  Mesh(const String& name);
  Mesh(const Mesh& source);
  Mesh& operator = (const Mesh& source);
  bool init(const moira::Mesh& mesh);
  typedef std::list<Geometry> GeometryList;
  GeometryList geometries;
  Ptr<GL::VertexBuffer> vertexBuffer;
  Ptr<GL::IndexBuffer> indexBuffer;
  Sphere bounds;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Geometry mesh subset.
 *  @ingroup renderer
 *
 *  This class represents a subset of a mesh, using a single render style
 *  and a single primitive mode.
 */
class Mesh::Geometry
{
public:
  /*! The range of indices used by this geometry.
   */
  GL::IndexRange range;
  /*! The primitive mode used by this geometry.
   */
  GLenum renderMode;
  /*! The render style used by this geometry.
   */
  Ref<Style> style;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Shadow volume generator.
 *  @ingroup renderer
 *
 *  This class is used to generate shadow volumes from a given source triangle
 *  mesh. It is intended for use with static geometry.
 */
class ShadowMesh : public Renderable
{
public:
  void update(const Vector3 origin);
  void enqueue(Queue& queue, const Transform3& transform) const;
  float getExtrudeDistance(void) const;
  void setExtrudeDistance(float newDistance);
  static ShadowMesh* createInstance(const moira::Mesh& mesh);
private:
  class Edge;
  class Triangle;
  ShadowMesh(void);
  ShadowMesh(const ShadowMesh& source);
  ShadowMesh& operator = (const ShadowMesh& source);
  bool init(const moira::Mesh& mesh);
  typedef std::vector<Vector3> VertexList;
  typedef std::vector<Edge> EdgeList;
  typedef std::vector<Triangle> TriangleList;
  VertexList vertices;
  mutable VertexList extrudedVertices;
  TriangleList triangles;
  EdgeList edges;
  float distance;
  unsigned int vertexCount;
  Ptr<GL::VertexBuffer> vertexBuffer;
  Ref<Style> style;
};

///////////////////////////////////////////////////////////////////////

/*! @internal
 */
class ShadowMesh::Edge
{
public:
  unsigned int vertices[2];
  unsigned int triangles[2];
};

///////////////////////////////////////////////////////////////////////

/*! @internal
 */
class ShadowMesh::Triangle
{
public:
  enum Status
  {
    UNREFERENCED,
    FRONT_FACE,
    BACK_FACE,
  };
  unsigned int vertices[3];
  Vector3 normal;
  mutable Status status;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERMESH_H*/
///////////////////////////////////////////////////////////////////////
