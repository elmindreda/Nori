///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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

namespace wendy
{

class AABB;
class Sphere;

class MeshTriangle
{
public:
  void setIndices(uint32 a, uint32 b, uint32 c);
  uint32 indices[3];
  vec3 normal;
};

/*! @brief Triangle mesh section.
 *
 *  A section is a set of triangles plus an associated material name.
 *  Each triangle contains indices into the vertex list of the mesh.
 */
class MeshSection
{
public:
  std::vector<MeshTriangle> triangles;
  std::string materialName;
};

/*! @brief Triangle mesh.
 *
 *  This is an ideal mesh representation intended for ease of use
 *  during calculations.  It is not intended for real-time use.
 */
class Mesh : public Resource, public RefObject
{
public:
  enum NormalType
  {
    SMOOTH_FACES,
    SEPARATE_FACES
  };
  /*! Constructor.
   *  @param name The desired name of the mesh.
   */
  Mesh(const ResourceInfo& info);
  /*! Merges all the sections in this mesh and assigns the specified material
   *  name to the resulting section.
   *  @remarks Duplicate vertices and triangles are not merged.
   */
  void mergeSections(const char* materialName);
  /*! Returns the section with the specified material name.
   */
  MeshSection* findSection(const char* materialName);
  /*! Generates and stores triangle and vertex normals for this
   *  mesh, according to the specified generation mode.
   */
  void generateNormals(NormalType type = SMOOTH_FACES);
  /*! Generates and stores triangle normals for this mesh.
   */
  void generateTriangleNormals();
  /*! Generates the bounding box of this mesh.
   */
  AABB generateBoundingAABB() const;
  /*! Generates the bounding sphere of this mesh.
   */
  Sphere generateBoundingSphere() const;
  bool write(const Path& path) const;
  /*! @return @c true if this mesh is valid, otherwise @c false.
   */
  bool isValid() const;
  /*! @return The number of triangles in all sections of this mesh.
   */
  size_t triangleCount() const;
  static Ref<Mesh> read(ResourceCache& cache, const std::string& name);
  /*! The list of sections in this mesh.
   */
  std::vector<MeshSection> sections;
  /*! The list of vertices in this mesh.
   */
  std::vector<Vertex3fn2ft3fv> vertices;
};

} /*namespace wendy*/

