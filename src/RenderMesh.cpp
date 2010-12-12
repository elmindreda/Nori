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

#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>
#include <wendy/GLState.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderQueue.h>
#include <wendy/RenderMesh.h>

#include <stdint.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

namespace
{

const unsigned int MESH_XML_VERSION = 1;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

void Mesh::enqueue(Queue& queue, const Transform3& transform) const
{
  for (GeometryList::const_iterator g = geometries.begin();  g != geometries.end();  g++)
  {
    Material* material = g->getMaterial();

    const Technique* technique = material->getActiveTechnique();
    if (!technique)
    {
      Log::writeError("Material \'%s\' has no active technique",
                      material->getPath().asString().c_str());
      return;
    }

    Operation operation;
    operation.range = GL::PrimitiveRange(GL::TRIANGLE_LIST,
                                         *vertexBuffer,
                                         g->getIndexRange());
    operation.transform = transform;
    operation.technique = technique;
    queue.addOperation(operation);
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

GL::VertexBuffer& Mesh::getVertexBuffer(void)
{
  return *vertexBuffer;
}

const GL::VertexBuffer& Mesh::getVertexBuffer(void) const
{
  return *vertexBuffer;
}

GL::IndexBuffer& Mesh::getIndexBuffer(void)
{
  return *indexBuffer;
}

const GL::IndexBuffer& Mesh::getIndexBuffer(void) const
{
  return *indexBuffer;
}

Ref<Mesh> Mesh::create(const ResourceInfo& info,
                       GL::Context& context,
                       const wendy::Mesh& data,
                       const MaterialMap& materials)
{
  Ref<Mesh> mesh(new Mesh(info, context));
  if (!mesh->init(data, materials))
    return NULL;

  return mesh;
}

Mesh::Mesh(const ResourceInfo& info, GL::Context& initContext):
  Resource(info),
  context(initContext)
{
}

Mesh::Mesh(const Mesh& source):
  Resource(source),
  context(source.context)
{
  // NOTE: Not implemented.
}

Mesh& Mesh::operator = (const Mesh& source)
{
  // NOTE: Not implemented.

  return *this;
}

bool Mesh::init(const wendy::Mesh& mesh, const MaterialMap& materials)
{
  size_t indexCount = 0;

  for (size_t i = 0;  i < mesh.geometries.size();  i++)
  {
    const String& name = mesh.geometries[i].shaderName;
    if (materials.find(name) == materials.end())
    {
      Log::writeError("Missing path for material \'%s\' of render mesh \'%s\'",
                      name.c_str(),
                      getPath().asString().c_str());
    }

    indexCount += mesh.geometries[i].triangles.size() * 3;
  }

  VertexFormat format;

  if (!format.createComponents("3f:position 3f:normal 2f:mapping"))
    return false;

  vertexBuffer = GL::VertexBuffer::create(context,
                                          (unsigned int) mesh.vertices.size(),
                                          format,
                                          GL::VertexBuffer::STATIC);
  if (!vertexBuffer)
    return false;

  vertexBuffer->copyFrom(&mesh.vertices[0], mesh.vertices.size());

  GL::IndexBuffer::Type indexType;

  if (indexCount <= (1 << 8))
    indexType = GL::IndexBuffer::UINT8;
  else if (indexCount <= (1 << 16))
    indexType = GL::IndexBuffer::UINT16;
  else
    indexType = GL::IndexBuffer::UINT32;

  indexBuffer = GL::IndexBuffer::create(context,
                                        indexCount,
                                        indexType,
                                        GL::IndexBuffer::STATIC);
  if (!indexBuffer)
    return false;

  size_t indexBase = 0;

  for (wendy::Mesh::GeometryList::const_iterator g = mesh.geometries.begin();  g != mesh.geometries.end();  g++)
  {
    indexCount = g->triangles.size() * 3;

    Ref<Material> material = Material::read(context, materials.find(g->shaderName)->second);
    if (!material)
    {
      Log::writeError("Cannot find material \'%s\' for render mesh \'%s\'",
                      g->shaderName.c_str(),
		      getPath().asString().c_str());
      return false;
    }

    GL::IndexRange range(*indexBuffer, indexBase, indexCount);

    geometries.push_back(Geometry(range, material));

    size_t index = 0;

    if (indexType == GL::IndexBuffer::UINT8)
    {
      GL::IndexRangeLock<uint8_t> indices(range);
      if (!indices)
        return false;

      for (MeshGeometry::TriangleList::const_iterator j = g->triangles.begin();
           j != g->triangles.end();
           j++)
      {
        indices[index++] = (*j).indices[0];
        indices[index++] = (*j).indices[1];
        indices[index++] = (*j).indices[2];
      }
    }
    else if (indexType == GL::IndexBuffer::UINT16)
    {
      GL::IndexRangeLock<uint16_t> indices(range);
      if (!indices)
        return false;

      for (MeshGeometry::TriangleList::const_iterator j = g->triangles.begin();
           j != g->triangles.end();
           j++)
      {
        indices[index++] = (*j).indices[0];
        indices[index++] = (*j).indices[1];
        indices[index++] = (*j).indices[2];
      }
    }
    else
    {
      GL::IndexRangeLock<uint32_t> indices(range);
      if (!indices)
        return false;

      for (MeshGeometry::TriangleList::const_iterator j = g->triangles.begin();
           j != g->triangles.end();
           j++)
      {
        indices[index++] = (*j).indices[0];
        indices[index++] = (*j).indices[1];
        indices[index++] = (*j).indices[2];
      }
    }

    indexBase += indexCount;
  }

  mesh.generateBounds(bounds);

  return true;
}

Ref<Mesh> Mesh::read(GL::Context& context, const Path& path)
{
  MeshReader reader(context);
  return reader.read(path);
}

///////////////////////////////////////////////////////////////////////

Mesh::Geometry::Geometry(const GL::IndexRange& initRange,
                         Material* initMaterial):
  range(initRange),
  material(initMaterial)
{
}

const GL::IndexRange& Mesh::Geometry::getIndexRange(void) const
{
  return range;
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

MeshReader::MeshReader(GL::Context& initContext):
  ResourceReader(initContext.getIndex()),
  context(initContext),
  info(getIndex())
{
}

Ref<Mesh> MeshReader::read(const Path& path)
{
  if (Resource* cache = getIndex().findResource(path))
    return dynamic_cast<Mesh*>(cache);

  info.path = path;

  std::ifstream stream;
  if (!open(stream, info.path))
    return NULL;

  if (!XML::Reader::read(stream))
  {
    data = NULL;
    return NULL;
  }

  if (!data)
    return NULL;

  Ref<Mesh> mesh = Mesh::create(info, context, *data, materials);
  if (!mesh)
    return NULL;

  return mesh;
}

bool MeshReader::onBeginElement(const String& name)
{
  if (name == "mesh")
  {
    const unsigned int version = readInteger("version");
    if (version != MESH_XML_VERSION)
    {
      Log::writeError("Mesh specification XML format version mismatch");
      return false;
    }

    Path dataPath(readString("data"));
    if (dataPath.isEmpty())
    {
      Log::writeError("Mesh data path for render mesh \'%s\' is empty",
                      info.path.asString().c_str());
      return false;
    }

    data = wendy::Mesh::read(getIndex(), dataPath);
    if (!data)
    {
      Log::writeError("Failed to load mesh data \'%s\' for render mesh \'%s\'",
                      dataPath.asString().c_str(),
                      info.path.asString().c_str());
      return false;
    }

    return true;
  }

  if (name == "material")
  {
    String name(readString("name"));
    if (name.empty())
    {
      Log::writeError("Empty material name in render mesh specification \'%s\'",
                      info.path.asString().c_str());
      return false;
    }

    Path path(readString("path"));
    if (path.isEmpty())
    {
      Log::writeError("Empty path for material name \'%s\' in render mesh specification \'%s\'",
                      name.c_str(),
                      info.path.asString().c_str());
      return false;
    }

    materials[name] = path;
    return true;
  }

  return true;
}

bool MeshReader::onEndElement(const String& name)
{
  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
