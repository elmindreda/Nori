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

#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>
#include <wendy/GLState.h>

#include <wendy/RenderPool.h>
#include <wendy/RenderSystem.h>
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

const unsigned int MODEL_XML_VERSION = 3;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

ModelSection::ModelSection(const GL::IndexRange& initRange,
                           Material* initMaterial):
  range(initRange),
  material(initMaterial)
{
}

const GL::IndexRange& ModelSection::getIndexRange() const
{
  return range;
}

Material* ModelSection::getMaterial() const
{
  return material;
}

void ModelSection::setMaterial(Material* newMaterial)
{
  material = newMaterial;
}

///////////////////////////////////////////////////////////////////////

void Model::enqueue(Scene& scene, const Camera& camera, const Transform3& transform) const
{
  for (ModelSectionList::const_iterator s = sections.begin();  s != sections.end();  s++)
  {
    Material* material = s->getMaterial();
    if (!material)
      continue;

    GL::PrimitiveRange range(GL::TRIANGLE_LIST, *vertexBuffer, s->getIndexRange());

    float depth = camera.getNormalizedDepth(transform.position + boundingSphere.center);

    scene.createOperations(transform, range, *material, depth);
  }
}

const AABB& Model::getBoundingAABB() const
{
  return boundingAABB;
}

const Sphere& Model::getBoundingSphere() const
{
  return boundingSphere;
}

const ModelSectionList& Model::getSections()
{
  return sections;
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
                         System& system,
                         const Mesh& data,
                         const MaterialMap& materials)
{
  Ref<Model> model(new Model(info));
  if (!model->init(system, data, materials))
    return NULL;

  return model;
}

Model::Model(const ResourceInfo& info):
  Resource(info)
{
}

Model::Model(const Model& source):
  Resource(source)
{
  panic("Models may not be copied");
}

Model& Model::operator = (const Model& source)
{
  panic("Models may not be assigned");
}

bool Model::init(System& system, const Mesh& data, const MaterialMap& materials)
{
  if (!data.isValid())
  {
    logError("Mesh \'%s\' for model \'%s\' is not valid",
             data.getName().c_str(),
             getName().c_str());
    return false;
  }

  for (MeshSectionList::const_iterator s = data.sections.begin();
       s != data.sections.end();
       s++)
  {
    if (materials.find(s->materialName) == materials.end())
    {
      logError("Missing material \'%s\' for model \'%s\'",
               s->materialName.c_str(),
               getName().c_str());
      return false;
    }
  }

  GL::Context& context = system.getContext();

  VertexFormat format;
  if (!format.createComponents("3f:wyPosition 3f:wyNormal 2f:wyTexCoord"))
    return false;

  vertexBuffer = GL::VertexBuffer::create(context,
                                          data.vertices.size(),
                                          format,
                                          GL::VertexBuffer::STATIC);
  if (!vertexBuffer)
    return false;

  vertexBuffer->copyFrom(&data.vertices[0], data.vertices.size());

  const size_t indexCount = data.getTriangleCount() * 3;

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

  size_t start = 0;

  for (MeshSectionList::const_iterator s = data.sections.begin();
       s != data.sections.end();
       s++)
  {
    const size_t count = s->triangles.size() * 3;
    GL::IndexRange range(*indexBuffer, start, count);

    sections.push_back(ModelSection(range, materials.find(s->materialName)->second));

    if (indexType == GL::IndexBuffer::UINT8)
    {
      GL::IndexRangeLock<uint8> indices(range);
      if (!indices)
        return false;

      size_t index = 0;

      for (MeshTriangleList::const_iterator t = s->triangles.begin();
           t != s->triangles.end();
           t++)
      {
        indices[index++] = t->indices[0];
        indices[index++] = t->indices[1];
        indices[index++] = t->indices[2];
      }
    }
    else if (indexType == GL::IndexBuffer::UINT16)
    {
      GL::IndexRangeLock<uint16> indices(range);
      if (!indices)
        return false;

      size_t index = 0;

      for (MeshTriangleList::const_iterator t = s->triangles.begin();
           t != s->triangles.end();
           t++)
      {
        indices[index++] = t->indices[0];
        indices[index++] = t->indices[1];
        indices[index++] = t->indices[2];
      }
    }
    else
    {
      GL::IndexRangeLock<uint32> indices(range);
      if (!indices)
        return false;

      size_t index = 0;

      for (MeshTriangleList::const_iterator t = s->triangles.begin();
           t != s->triangles.end();
           t++)
      {
        indices[index++] = t->indices[0];
        indices[index++] = t->indices[1];
        indices[index++] = t->indices[2];
      }
    }

    start += count;
  }

  boundingAABB = data.generateBoundingAABB();
  boundingSphere = data.generateBoundingSphere();
  return true;
}

Ref<Model> Model::read(System& system, const String& name)
{
  ModelReader reader(system);
  return reader.read(name);
}

///////////////////////////////////////////////////////////////////////

ModelReader::ModelReader(System& initSystem):
  ResourceReader<Model>(initSystem.getCache()),
  system(initSystem)
{
}

Ref<Model> ModelReader::read(const String& name, const Path& path)
{
  std::ifstream stream(path.asString().c_str());
  if (stream.fail())
  {
    logError("Failed to open model \'%s\'", name.c_str());
    return NULL;
  }

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load model \'%s\': %s",
             name.c_str(),
             result.description());
    return NULL;
  }

  pugi::xml_node root = document.child("model");
  if (!root || root.attribute("version").as_uint() != MODEL_XML_VERSION)
  {
    logError("Model file format mismatch in \'%s\'", name.c_str());
    return NULL;
  }

  const String meshName(root.attribute("mesh").value());
  if (meshName.empty())
  {
    logError("No mesh for model \'%s\'", name.c_str());
    return NULL;
  }

  Ref<Mesh> mesh = Mesh::read(cache, meshName);
  if (!mesh)
  {
    logError("Failed to load mesh for model \'%s\'", name.c_str());
    return NULL;
  }

  Model::MaterialMap materials;

  for (pugi::xml_node m = root.child("material");  m;  m = m.next_sibling("material"))
  {
    const String materialAlias(m.attribute("alias").value());
    if (materialAlias.empty())
    {
      logError("Empty material alias found in model \'%s\'", name.c_str());
      return NULL;
    }

    const String materialName(m.attribute("name").value());
    if (materialName.empty())
    {
      logError("Empty material name for alias \'%s\' in model \'%s\'",
               materialAlias.c_str(),
               name.c_str());
      return NULL;
    }

    Ref<Material> material = Material::read(system, materialName);
    if (!material)
    {
      logError("Failed to load material for alias \'%s\' of model \'%s\'",
               materialAlias.c_str(),
               materialName.c_str());
    }

    materials[materialAlias] = material;
  }

  return Model::create(ResourceInfo(cache, name, path), system, *mesh, materials);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
