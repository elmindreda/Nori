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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLShader.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>
#include <wendy/GLPass.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderStyle.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderQueue.h>
#include <wendy/RenderMesh.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Mesh::~Mesh(void)
{
  while (!geometries.empty())
  {
    delete geometries.back();
    geometries.pop_back();
  }
}

void Mesh::enqueue(Queue& queue, const Transform3& transform) const
{
  for (GeometryList::const_iterator i = geometries.begin();  i != geometries.end();  i++)
  {
    const Technique* technique = (*i)->style->getActiveTechnique();
    if (!technique)
    {
      Log::writeError("Render style %s has no active technique",
                      (*i)->style->getName().c_str());
      return;
    }

    Operation& operation = queue.createOperation();
    operation.vertexBuffer = vertexBuffer;
    operation.indexBuffer = (*i)->range.getIndexBuffer();
    operation.type = (*i)->primitiveType;
    operation.transform = transform;
    operation.start = (*i)->range.getStart();
    operation.count = (*i)->range.getCount();
    operation.technique = technique;
  }
}

const Sphere& Mesh::getBounds(void) const
{
  return bounds;
}

const Mesh::GeometryList& Mesh::getGeometries(void)
{
  return geometries;
}

Mesh* Mesh::createInstance(const moira::Mesh& mesh, const String& name)
{
  Ptr<Mesh> renderMesh = new Mesh(name);
  if (!renderMesh->init(mesh))
    return NULL;

  return renderMesh.detachObject();
}

Mesh::Mesh(const String& name):
  DerivedResource<Mesh, moira::Mesh>(name)
{
}

Mesh::Mesh(const Mesh& source):
  DerivedResource<Mesh, moira::Mesh>(source)
{
  // NOTE: Not implemented.
}

Mesh& Mesh::operator = (const Mesh& source)
{
  // NOTE: Not implemented.

  return *this;
}

bool Mesh::init(const moira::Mesh& mesh)
{
  unsigned int indexCount = 0;

  for (unsigned int i = 0;  i < mesh.geometries.size();  i++)
    indexCount += (unsigned int) mesh.geometries[i].triangles.size() * 3;

  GL::VertexFormat format;

  if (!format.createComponents("3f:position 3f:normal 2f:mapping"))
    return false;

  vertexBuffer = GL::VertexBuffer::createInstance((unsigned int) mesh.vertices.size(),
                                                  format);
  if (!vertexBuffer)
    return false;

  // NOTE: This may not be portable to really weird platforms.
  vertexBuffer->copyFrom(&mesh.vertices[0], mesh.vertices.size());

  indexBuffer = GL::IndexBuffer::createInstance(indexCount, GL::IndexBuffer::UINT);
  if (!indexBuffer)
    return false;

  unsigned int indexBase = 0;

  for (moira::Mesh::GeometryList::const_iterator i = mesh.geometries.begin();  i != mesh.geometries.end();  i++)
  {
    Geometry* geometry = new Geometry();
    geometries.push_back(geometry);

    indexCount = (unsigned int) (*i).triangles.size() * 3;

    geometry->style = Style::readInstance((*i).shaderName);
    if (!geometry->style)
      return false;

    geometry->primitiveType = GL::TRIANGLE_LIST;
    geometry->range = GL::IndexRange(*indexBuffer, indexBase, indexCount);

    unsigned int* indices = (unsigned int*) geometry->range.lock();
    if (!indices)
      return false;

    for (MeshGeometry::TriangleList::const_iterator j = (*i).triangles.begin();  j != (*i).triangles.end();  j++)
    {
      *indices++ = (*j).indices[0];
      *indices++ = (*j).indices[1];
      *indices++ = (*j).indices[2];
    }

    geometry->range.unlock();

    indexBase += indexCount;
  }

  mesh.getBounds(bounds);

  return true;
}

///////////////////////////////////////////////////////////////////////

/*
Mesh::Geometry::Geometry(const GL::IndexRange& initRange,
                         GLenum initRenderMode,
                               Style* initStyle):
  range(initRange),
  renderMode(initRenderMode),
  style(initStyle)
{
}
*/

