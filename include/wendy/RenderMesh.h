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
#ifndef WENDY_RENDERMESH_H
#define WENDY_RENDERMESH_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Vector.h>
#include <wendy/Sphere.h>
#include <wendy/Mesh.h>

#include <map>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

/*! @brief Renderable mesh object.
 *  @ingroup renderer
 *
 *  This class represents a single static mesh, consisting of one or more
 *  Mesh::Geometry objects. Each geometry is a part of the mesh using a single
 *  render material and primitive mode.
 */
class Mesh : public Renderable, public Resource
{
public:
  class Geometry;
  typedef std::vector<Geometry> GeometryList;
  typedef std::map<String, Path> MaterialMap;
  void enqueue(Queue& queue, const Transform3& transform) const;
  /*! @return The bounding sphere of this mesh.
   */
  const Sphere& getBounds(void) const;
  /*! @return The list of geometries in this mesh.
   */
  const GeometryList& getGeometries(void);
  /*! @return The vertex buffer used by this mesh.
   */
  GL::VertexBuffer& getVertexBuffer(void);
  /*! @return The vertex buffer used by this mesh.
   */
  const GL::VertexBuffer& getVertexBuffer(void) const;
  /*! @return The index buffer used by this mesh.
   */
  GL::IndexBuffer& getIndexBuffer(void);
  /*! @return The index buffer used by this mesh.
   */
  const GL::IndexBuffer& getIndexBuffer(void) const;
  /*! Creates a renderable mesh from the specified mesh data.
   *  @param[in] info The resource info for the texture.
   *  @param[in] context The OpenGL context within which to create the texture.
   *  @param[in] data The mesh data to use.
   *  @return The newly created renderable mesh, or @c NULL if an error
   *  occurred.
   */
  static Ref<Mesh> create(const ResourceInfo& info,
                          GL::Context& context,
                          const wendy::Mesh& data,
                          const MaterialMap& materials);
  /*! Creates a renderable mesh specification using the specified file.
   *  @param[in] context The OpenGL context within which to create the texture.
   *  @param[in] path The path of the specification file to use.
   *  @return The newly created renderable mesh, or @c NULL if an error
   *  occurred.
   */
  static Ref<Mesh> read(GL::Context& context, const Path& path);
private:
  Mesh(const ResourceInfo& info, GL::Context& context);
  Mesh(const Mesh& source);
  Mesh& operator = (const Mesh& source);
  bool init(const wendy::Mesh& mesh, const MaterialMap& materials);
  GL::Context& context;
  GeometryList geometries;
  Ref<GL::VertexBuffer> vertexBuffer;
  Ref<GL::IndexBuffer> indexBuffer;
  Sphere bounds;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Renderable mesh subset.
 *  @ingroup renderer
 *
 *  This class represents a subset of a mesh, using a single %render material
 *  and a single primitive mode.
 */
class Mesh::Geometry
{
public:
  /*! Constructor.
   */
  Geometry(const GL::IndexRange& range, Material* material);
  /*! @return The range of indices used by this geometry.
   */
  const GL::IndexRange& getIndexRange(void) const;
  /*! @return The %render material used by this geometry.
   */
  Material* getMaterial(void) const;
  /*! Sets the material of this geometry.
   */
  void setMaterial(Material* newMaterial);
private:
  GL::IndexRange range;
  Ref<Material> material;
};

///////////////////////////////////////////////////////////////////////

class MeshReader : public ResourceReader, public XML::Reader
{
public:
  MeshReader(GL::Context& context);
  Ref<Mesh> read(const Path& path);
private:
  bool onBeginElement(const String& name);
  bool onEndElement(const String& name);
  GL::Context& context;
  ResourceInfo info;
  Ref<wendy::Mesh> data;
  Mesh::MaterialMap materials;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERMESH_H*/
///////////////////////////////////////////////////////////////////////
