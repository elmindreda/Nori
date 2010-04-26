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

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Block.h>
#include <wendy/Color.h>
#include <wendy/Vector.h>
#include <wendy/Matrix.h>
#include <wendy/Rectangle.h>
#include <wendy/Plane.h>
#include <wendy/Sphere.h>
#include <wendy/Quaternion.h>
#include <wendy/Transform.h>
#include <wendy/Frustum.h>
#include <wendy/Pixel.h>
#include <wendy/Vertex.h>
#include <wendy/Timer.h>
#include <wendy/Signal.h>
#include <wendy/Path.h>
#include <wendy/Stream.h>
#include <wendy/Managed.h>
#include <wendy/Resource.h>
#include <wendy/Image.h>
#include <wendy/Mesh.h>

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

Mesh* Mesh::createInstance(const wendy::Mesh& mesh, const String& name)
{
  Ptr<Mesh> renderMesh(new Mesh(name));
  if (!renderMesh->init(mesh))
    return NULL;

  return renderMesh.detachObject();
}

Mesh::Mesh(const String& name):
  DerivedResource<Mesh, wendy::Mesh>(name)
{
}

Mesh::Mesh(const Mesh& source):
  DerivedResource<Mesh, wendy::Mesh>(source)
{
  // NOTE: Not implemented.
}

Mesh& Mesh::operator = (const Mesh& source)
{
  // NOTE: Not implemented.

  return *this;
}

bool Mesh::init(const wendy::Mesh& mesh)
{
  size_t indexCount = 0;

  for (size_t i = 0;  i < mesh.geometries.size();  i++)
    indexCount += mesh.geometries[i].triangles.size() * 3;

  VertexFormat format;

  if (!format.createComponents("3f:position 3f:normal 2f:mapping"))
    return false;

  vertexBuffer = GL::VertexBuffer::createInstance((unsigned int) mesh.vertices.size(),
                                                  format);
  if (!vertexBuffer)
    return false;

  vertexBuffer->copyFrom(&mesh.vertices[0], mesh.vertices.size());

  GL::IndexBuffer::Type indexType;

  if (indexCount <= (1 << 8))
    indexType = GL::IndexBuffer::UBYTE;
  else if (indexCount <= (1 << 16))
    indexType = GL::IndexBuffer::USHORT;
  else
    indexType = GL::IndexBuffer::UINT;

  indexBuffer = GL::IndexBuffer::createInstance(indexCount, indexType);
  if (!indexBuffer)
    return false;

  size_t indexBase = 0;

  for (wendy::Mesh::GeometryList::const_iterator i = mesh.geometries.begin();  i != mesh.geometries.end();  i++)
  {
    indexCount = (*i).triangles.size() * 3;

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

    size_t index = 0;

    if (indexType == GL::IndexBuffer::UBYTE)
    {
      GL::IndexRangeLock<unsigned char> indices(range);
      if (!indices)
        return false;

      for (MeshGeometry::TriangleList::const_iterator j = (*i).triangles.begin();
           j != (*i).triangles.end();
           j++)
      {
        indices[index++] = (*j).indices[0];
        indices[index++] = (*j).indices[1];
        indices[index++] = (*j).indices[2];
      }
    }
    else if (indexType == GL::IndexBuffer::USHORT)
    {
      GL::IndexRangeLock<unsigned short> indices(range);
      if (!indices)
        return false;

      for (MeshGeometry::TriangleList::const_iterator j = (*i).triangles.begin();
           j != (*i).triangles.end();
           j++)
      {
        indices[index++] = (*j).indices[0];
        indices[index++] = (*j).indices[1];
        indices[index++] = (*j).indices[2];
      }
    }
    else
    {
      GL::IndexRangeLock<unsigned int> indices(range);
      if (!indices)
        return false;

      for (MeshGeometry::TriangleList::const_iterator j = (*i).triangles.begin();
           j != (*i).triangles.end();
           j++)
      {
        indices[index++] = (*j).indices[0];
        indices[index++] = (*j).indices[1];
        indices[index++] = (*j).indices[2];
      }
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