const GL::IndexRange& Mesh::Geometry::getIndexRange(void) const
{
  return range;
}

GL::PrimitiveType Mesh::Geometry::getPrimitiveType(void) const
{
  return primitiveType;
}

Style* Mesh::Geometry::getStyle(void) const
{
  return style;
}

void Mesh::Geometry::setStyle(Style* newStyle)
{
  style = newStyle;
}

///////////////////////////////////////////////////////////////////////

void ShadowMesh::update(const Vector3 origin)
{
  Vector3* volume = (Vector3*) vertexBuffer->lock();
  if (!volume)
    return;

  vertexCount = 0;

  for (unsigned int i = 0;  i < triangles.size();  i++)
    triangles[i].status = Triangle::UNREFERENCED;

  for (unsigned int i = 0;  i < edges.size();  i++)
  {
    const Edge& edge = edges[i];
    Triangle::Status statuses[2];

    for (unsigned int j = 0;  j < 2;  j++)
    {
      const Triangle& triangle = triangles[edge.triangles[j]];

      if (triangle.status == Triangle::UNREFERENCED)
      {
	Vector3 eye = origin - vertices[triangle.vertices[0]];

	if (triangle.normal.dot(eye) > 0.f)
	{
	  // Generate front cap
	  for (unsigned int k = 0;  k < 3;  k++)
	    volume[vertexCount++] = vertices[triangle.vertices[k]];

	  // Generate back cap, saving vertices
	  for (unsigned int k = 0;  k < 3;  k++)
	  {
	    Vector3& vertex = extrudedVertices[triangle.vertices[2 - k]];
	    vertex = vertices[triangle.vertices[2 - k]];
	    Vector3 offset = vertex - origin;
	    offset.scaleTo(distance);
	    vertex += offset;
	    volume[vertexCount++] = vertex;
	  }

	  triangle.status = Triangle::FRONT_FACE;
	}
	else
	  triangle.status = Triangle::BACK_FACE;
      }

      statuses[j] = triangle.status;
    }

    if (statuses[0] != statuses[1])
    {
      // Generate sides, re-using vertices

      if (statuses[0] == Triangle::FRONT_FACE)
      {
        volume[vertexCount++] = vertices[edge.vertices[0]];
        volume[vertexCount++] = extrudedVertices[edge.vertices[0]];
        volume[vertexCount++] = extrudedVertices[edge.vertices[1]];
        volume[vertexCount++] = vertices[edge.vertices[1]];
        volume[vertexCount++] = vertices[edge.vertices[0]];
        volume[vertexCount++] = extrudedVertices[edge.vertices[1]];
      }
      else
      {
        volume[vertexCount++] = vertices[edge.vertices[0]];
        volume[vertexCount++] = extrudedVertices[edge.vertices[1]];
        volume[vertexCount++] = extrudedVertices[edge.vertices[0]];
        volume[vertexCount++] = vertices[edge.vertices[0]];
        volume[vertexCount++] = vertices[edge.vertices[1]];
        volume[vertexCount++] = extrudedVertices[edge.vertices[1]];
      }
    }
  }

  vertexBuffer->unlock();
}

void ShadowMesh::enqueue(Queue& queue, const Transform3& transform) const
{
  if (!vertexCount)
  {
    Log::writeWarning("Cannot enqueue non-updated shadow mesh");
    return;
  }

  const Technique* technique = style->getActiveTechnique();
  if (!technique)
  {
    Log::writeError("Render style %s has no active technique", style->getName().c_str());
    return;
  }

  Operation& operation = queue.createOperation();
  operation.vertexBuffer = vertexBuffer;
  operation.type = GL::TRIANGLE_LIST;
  operation.technique = technique;
  operation.count = vertexCount;
  operation.transform = transform;
}

float ShadowMesh::getExtrudeDistance(void) const
{
  return distance;
}

void ShadowMesh::setExtrudeDistance(float newDistance)
{
  distance = newDistance;
}

