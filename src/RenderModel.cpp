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
#include <wendy/RenderScene.h>
#include <wendy/RenderModel.h>

#include <pugixml.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

namespace
{

const unsigned int MODEL_XML_VERSION = 2;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

void Model::enqueue(Scene& scene, const Camera& camera, const Transform3& transform) const
{
  for (GeometryList::const_iterator g = geometries.begin();  g != geometries.end();  g++)
  {
    GL::PrimitiveRange range(GL::TRIANGLE_LIST, *vertexBuffer, g->getIndexRange());

    float depth = camera.getNormalizedDepth(transform.position + bounds.center);

    scene.createOperations(transform, range, *g->getMaterial(), depth);
  }
}

const Sphere& Model::getBounds() const
{
  return bounds;
}

const Model::GeometryList& Model::getGeometries()
{
  return geometries;
}

GL::VertexBuffer& Model::getVertexBuffer()
{
  return *vertexBuffer;
}

const GL::VertexBuffer& Model::getVertexBuffer() const
{
  return *vertexBuffer;
}

GL::IndexBuffer& Model::getIndexBuffer()
{
  return *indexBuffer;
}

const GL::IndexBuffer& Model::getIndexBuffer() const
{
  return *indexBuffer;
}

Ref<Model> Model::create(const ResourceInfo& info,
                         GL::Context& context,
                         const Mesh& data,
                         const MaterialMap& materials)
{
  Ref<Model> model(new Model(info, context));
  if (!model->init(data, materials))
    return NULL;

  return model;
}

Model::Model(const ResourceInfo& info, GL::Context& initContext):
  Resource(info),
  context(initContext)
{
}

Model::Model(const Model& source):
  Resource(source),
  context(source.context)
{
  panic("Models may not be copied");
}

Model& Model::operator = (const Model& source)
{
  panic("Models may not be assigned");
}

bool Model::init(const Mesh& data, const MaterialMap& materials)
{
  /*
  if (!data.isValid())
  {
    logError("Mesh \'%s\' for model \'%s\' is not valid",
             data.getPath().asString().c_str(),
             getPath().asString().c_str());
    return false;
  }
  */

  int indexCount = 0;

  for (Mesh::GeometryList::const_iterator g = data.geometries.begin();
       g != data.geometries.end();
       g++)
  {
    if (materials.find(g->shaderName) == materials.end())
    {
      logError("Missing path for material \'%s\' of render mesh \'%s\'",
               g->shaderName.c_str(),
               getPath().asString().c_str());
      return false;
    }

    indexCount += g->triangles.size() * 3;
  }

  VertexFormat format;
  if (!format.createComponents("3f:wyPosition 3f:wyNormal 2f:wyTexCoord"))
    return false;

  vertexBuffer = GL::VertexBuffer::create(context,
                                          (unsigned int) data.vertices.size(),
                                          format,
                                          GL::VertexBuffer::STATIC);
  if (!vertexBuffer)
    return false;

  vertexBuffer->copyFrom(&data.vertices[0], data.vertices.size());

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

  int indexBase = 0;

  for (Mesh::GeometryList::const_iterator g = data.geometries.begin();
       g != data.geometries.end();
       g++)
  {
    indexCount = g->triangles.size() * 3;

    Path materialPath = materials.find(g->shaderName)->second;

    Ref<Material> material = Material::read(context, materialPath);
    if (!material)
    {
      logError("Failed to find material \'%s\' for render mesh \'%s\'",
               g->shaderName.c_str(),
               getPath().asString().c_str());
      return false;
    }

    GL::IndexRange range(*indexBuffer, indexBase, indexCount);

    geometries.push_back(Geometry(range, material));

    int index = 0;

    if (indexType == GL::IndexBuffer::UINT8)
    {
      GL::IndexRangeLock<uint8> indices(range);
      if (!indices)
        return false;

      for (MeshGeometry::TriangleList::const_iterator j = g->triangles.begin();
           j != g->triangles.end();
           j++)
      {
        indices[index++] = j->indices[0];
        indices[index++] = j->indices[1];
        indices[index++] = j->indices[2];
      }
    }
    else if (indexType == GL::IndexBuffer::UINT16)
    {
      GL::IndexRangeLock<uint16> indices(range);
      if (!indices)
        return false;

      for (MeshGeometry::TriangleList::const_iterator j = g->triangles.begin();
           j != g->triangles.end();
           j++)
      {
        indices[index++] = j->indices[0];
        indices[index++] = j->indices[1];
        indices[index++] = j->indices[2];
      }
    }
    else
    {
      GL::IndexRangeLock<uint32> indices(range);
      if (!indices)
        return false;

      for (MeshGeometry::TriangleList::const_iterator j = g->triangles.begin();
           j != g->triangles.end();
           j++)
      {
        indices[index++] = j->indices[0];
        indices[index++] = j->indices[1];
        indices[index++] = j->indices[2];
      }
    }

    indexBase += indexCount;
  }

  data.generateBounds(bounds);
  return true;
}

Ref<Model> Model::read(GL::Context& context, const Path& path)
{
  ModelReader reader(context);
  return reader.read(path);
}

///////////////////////////////////////////////////////////////////////

Model::Geometry::Geometry(const GL::IndexRange& initRange,
                          Material* initMaterial):
  range(initRange),
  material(initMaterial)
{
}

const GL::IndexRange& Model::Geometry::getIndexRange() const
{
  return range;
}

Material* Model::Geometry::getMaterial() const
{
  return material;
}

void Model::Geometry::setMaterial(Material* newMaterial)
{
  material = newMaterial;
}

///////////////////////////////////////////////////////////////////////

ModelReader::ModelReader(GL::Context& initContext):
  ResourceReader(initContext.getCache()),
  context(initContext)
{
}

Ref<Model> ModelReader::read(const Path& path)
{
  if (Resource* cached = getCache().findResource(path))
    return dynamic_cast<Model*>(cached);

  std::ifstream stream;
  if (!getCache().openFile(stream, path))
    return NULL;

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load model \'%s\': %s",
             path.asString().c_str(),
             result.description());
    return NULL;
  }

  pugi::xml_node root = document.child("model");
  if (!root || root.attribute("version").as_uint() != MODEL_XML_VERSION)
  {
    logError("Model file format mismatch in \'%s\'",
             path.asString().c_str());
    return NULL;
  }

  const Path meshPath(root.attribute("mesh").value());
  if (meshPath.isEmpty())
  {
    logError("Mesh path for model \'%s\' is empty",
              path.asString().c_str());
    return NULL;
  }

  Ref<Mesh> mesh = Mesh::read(getCache(), meshPath);
  if (!mesh)
  {
    logError("Failed to load mesh for model \'%s\'",
              path.asString().c_str());
    return NULL;
  }

  Model::MaterialMap materials;

  for (pugi::xml_node m = root.child("material");  m;  m = m.next_sibling("material"))
  {
    const String name(m.attribute("name").value());
    if (name.empty())
    {
      logError("Empty material name found in model \'%s\'",
               path.asString().c_str());
      return NULL;
    }

    const Path path(m.attribute("path").value());
    if (path.isEmpty())
    {
      logError("Empty path for material \'%s\' in model \'%s\'",
               name.c_str(),
               path.asString().c_str());
      return NULL;
    }

    materials[name] = path;
  }

  return Model::create(ResourceInfo(getCache(), path), context, *mesh, materials);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
