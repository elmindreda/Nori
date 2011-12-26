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
#ifndef WENDY_RENDERMODEL_H
#define WENDY_RENDERMODEL_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Sphere.h>
#include <wendy/Mesh.h>

#include <map>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

/*! @brief Renderable model.
 *  @ingroup renderer
 *
 *  This class represents a single static model consisting of one or more
 *  Model::Geometry objects. Each geometry is a part of the model using a
 *  single Material and PrimitiveMode.
 */
class Model : public Renderable, public Resource
{
public:
  class Geometry;
  typedef std::vector<Geometry> GeometryList;
  typedef std::map<String, String> MaterialMap;
  void enqueue(Scene& scene, const Camera& camera, const Transform3& transform) const;
  /*! @return The bounding AABB of this model.
   */
  const AABB& getBoundingAABB() const;
  /*! @return The bounding sphere of this model.
   */
  const Sphere& getBoundingSphere() const;
  /*! @return The list of geometries in this model.
   */
  const GeometryList& getGeometries();
  /*! @return The vertex buffer used by this model.
   */
  GL::VertexBuffer& getVertexBuffer();
  /*! @return The vertex buffer used by this model.
   */
  const GL::VertexBuffer& getVertexBuffer() const;
  /*! @return The index buffer used by this model.
   */
  GL::IndexBuffer& getIndexBuffer();
  /*! @return The index buffer used by this model.
   */
  const GL::IndexBuffer& getIndexBuffer() const;
  /*! Creates a model from the specified mesh.
   *  @param[in] info The resource info for the texture.
   *  @param[in] context The OpenGL context within which to create the texture.
   *  @param[in] data The mesh to use.
   *  @return The newly created model, or @c NULL if an error
   *  occurred.
   */
  static Ref<Model> create(const ResourceInfo& info,
                           System& system,
                           const Mesh& data,
                           const MaterialMap& materials);
  /*! Creates a model specification using the specified file.
   *  @param[in] context The OpenGL context within which to create the texture.
   *  @param[in] path The path of the specification file to use.
   *  @return The newly created model, or @c NULL if an error occurred.
   */
  static Ref<Model> read(System& system, const String& name);
private:
  Model(const ResourceInfo& info);
  Model(const Model& source);
  Model& operator = (const Model& source);
  bool init(System& system, const Mesh& data, const MaterialMap& materials);
  GeometryList geometries;
  Ref<GL::VertexBuffer> vertexBuffer;
  Ref<GL::IndexBuffer> indexBuffer;
  Sphere boundingSphere;
  AABB boundingAABB;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Renderable model subset.
 *  @ingroup renderer
 *
 *  This class represents a subset of a model using a single %render material
 *  and a single primitive mode.
 */
class Model::Geometry
{
public:
  /*! Constructor.
   */
  Geometry(const GL::IndexRange& range, Material* material);
  /*! @return The range of indices used by this geometry.
   */
  const GL::IndexRange& getIndexRange() const;
  /*! @return The %render material used by this geometry.
   */
  Material* getMaterial() const;
  /*! Sets the material of this geometry.
   */
  void setMaterial(Material* newMaterial);
private:
  GL::IndexRange range;
  Ref<Material> material;
};

///////////////////////////////////////////////////////////////////////

class ModelReader : public ResourceReader<Model>
{
public:
  ModelReader(System& system);
  using ResourceReader::read;
  Ref<Model> read(const String& name, const Path& path);
private:
  System& system;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERMODEL_H*/
///////////////////////////////////////////////////////////////////////
