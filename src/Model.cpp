///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

#include <nori/Config.hpp>

#include <nori/Core.hpp>
#include <nori/Transform.hpp>
#include <nori/Primitive.hpp>
#include <nori/Frustum.hpp>
#include <nori/Camera.hpp>

#include <nori/Texture.hpp>
#include <nori/RenderBuffer.hpp>
#include <nori/Program.hpp>
#include <nori/RenderContext.hpp>

#include <nori/Pass.hpp>
#include <nori/Material.hpp>
#include <nori/RenderQueue.hpp>
#include <nori/Model.hpp>

#include <pugixml.hpp>

namespace nori
{

namespace
{

IndexType smallestIndexType(uint indexCount)
{
  if (indexCount <= (1 << 8))
    return INDEX_UINT8;
  else if (indexCount <= (1 << 16))
    return INDEX_UINT16;
  else
    return INDEX_UINT32;
}

const uint MODEL_XML_VERSION = 3;

} /*namespace*/

ModelSection::ModelSection(const PrimitiveRange& range,
                           Material* material):
  m_range(range),
  m_material(material)
{
}

void ModelSection::setMaterial(Material* newMaterial)
{
  m_material = newMaterial;
}

void Model::enqueue(RenderQueue& queue, const Camera& camera, const Transform3& transform) const
{
  for (const ModelSection& s : m_sections)
  {
    Material* material = s.material();
    if (!material)
      continue;

    const float depth = camera.normalizedDepth(transform.position +
                                               m_boundingSphere.center);

    queue.createOperations(transform, s.range(), *material, depth);
  }
}

Sphere Model::bounds() const
{
  return m_boundingSphere;
}

Ref<Model> Model::create(const ResourceInfo& info,
                         RenderContext& context,
                         const Mesh& data,
                         const MaterialMap& materials)
{
  Ref<Model> model(new Model(info));
  if (!model->init(context, data, materials))
    return nullptr;

  return model;
}

Model::Model(const ResourceInfo& info):
  Resource(info)
{
}

bool Model::init(RenderContext& context, const Mesh& data, const MaterialMap& materials)
{
  if (!data.isValid())
  {
    logError("Mesh %s for model %s is not valid",
             data.name().c_str(),
             name().c_str());
    return false;
  }

  for (const MeshSection& s : data.sections)
  {
    if (materials.find(s.materialName) == materials.end())
    {
      logError("Missing material %s for model %s",
               s.materialName.c_str(),
               name().c_str());
      return false;
    }
  }

  const size_t vertexBufferSize = data.vertices.size() *
                                  Vertex3fn2ft3fv::format.size();
  m_vertexBuffer = Buffer::create(context,
                                  VERTEX_BUFFER,
                                  vertexBufferSize,
                                  USAGE_STATIC);
  if (!m_vertexBuffer)
    return false;

  m_vertexBuffer->copyFrom(data.vertices.data(), vertexBufferSize);

  const size_t indexCount = data.triangleCount() * 3;
  const IndexType indexType = smallestIndexType(indexCount);
  const size_t indexSize = getIndexTypeSize(indexType);
  m_indexBuffer = Buffer::create(context, INDEX_BUFFER, indexSize * indexCount, USAGE_STATIC);
  if (!m_indexBuffer)
    return false;

  size_t start = 0;

  for (const MeshSection& s : data.sections)
  {
    const size_t count = s.triangles.size() * 3;
    PrimitiveRange range(TRIANGLE_LIST, indexType, start, count);

    m_sections.push_back(ModelSection(range, materials.find(s.materialName)->second));

    if (indexType == INDEX_UINT8)
    {
      std::vector<uint8> indices(range.count);

      size_t index = 0;

      for (const MeshTriangle& t : s.triangles)
      {
        indices[index++] = t.indices[0];
        indices[index++] = t.indices[1];
        indices[index++] = t.indices[2];
      }

      m_indexBuffer->copyFrom(indices.data(), indexSize * count, indexSize * start);
    }
    else if (indexType == INDEX_UINT16)
    {
      std::vector<uint16> indices(range.count);

      size_t index = 0;

      for (const MeshTriangle& t : s.triangles)
      {
        indices[index++] = t.indices[0];
        indices[index++] = t.indices[1];
        indices[index++] = t.indices[2];
      }

      m_indexBuffer->copyFrom(indices.data(), indexSize * count, indexSize * start);
    }
    else
    {
      std::vector<uint32> indices(range.count);

      size_t index = 0;

      for (const MeshTriangle& t : s.triangles)
      {
        indices[index++] = t.indices[0];
        indices[index++] = t.indices[1];
        indices[index++] = t.indices[2];
      }

      m_indexBuffer->copyFrom(indices.data(), indexSize * count, indexSize * start);
    }

    start += count;
  }

  m_boundingAABB = data.generateBoundingAABB();
  m_boundingSphere = data.generateBoundingSphere();
  return true;
}

Ref<Model> Model::read(RenderContext& context, const std::string& name)
{
  if (Model* cached = context.cache().find<Model>(name))
    return cached;

  const Path path = context.cache().findFile(name);
  if (path.isEmpty())
  {
    logError("Failed to find model %s", name.c_str());
    return nullptr;
  }

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load_file(path.name().c_str());
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

  const std::string meshName(root.attribute("mesh").value());
  if (meshName.empty())
  {
    logError("No mesh for model %s", name.c_str());
    return nullptr;
  }

  Ref<Mesh> mesh = Mesh::read(context.cache(), meshName);
  if (!mesh)
  {
    logError("Failed to load mesh for model %s", name.c_str());
    return nullptr;
  }

  Model::MaterialMap materials;

  for (auto m : root.children("material"))
  {
    const std::string materialAlias(m.attribute("alias").value());
    if (materialAlias.empty())
    {
      logError("Empty material alias found in model %s", name.c_str());
      return nullptr;
    }

    const std::string materialName(m.attribute("name").value());
    if (materialName.empty())
    {
      logError("Empty material name for alias %s in model %s",
               materialAlias.c_str(),
               name.c_str());
      return nullptr;
    }

    Ref<Material> material = Material::read(context, materialName);
    if (!material)
    {
      logError("Failed to load material for alias %s of model %s",
               materialAlias.c_str(),
               materialName.c_str());
      return nullptr;
    }

    materials[materialAlias] = material;
  }

  return create(ResourceInfo(context.cache(), name, path),
                context, *mesh, materials);
}

} /*namespace nori*/

