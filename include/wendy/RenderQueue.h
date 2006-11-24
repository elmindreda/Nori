///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_RENDERQUEUE_H
#define WENDY_RENDERQUEUE_H
///////////////////////////////////////////////////////////////////////

#include <list>
#include <vector>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @defgroup renderer 3D rendering pipeline
 *
 *  These classes make up the core 3D renderering pipeline.
 *
 *  It also provides some basic high-level rendering primitives such as
 *  multi-style geometry meshes, a simple terrain renderer and an extensible
 *  particle system.
 */

///////////////////////////////////////////////////////////////////////

/*! @brief Render operation in the 3D pipeline.
 *  @ingroup renderer
 *
 *  This represents a single render operation, including a style and
 *  local-to-world transformation.
 *
 *  @remarks Note that this class does not include any references to a camera.
 *  The camera transformation is handled by the Camera class, and the
 *  Queue to which your Operation belongs contains a reference to
 *  the correct Camera.
 */
class Operation
{
  friend class Group;
public:
  /*! Constructor.
   */
  Operation(void);
  /*! Comparison operator to enable sorting.
   *  @param other The object to compare to.
   */
  bool operator < (const Operation& other) const;
  /*! The vertex buffer that contains the geometry to render.
   */
  const GL::VertexBuffer* vertexBuffer;
  /*! The index buffer that references the vertex buffer, or @c NULL
   *  if no index buffer is to be used.
   */
  const GL::IndexBuffer* indexBuffer;
  /*! The render technique to use.
   */
  const Technique* technique;
  /*! If an index buffer is set, this is the first element of the range of
   *  indices to be used for rendering.  Otherwise, it is the first vertex in
   *  the vertex buffer to be used for rendering.
   */
  unsigned int start;
  /*! If an index buffer is set, this is the number of elements in the index
   *  buffer to be used for rendering.  Otherwise, it is the number of vertices
   *  in the vertex buffer to be used for rendering.
   */
  unsigned int count;
  /*! The local-to-world transformation. Leave this set to identity if the
   *  geometry already is in world space.
   */
  Matrix4 transform;
  /*! The distance from the camera to the geometry rendered by this operation.
   *  It must be set for blending operations, but should also be set for opaque
   *  operations.
   */
  float distance;
  /*! The geometry rendering mode to use.
   */
  GLenum renderMode;
private:
  mutable bool blending;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Sorted list of render operations.
 *  @ingroup renderer
 */
typedef std::vector<const Operation*> OperationList;

///////////////////////////////////////////////////////////////////////

/*! @brief Render operation group.
 */
class Group
{
public:
  Group(GL::Light* light = NULL);
  Operation& createOperation(void);
  void destroyOperations(void);
  GL::Light* getLight(void) const;
  const OperationList& getOperations(void) const;
private:
  typedef std::list<Operation> List;
  List operations;
  mutable OperationList sortedOperations;
  mutable bool sorted;
  GL::Light* light;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Render operation queue for the 3D pipeline.
 *  @ingroup renderer
 *
 *  This class is a container for all data needed to render a scene, including
 *  separate render operation groups for each light, and can therefore be seen
 *  as a higher-level rendering operation.
 */
class Queue
{
public:
  Queue(const Camera& camera);
  void attachLight(GL::Light& light);
  void detachLights(void);
  Operation& createOperation(void);
  Operation& createLightOperation(GL::Light& light);
  void destroyOperations(void);
  const Camera& getCamera(void) const;
  unsigned int getLightCount(void) const;
  Group& getDefaultGroup(void);
  const Group& getDefaultGroup(void) const;
  Group& getLightGroup(unsigned int index);
  const Group& getLightGroup(unsigned int index) const;
private:
  Group* findGroup(GL::Light& light);
  const Group* findGroup(GL::Light& light) const;
  typedef std::list<Group> GroupList;
  GroupList lightGroups;
  Group defaultGroup;
  const Camera& camera;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Abstract renderable object.
 *  @ingroup renderer
 *
 *  This is the interface for enqueueable, non-scene renderables. It provides a
 *  simple mechanism for retrieving render operations either in local or world
 *  space.
 */
class Renderable
{
public:
  /*! Destructor.
   */
  virtual ~Renderable(void);
  /*! Queries the renderable for render operations.
   *  @param[in,out] queue The render queue where the operations are to
   *  be created.
   *  @param[in] transform The local-to-world transform.
   */
  virtual void enqueue(Queue& queue, const Transform3& transform) const = 0;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERQUEUE_H*/
///////////////////////////////////////////////////////////////////////