ShadowMesh* ShadowMesh::createInstance(const moira::Mesh& mesh)
{
  Ptr<ShadowMesh> shadowMesh = new ShadowMesh();
  if (!shadowMesh->init(mesh))
    return NULL;

  return shadowMesh.detachObject();
}

ShadowMesh::ShadowMesh(void):
  distance(100.f),
  vertexCount(0)
{
}

ShadowMesh::ShadowMesh(const ShadowMesh& source)
{
  // NOTE: Not implemented.
}

ShadowMesh& ShadowMesh::operator = (const ShadowMesh& source)
{
  // NOTE: Not implemented.

  return *this;
}

bool ShadowMesh::init(const moira::Mesh& mesh)
{
  // Validate source mesh (a little bit)
  {
    if (mesh.edges.empty())
    {
      Log::writeError("Source for shadow mesh creation must have edge data");
      return false;
    }

    std::vector<unsigned int> references;
    references.insert(references.end(), mesh.edges.size(), 0);

    for (unsigned int i = 0;  i < mesh.geometries.size();  i++)
    {
      const MeshGeometry& geometry = mesh.geometries[i];

      for (unsigned int j = 0;  j < geometry.triangles.size();  j++)
      {
	for (unsigned int k = 0;  k < 3;  k++)
	  references[geometry.triangles[j].edges[k]]++;
      }
    }

    for (unsigned int i = 0;  i < references.size();  i++)
    {
      if (references[i] != 2)
      {
	Log::writeError("Invalid mesh for shadow volume extraction");
	return false;
      }
    }
  }

  // Create hardware objects
  {
    GL::VertexFormat format;
    if (!format.createComponents("3fv"))
      return false;

    vertexBuffer = GL::VertexBuffer::createInstance(mesh.vertices.size() * 2,
                                                    format,
                                                    GL::VertexBuffer::DYNAMIC);
    if (!vertexBuffer)
      return false;

    style = new Style();

    /*
    RenderPass& back = style->createPass();
    back.setStencilOperations(GL_KEEP, GL_INCR, GL_KEEP);
    back.setStencilTesting(true);
    back.setDepthWriting(false);
    back.setColorWriting(false);

    RenderPass& front = style->createPass();
    front.setStencilOperations(GL_KEEP, GL_DECR, GL_KEEP);
    front.setStencilTesting(true);
    front.setDepthWriting(false);
    front.setColorWriting(false);
    */
  }

  // Convert mesh data to internal format

  vertices.reserve(mesh.vertices.size());
  extrudedVertices.resize(mesh.vertices.size());

  for (unsigned int i = 0;  i < mesh.vertices.size();  i++)
    vertices.push_back(mesh.vertices[i].position);

  for (unsigned int i = 0;  i < mesh.geometries.size();  i++)
  {
    const MeshGeometry& geometry = mesh.geometries[i];

    for (unsigned int j = 0;  j < geometry.triangles.size();  j++)
    {
      triangles.push_back(Triangle());
      Triangle& triangle = triangles.back();

      for (unsigned int k = 0;  k < 3;  k++)
          triangle.vertices[k] = geometry.triangles[j].indices[k];

      const Vector3 one = vertices[triangle.vertices[1]] -
                          vertices[triangle.vertices[0]];
      const Vector3 two = vertices[triangle.vertices[2]] -
                          vertices[triangle.vertices[0]];
      triangle.normal = one.cross(two);
      triangle.normal.normalize();

      for (unsigned int k = 0;  k < 3;  k++)
      {
	unsigned int l;

	for (l = 0;  l < edges.size();  l++)
	{
	  if (edges[l].vertices[0] == triangle.vertices[(k + 1) % 3] &&
	      edges[l].vertices[1] == triangle.vertices[k])
	  {
	    edges[l].triangles[1] = triangles.size() - 1;
	    break;
	  }
	}

	if (l == edges.size())
	{
	  edges.push_back(Edge());
	  Edge& edge = edges.back();

	  edge.triangles[0] = triangles.size() - 1;
	  edge.vertices[0] = triangle.vertices[k];
	  edge.vertices[1] = triangle.vertices[(k + 1) % 3];
	}
      }
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
