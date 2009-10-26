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

#include <list>

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
class Mesh : public Renderable,
             public DerivedResource<Mesh, moira::Mesh>,
	     public RefObject
{
public:
  class Geometry;
  typedef std::vector<Geometry*> GeometryList;
  ~Mesh(void);
  void enqueue(Queue& queue, const Transform3& transform) const;
  /*! @return The bounding sphere of this mesh.
   */
  const Sphere& getBounds(void) const;
  const GeometryList& getGeometries(void);
  /*! Creates a renderable mesh from the specified mesh data.
   *  @param[in] mesh The mesh data to use.
   *  @param[in] name The desired name of the created renderable mesh.
   *  @return The newly created renderable mesh, or @c NULL if an error occurred.
   */
  static Mesh* createInstance(const moira::Mesh& mesh, const String& name = "");
private:
  Mesh(const String& name);
  Mesh(const Mesh& source);
  Mesh& operator = (const Mesh& source);
  bool init(const moira::Mesh& mesh);
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
  Geometry(const GL::IndexRange& range,
           GL::PrimitiveType primitiveType,
           Material* material);
  /*! @return The range of indices used by this geometry.
   */
  const GL::IndexRange& getIndexRange(void) const;
  /*! @return The primitive mode used by this geometry.
   */
  GL::PrimitiveType getPrimitiveType(void) const;
  /*! @return The %render material used by this geometry.
   */
  Material* getMaterial(void) const;
  /*! Sets the material of this geometry.
   */
  void setMaterial(Material* newMaterial);
private:
  GL::IndexRange range;
  GL::PrimitiveType primitiveType;
  Ref<Material> material;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERMESH_H*/
///////////////////////////////////////////////////////////////////////
