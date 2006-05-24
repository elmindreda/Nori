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
#ifndef WENDY_GLRENDER_H
#define WENDY_GLRENDER_H
///////////////////////////////////////////////////////////////////////

#include <string>
#include <list>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

class Shader;
class Light;
class VertexBuffer;
class IndexBuffer;

///////////////////////////////////////////////////////////////////////

class RenderOperation
{
public:
  RenderOperation(void);
  bool operator < (const RenderOperation& other) const;
  const VertexBuffer* vertexBuffer;
  const IndexBuffer* indexBuffer;
  const Shader* shader;
  Matrix4 transform;
  GLenum renderMode;
};

///////////////////////////////////////////////////////////////////////

class RenderQueue
{
public:
  typedef std::vector<Light*> LightList;
  typedef std::vector<RenderOperation> OperationList;
  RenderQueue(void);
  void addLight(Light& light);
  void removeLights(void);
  void addOperation(RenderOperation& operation);
  void removeOperations(void);
  void renderOperations(void);
  const LightList& getLights(void) const;
  const OperationList& getOperations(void) const;
private:
  void sortOperations(void);
  LightList lights;
  OperationList operations;
  bool sorted;
};

///////////////////////////////////////////////////////////////////////

class RenderMesh : public Managed<RenderMesh>
{
public:
  class Geometry;
  typedef std::list<Geometry> GeometryList;
  ~RenderMesh(void);
  void enqueue(RenderQueue& queue, const Matrix4& transform) const;
  void render(void) const;
  GeometryList& getGeometries(void);
  VertexBuffer* getVertexBuffer(void);
  static RenderMesh* createInstance(const Path& path, const std::string& name = "");
  static RenderMesh* createInstance(const Mesh& mesh, const std::string& name = "");
private:
  RenderMesh(const std::string& name);
  bool init(const Mesh& mesh);
  typedef std::list<IndexBuffer*> IndexBufferList;
  GeometryList geometries;
  Ptr<VertexBuffer> vertexBuffer;
  IndexBufferList indexBuffers;
};

///////////////////////////////////////////////////////////////////////

class RenderMesh::Geometry
{
public:
  IndexBuffer* indexBuffer;
  GLenum renderMode;
  std::string shaderName;
};

///////////////////////////////////////////////////////////////////////

class RenderSprite : public Managed<RenderSprite>
{
public:
  ~RenderSprite(void);
  void enqueue(RenderQueue& queue, const Matrix4& transform) const;
  void render(void) const;
  IndexBuffer* getIndexBuffer(void);
  VertexBuffer* getVertexBuffer(void);
  const std::string& getShaderName(void) const;
  void setShaderName(const std::string& newShaderName);
  const Vector2& getSpriteSize(void) const;
  void setSpriteSize(const Vector2& newSize);
  static RenderSprite* createInstance(const std::string& name = "");
private:
  RenderSprite(const std::string& name);
  bool init(void);
  std::string shaderName;
  Vector2 spriteSize;
  Ptr<IndexBuffer> indexBuffer;
  Ptr<VertexBuffer> vertexBuffer;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLRENDER_H*/
///////////////////////////////////////////////////////////////////////
