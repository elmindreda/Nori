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

#include <moira/Config.h>
#include <moira/Core.h>
#include <moira/Signal.h>
#include <moira/Color.h>
#include <moira/Vector.h>
#include <moira/Matrix.h>
#include <moira/Stream.h>
#include <moira/XML.h>
#include <moira/Mesh.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
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

Mesh::~Mesh(void)
{
  while (indexBuffers.size())
  {
    delete indexBuffers.back();
    indexBuffers.pop_back();
  }
}

void Mesh::enqueue(RenderQueue& queue, const Matrix4& transform) const
{
  for (GeometryList::const_iterator i = geometries.begin();  i != geometries.end();  i++)
  {
    RenderStyle* style = RenderStyle::findInstance((*i).styleName);
    if (!style)
    {
      Log::writeWarning("Render style %s not found", (*i).styleName.c_str());
      return;
    }

    RenderOperation operation;
    operation.vertexBuffer = vertexBuffer;
    operation.indexBuffer = (*i).indexBuffer;
    operation.renderMode = (*i).renderMode;
    operation.transform = transform;
    operation.style = style;
    queue.addOperation(operation);
  }
}

void Mesh::render(void) const
{
  vertexBuffer->apply();

  for (GeometryList::const_iterator i = geometries.begin();  i != geometries.end();  i++)
  {
    const Geometry& geometry = *i;

    RenderStyle* style = RenderStyle::findInstance(geometry.styleName);
    if (!style)
    {
      Log::writeError("Render style %s not found", (*i).styleName.c_str());
      return;
    }

    for (unsigned int pass = 0;  pass < style->getPassCount();  pass++)
    {
      style->applyPass(pass);

      geometry.indexBuffer->apply();
      geometry.indexBuffer->render(geometry.renderMode);
    }
  }
}

Mesh::GeometryList& Mesh::getGeometries(void)
{
  return geometries;
}

VertexBuffer* Mesh::getVertexBuffer(void)
{
  return vertexBuffer;
}

Mesh* Mesh::createInstance(const Path& path, const String& name)
{
  MeshReader reader;
  Ptr<moira::Mesh> mesh = reader.read(path);
  if (!mesh)
    return NULL;

  return createInstance(*mesh, name);
}

Mesh* Mesh::createInstance(const moira::Mesh& mesh, const String& name)
{
  Ptr<Mesh> renderMesh = new Mesh(name);
  if (!renderMesh->init(mesh))
    return NULL;

  return renderMesh.detachObject();
}

Mesh::Mesh(const String& name):
  Managed<Mesh>(name)
{
}

bool Mesh::init(const moira::Mesh& mesh)
{
  VertexFormat format;

  if (!format.addComponents("3fv3fn"))
    return false;

  vertexBuffer = VertexBuffer::createInstance((unsigned int) mesh.vertices.size(),
					      format);
  if (!vertexBuffer)
    return false;

  MeshVertex* vertices = (MeshVertex*) vertexBuffer->lock();
  if (!vertices)
    return false;

  for (unsigned int i = 0;  i < mesh.vertices.size();  i++)
    *vertices++ = mesh.vertices[i];

  vertexBuffer->unlock();

  for (moira::Mesh::GeometryList::const_iterator i = mesh.geometries.begin();  i != mesh.geometries.end();  i++)
  {
    geometries.push_back(Geometry());
    Geometry& geometry = geometries.back();

    geometry.styleName = (*i).shaderName;
    geometry.renderMode = GL_TRIANGLES;

    geometry.indexBuffer = IndexBuffer::createInstance((unsigned int) (*i).triangles.size() * 3,
						       IndexBuffer::UINT);
    if (!geometry.indexBuffer)
      return false;

    indexBuffers.push_back(geometry.indexBuffer);

    unsigned int* indices = (unsigned int*) geometry.indexBuffer->lock();
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
