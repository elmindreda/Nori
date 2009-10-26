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
#include <wendy/GLProgram.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
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
    Material* material = (*i)->getMaterial();

    const Technique* technique = material->getActiveTechnique();
    if (!technique)
    {
      Log::writeError("Material %s has no active technique",
                      material->getName().c_str());
      return;
    }

    Operation& operation = queue.createOperation();
    operation.range = GL::PrimitiveRange((*i)->getPrimitiveType(),
                                         *vertexBuffer,
                                         (*i)->getIndexRange());
    operation.transform = transform;
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
  Ptr<Mesh> renderMesh(new Mesh(name));
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
    indexCount = (unsigned int) (*i).triangles.size() * 3;

    Ref<Material> material = Material::readInstance((*i).shaderName);
    if (!material)
    {
      Log::writeError("Cannot find material \'%s\' for mesh \'%s\'",
                      (*i).shaderName.c_str(),
		      getName().c_str());
      return false;
    }

    GL::IndexRange range(*indexBuffer, indexBase, indexCount);

    Geometry* geometry = new Geometry(range, GL::TRIANGLE_LIST, material);
    geometries.push_back(geometry);

    GL::IndexRangeLock<unsigned int> indices(range);
    if (!indices)
      return false;

    unsigned int index = 0;

    for (MeshGeometry::TriangleList::const_iterator j = (*i).triangles.begin();  j != (*i).triangles.end();  j++)
    {
      indices[index++] = (*j).indices[0];
      indices[index++] = (*j).indices[1];
      indices[index++] = (*j).indices[2];
    }

    indexBase += indexCount;
  }

  mesh.getBounds(bounds);

  return true;
}

///////////////////////////////////////////////////////////////////////

Mesh::Geometry::Geometry(const GL::IndexRange& initRange,
                         GL::PrimitiveType initPrimitiveType,
                         Material* initMaterial):
  range(initRange),
  primitiveType(initPrimitiveType),
  material(initMaterial)
{
}

const GL::IndexRange& Mesh::Geometry::getIndexRange(void) const
{
  return range;
}

GL::PrimitiveType Mesh::Geometry::getPrimitiveType(void) const
{
  return primitiveType;
}

Material* Mesh::Geometry::getMaterial(void) const
{
  return material;
}

void Mesh::Geometry::setMaterial(Material* newMaterial)
{
  material = newMaterial;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
