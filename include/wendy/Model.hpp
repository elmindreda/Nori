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

#pragma once

#include <nori/Core.hpp>
#include <nori/Primitive.hpp>
#include <nori/Mesh.hpp>

#include <map>

namespace nori
{

/*! @brief Model section.
 *
 *  This class represents a section of triangles in a model using a single
 *  material.
 */
class ModelSection
{
public:
  /*! Constructor.
   */
  ModelSection(const IndexRange& range, Material* material);
  /*! @return The range of indices used by this geometry.
   */
  const IndexRange& indexRange() const { return m_range; }
  /*! @return The %render material used by this geometry.
   */
  Material* material() const { return m_material; }
  /*! Sets the material of this geometry.
   */
  void setMaterial(Material* newMaterial);
private:
  IndexRange m_range;
  Ref<Material> m_material;
};

/*! @brief Triangle mesh model.
 *
 *  This class represents a single model consisting of one or more
 *  sections.  Each section is a range of triangles sharing a material.
 */
class Model : public Renderable, public Resource
{
public:
  typedef std::map<std::string, Ref<Material>> MaterialMap;
  void enqueue(RenderQueue& queue,
               const Camera& camera,
               const Transform3& transform) const override;
  Sphere bounds() const override;
  /*! @return The bounding AABB of this model.
   */
  const AABB& boundingAABB() const { return m_boundingAABB; }
  /*! @return The bounding sphere of this model.
   */
  const Sphere& boundingSphere() const { return m_boundingSphere; }
  /*! @return The list of geometries in this model.
   */
  const std::vector<ModelSection>& sections() { return m_sections; }
  /*! @return The vertex buffer used by this model.
   */
  VertexBuffer& vertexBuffer() { return *m_vertexBuffer; }
  /*! @return The vertex buffer used by this model.
   */
  const VertexBuffer& vertexBuffer() const { return *m_vertexBuffer; }
  /*! @return The index buffer used by this model.
   */
  IndexBuffer& indexBuffer() { return *m_indexBuffer; }
  /*! @return The index buffer used by this model.
   */
  const IndexBuffer& indexBuffer() const { return *m_indexBuffer; }
  /*! Creates a model from the specified mesh.
   *  @param[in] info The resource info for the texture.
   *  @param[in] context The render context within which to create the texture.
   *  @param[in] data The mesh to use.
   *  @param[in] materials The materials to use.
   *  @return The newly created model, or @c nullptr if an error
   *  occurred.
   */
  static Ref<Model> create(const ResourceInfo& info,
                           RenderContext& context,
                           const Mesh& data,
                           const MaterialMap& materials);
  /*! Creates a model specification using the specified file.
   *  @param[in] context The OpenGL context within which to create the texture.
   *  @param[in] path The path of the specification file to use.
   *  @return The newly created model, or @c nullptr if an error occurred.
   */
  static Ref<Model> read(RenderContext& context, const std::string& name);
private:
  Model(const ResourceInfo& info);
  Model(const Model&) = delete;
  bool init(RenderContext& context, const Mesh& data, const MaterialMap& materials);
  Model& operator = (const Model&) = delete;
  std::vector<ModelSection> m_sections;
  Ref<VertexBuffer> m_vertexBuffer;
  Ref<IndexBuffer> m_indexBuffer;
  Sphere m_boundingSphere;
  AABB m_boundingAABB;
};

} /*namespace nori*/

