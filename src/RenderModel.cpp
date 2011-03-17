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
#include <wendy/RenderModel.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

namespace
{

const unsigned int MODEL_XML_VERSION = 1;

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

const Sphere& Model::getBounds(void) const
{
  return bounds;
}

const Model::GeometryList& Model::getGeometries(void)
{
  return geometries;
}

GL::VertexBuffer& Model::getVertexBuffer(void)
{
  return *vertexBuffer;
}

const GL::VertexBuffer& Model::getVertexBuffer(void) const
{
  return *vertexBuffer;
}

GL::IndexBuffer& Model::getIndexBuffer(void)
{
  return *indexBuffer;
}

const GL::IndexBuffer& Model::getIndexBuffer(void) const
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
  // NOTE: Not implemented.
}

Model& Model::operator = (const Model& source)
{
  // NOTE: Not implemented.

  return *this;
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
  if (!format.createComponents("3f:vertex.position 3f:vertex.normal 2f:vertex.mapping"))
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
      logError("Cannot find material \'%s\' for render mesh \'%s\'",
               g->shaderName.c_str(),
	       getPath().asString().c_str());
      return false;
    }

    GL::IndexRange range(*indexBuffer, indexBase, indexCount);

    geometries.push_back(Geometry(range, material));

    int index = 0;

    if (indexType == GL::IndexBuffer::UINT8)
    {
      GL::IndexRangeLock<uint8_t> indices(range);
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
      GL::IndexRangeLock<uint16_t> indices(range);
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
      GL::IndexRangeLock<uint32_t> indices(range);
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

const GL::IndexRange& Model::Geometry::getIndexRange(void) const
{
  return range;
}

Material* Model::Geometry::getMaterial(void) const
{
  return material;
}

void Model::Geometry::setMaterial(Material* newMaterial)
{
  material = newMaterial;
}

///////////////////////////////////////////////////////////////////////

ModelReader::ModelReader(GL::Context& initContext):
  ResourceReader(initContext.getIndex()),
  context(initContext),
  info(getIndex())
{
}

Ref<Model> ModelReader::read(const Path& path)
{
  if (Resource* cache = getIndex().findResource(path))
    return dynamic_cast<Model*>(cache);

  materials.clear();
  info.path = path;

  std::ifstream stream;
  if (!getIndex().openFile(stream, info.path))
    return NULL;

  if (!XML::Reader::read(stream))
  {
    data = NULL;
    return NULL;
  }

  if (!data)
    return NULL;

  Ref<Model> mesh = Model::create(info, context, *data, materials);
  if (!mesh)
    return NULL;

  data = NULL;
  return mesh;
}

bool ModelReader::onBeginElement(const String& name)
{
  if (name == "mesh")
  {
    const unsigned int version = readInteger("version");
    if (version != MODEL_XML_VERSION)
    {
      logError("Model specification XML format version mismatch");
      return false;
    }

    Path dataPath(readString("data"));
    if (dataPath.isEmpty())
    {
      logError("Model data path for render mesh \'%s\' is empty",
               info.path.asString().c_str());
      return false;
    }

    data = Mesh::read(getIndex(), dataPath);
    if (!data)
    {
      logError("Failed to load mesh data \'%s\' for render mesh \'%s\'",
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
      logError("Empty material name in render mesh specification \'%s\'",
               info.path.asString().c_str());
      return false;
    }

    Path path(readString("path"));
    if (path.isEmpty())
    {
      logError("Empty path for material name \'%s\' in render mesh specification \'%s\'",
               name.c_str(),
               info.path.asString().c_str());
      return false;
    }

    materials[name] = path;
    return true;
  }

  return true;
}

bool ModelReader::onEndElement(const String& name)
{
  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
