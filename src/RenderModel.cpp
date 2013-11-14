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

#include <wendy/Config.hpp>

#include <wendy/Core.hpp>
#include <wendy/Transform.hpp>
#include <wendy/Primitive.hpp>
#include <wendy/Frustum.hpp>
#include <wendy/Camera.hpp>

#include <wendy/GLTexture.hpp>
#include <wendy/GLBuffer.hpp>
#include <wendy/GLProgram.hpp>
#include <wendy/GLContext.hpp>

#include <wendy/RenderPool.hpp>
#include <wendy/RenderState.hpp>
#include <wendy/RenderSystem.hpp>
#include <wendy/RenderMaterial.hpp>
#include <wendy/RenderScene.hpp>
#include <wendy/RenderModel.hpp>

#include <pugixml.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

namespace
{

const uint MODEL_XML_VERSION = 3;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

ModelSection::ModelSection(const GL::IndexRange& range,
                           Material* material):
  m_range(range),
  m_material(material)
{
}

void ModelSection::setMaterial(Material* newMaterial)
{
  m_material = newMaterial;
}

///////////////////////////////////////////////////////////////////////

void Model::enqueue(Scene& scene, const Camera& camera, const Transform3& transform) const
{
  for (auto& s : m_sections)
  {
    Material* material = s.material();
    if (!s.material())
      continue;

    GL::PrimitiveRange range(GL::TRIANGLE_LIST, *m_vertexBuffer, s.indexRange());

    float depth = camera.normalizedDepth(transform.position + m_boundingSphere.center);

    scene.createOperations(transform, range, *material, depth);
  }
}

Sphere Model::bounds() const
{
  return m_boundingSphere;
}

Ref<Model> Model::create(const ResourceInfo& info,
                         System& system,
                         const Mesh& data,
                         const MaterialMap& materials)
{
  Ref<Model> model(new Model(info));
  if (!model->init(system, data, materials))
    return nullptr;

  return model;
}

Model::Model(const ResourceInfo& info):
  Resource(info)
{
}

bool Model::init(System& system, const Mesh& data, const MaterialMap& materials)
{
  if (!data.isValid())
  {
    logError("Mesh %s for model %s is not valid",
             data.name().c_str(),
             name().c_str());
    return false;
  }

  for (auto& s : data.sections)
  {
    if (materials.find(s.materialName) == materials.end())
    {
      logError("Missing material %s for model %s",
               s.materialName.c_str(),
               name().c_str());
      return false;
    }
  }

  GL::Context& context = system.context();

  VertexFormat format;
  if (!format.createComponents("3f:vPosition 3f:vNormal 2f:vTexCoord"))
    return false;

  m_vertexBuffer = GL::VertexBuffer::create(context,
                                            data.vertices.size(),
                                            format,
                                            GL::USAGE_STATIC);
  if (!m_vertexBuffer)
    return false;

  m_vertexBuffer->copyFrom(&data.vertices[0], data.vertices.size());

  const size_t indexCount = data.triangleCount() * 3;

  GL::IndexBufferType indexType;
  if (indexCount <= (1 << 8))
    indexType = GL::INDEX_UINT8;
  else if (indexCount <= (1 << 16))
    indexType = GL::INDEX_UINT16;
  else
    indexType = GL::INDEX_UINT32;

  m_indexBuffer = GL::IndexBuffer::create(context,
                                          indexCount,
                                          indexType,
                                          GL::USAGE_STATIC);
  if (!m_indexBuffer)
    return false;

  size_t start = 0;

  for (auto& s : data.sections)
  {
    const size_t count = s.triangles.size() * 3;
    GL::IndexRange range(*m_indexBuffer, start, count);

    m_sections.push_back(ModelSection(range, materials.find(s.materialName)->second));

    if (indexType == GL::INDEX_UINT8)
    {
      std::vector<uint8> indices(range.count());

      size_t index = 0;

      for (auto& t : s.triangles)
      {
        indices[index++] = t.indices[0];
        indices[index++] = t.indices[1];
        indices[index++] = t.indices[2];
      }

      range.copyFrom(&indices[0]);
    }
    else if (indexType == GL::INDEX_UINT16)
    {
      std::vector<uint16> indices(range.count());

      size_t index = 0;

      for (auto& t : s.triangles)
      {
        indices[index++] = t.indices[0];
        indices[index++] = t.indices[1];
        indices[index++] = t.indices[2];
      }

      range.copyFrom(&indices[0]);
    }
    else
    {
      std::vector<uint32> indices(range.count());

      size_t index = 0;

      for (auto& t : s.triangles)
      {
        indices[index++] = t.indices[0];
        indices[index++] = t.indices[1];
        indices[index++] = t.indices[2];
      }

      range.copyFrom(&indices[0]);
    }

    start += count;
  }

  m_boundingAABB = data.generateBoundingAABB();
  m_boundingSphere = data.generateBoundingSphere();
  return true;
}

Ref<Model> Model::read(System& system, const String& name)
{
  ModelReader reader(system);
  return reader.read(name);
}

///////////////////////////////////////////////////////////////////////

ModelReader::ModelReader(System& initSystem):
  ResourceReader<Model>(initSystem.cache()),
  system(initSystem)
{
}

Ref<Model> ModelReader::read(const String& name, const Path& path)
{
  std::ifstream stream(path.name().c_str());
  if (stream.fail())
  {
    logError("Failed to open model %s", name.c_str());
    return nullptr;
  }

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load model %s: %s",
             name.c_str(),
             result.description());
    return nullptr;
  }

  pugi::xml_node root = document.child("model");
  if (!root || root.attribute("version").as_uint() != MODEL_XML_VERSION)
  {
    logError("Model file format mismatch in %s", name.c_str());
    return nullptr;
  }

  const String meshName(root.attribute("mesh").value());
  if (meshName.empty())
  {
    logError("No mesh for model %s", name.c_str());
    return nullptr;
  }

  Ref<Mesh> mesh = Mesh::read(cache, meshName);
  if (!mesh)
  {
    logError("Failed to load mesh for model %s", name.c_str());
    return nullptr;
  }

  Model::MaterialMap materials;

  for (auto m : root.children("material"))
  {
    const String materialAlias(m.attribute("alias").value());
    if (materialAlias.empty())
    {
      logError("Empty material alias found in model %s", name.c_str());
      return nullptr;
    }

    const String materialName(m.attribute("name").value());
    if (materialName.empty())
    {
      logError("Empty material name for alias %s in model %s",
               materialAlias.c_str(),
               name.c_str());
      return nullptr;
    }

    Ref<Material> material = Material::read(system, materialName);
    if (!material)
    {
      logError("Failed to load material for alias %s of model %s",
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
