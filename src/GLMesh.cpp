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

#include <moira/Config.h>
#include <moira/Core.h>
#include <moira/Color.h>
#include <moira/Vector.h>
#include <moira/Matrix.h>
#include <moira/Stream.h>
#include <moira/XML.h>
#include <moira/Mesh.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLVertex.h>
#include <wendy/GLIndexBuffer.h>
#include <wendy/GLVertexBuffer.h>
#include <wendy/GLShader.h>
#include <wendy/GLRender.h>
#include <wendy/GLMesh.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

SimpleMesh::~SimpleMesh(void)
{
  while (indexBuffers.size())
  {
    delete indexBuffers.back();
    indexBuffers.pop_back();
  }
}

void SimpleMesh::enqueue(RenderQueue& queue, const Matrix4& transform) const
{
  for (GeometryList::const_iterator i = geometries.begin();  i != geometries.end();  i++)
  {
    if (Shader* shader = Shader::findInstance((*i).shaderName))
    {
      RenderOperation operation;
      operation.vertexBuffer = vertexBuffer;
      operation.indexBuffer = (*i).indexBuffer;
      operation.renderMode = (*i).renderMode;
      operation.transform = transform;
      operation.shader = shader;
      queue.addOperation(operation);
    }
  }
}

void SimpleMesh::render(void) const
{
  vertexBuffer->apply();

  for (GeometryList::const_iterator i = geometries.begin();  i != geometries.end();  i++)
  {
    const Geometry& geometry = *i;

    if (Shader* shader = Shader::findInstance(geometry.shaderName))
    {
      for (unsigned int pass = 0;  pass < shader->getPassCount();  pass++)
      {
        shader->applyPass(pass);

        geometry.indexBuffer->apply();
        geometry.indexBuffer->render(geometry.renderMode);
      }
    }
  }
}

SimpleMesh::GeometryList& SimpleMesh::getGeometries(void)
{
  return geometries;
}

VertexBuffer* SimpleMesh::getVertexBuffer(void)
{
  return vertexBuffer;
}

SimpleMesh* SimpleMesh::createInstance(const Path& path)
{
  MeshReader reader;
  Ptr<Mesh> mesh = reader.read(path);
  if (!mesh)
    return NULL;

  return createInstance(*mesh);
}

SimpleMesh* SimpleMesh::createInstance(const Mesh& mesh, const std::string& name)
{
  Ptr<SimpleMesh> simpleMesh = new SimpleMesh(name);
  if (!simpleMesh->init(mesh))
    return NULL;

  return simpleMesh.detachObject();
}

SimpleMesh::SimpleMesh(const std::string& name):
  Managed<SimpleMesh>(name)
{
}

bool SimpleMesh::init(const Mesh& mesh)
{
  VertexFormat format;

  if (!format.addComponents("3fv3fn"))
    return false;

  std::string vertexBufferName;
  vertexBufferName.append("mesh:");
  vertexBufferName.append(getName());

  vertexBuffer = VertexBuffer::createInstance(vertexBufferName, mesh.vertices.size(), format);
  if (!vertexBuffer)
    return false;

  MeshVertex* vertices = reinterpret_cast<MeshVertex*>(vertexBuffer->lock());
  if (!vertices)
    return false;

  for (Mesh::VertexList::const_iterator i = mesh.vertices.begin();  i != mesh.vertices.end();  i++)
    *vertices++ = *i;

  vertexBuffer->unlock();

  for (Mesh::GeometryList::const_iterator i = mesh.geometries.begin();  i != mesh.geometries.end();  i++)
  {
    geometries.push_back(Geometry());
    Geometry& geometry = geometries.back();

    geometry.shaderName = (*i).shaderName;
    geometry.renderMode = GL_TRIANGLES;

    std::string indexBufferName;
    indexBufferName.append("mesh:");
    indexBufferName.append(getName());
    indexBufferName.append("/");
    indexBufferName.append(geometry.shaderName);

    geometry.indexBuffer = IndexBuffer::createInstance(indexBufferName, (*i).triangles.size() * 3, IndexBuffer::UINT);
    if (!geometry.indexBuffer)
      return false;

    indexBuffers.push_back(geometry.indexBuffer);

    unsigned int* indices = reinterpret_cast<unsigned int*>(geometry.indexBuffer->lock());
    if (!indices)
      return false;

    for (MeshGeometry::TriangleList::const_iterator j = (*i).triangles.begin();  j != (*i).triangles.end();  j++)
    {
      *indices++ = (*j).indices[0];
      *indices++ = (*j).indices[1];
      *indices++ = (*j).indices[2];
    }

    geometry.indexBuffer->unlock();
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
