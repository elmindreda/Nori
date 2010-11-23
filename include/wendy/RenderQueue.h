///////////////////////////////////////////////////////////////////////
// Wendy default renderer
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

#include <wendy/GLContext.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>

#include <list>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

/*! @defgroup renderer 3D rendering pipeline
 *
 *  These classes make up the core 3D renderering pipeline.
 *
 *  It also provides some basic high-level rendering primitives such as
 *  multi-material geometry meshes and an extensible particle system.
 */

///////////////////////////////////////////////////////////////////////

class Light;
class LightState;

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
 *  the camera being used.
 */
class Operation
{
public:
  /*! Constructor.
   */
  Operation(void);
  /*! The primitive range to render.
   */
  GL::PrimitiveRange range;
  /*! The render technique to use.
   */
  const Technique* technique;
  /*! The local-to-world transformation. Leave this set to identity if the
   *  geometry already is in world space.
   */
  Mat4 transform;
  /*! The distance from the camera to the geometry rendered by this operation.
   *  It must be set for blending operations, but should also be set for opaque
   *  operations.
   */
  float distance;
  /*! Hash value for sorting. Used by the Queue class.
   */
  unsigned int hash;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Sorted list of render operations.
 *  @ingroup renderer
 */
typedef std::vector<const Operation*> OperationList;

///////////////////////////////////////////////////////////////////////

/*! @brief Render operation queue.
 *  @ingroup renderer
 *
 *  @remarks To avoid thrashing the heap, keep your queue objects around
 *  between frames when possible.
 */
class Queue
{
public:
  /*! Queue phase type enumeration.
  */
  enum Phase
  {
    /*! Light collection phase.
    */
    COLLECT_LIGHTS,
    /*! Collection phase for operations used to render a shadow map.
    */
    COLLECT_SHADOW_CASTERS,
    /*! Geometry collection phase.
     *  This is the default phase.
    */
    COLLECT_GEOMETRY,
  };
  /*! Constructor. Creates a render queue for the specified camera, optionally
   *  with the specified light and name.
   *  @param[in] camera The camera to use.
   *  @param[in] light The light to collect geometry for, or @c NULL to not
   *  collect for a light.
   */
  Queue(const Camera& camera, Light* light = NULL);
  /*! Attaches a light to this render queue.
   *  @param[in] light The light to attach.
   */
  void attachLight(Light& light);
  /*! Detaches all attached lights from this render queue.
   */
  void detachLights(void);
  /*! Creates a render operation in this render queue.
   *  @return The created operation.
   */
  void addOperation(const Operation& operation);
  /*! Destroys all render operations in this render queue.
   */
  void removeOperations(void);
  /*! Renders the operations in this render queue, using the specified camera
   *  and the attached lights.
   *  @param[in] passName The name of render passes to use.
   */
  void render(const String& passName = "") const;
  /*! @return The current colleciton phase of this render queue.
   */
  Phase getPhase(void) const;
  /*! Sets the collection phase of this render queue.
   *  @param[in] newPhase The desired collection phase.
   */
  void setPhase(Phase newPhase);
  /*! @return The camera used by this render queue.
   */
  const Camera& getCamera(void) const;
  /*! @return The light for which to collect geometry, or @c NULL if not
   *  collecting geometry for a light.
   */
  Light* getActiveLight(void) const;
  /*! @return The opaque render operations in this render queue.
   */
  const OperationList& getOpaqueOperations(void) const;
  /*! @return The blended render operations in this render queue.
   */
  const OperationList& getBlendedOperations(void) const;
  /*! @return The lights attached to this render queue.
   */
  const LightState& getLights(void) const;
private:
  typedef std::vector<Operation> List;
  void renderOperations(const OperationList& operations,
                        const String& passName) const;
  const Camera& camera;
  Light* light;
  LightState lights;
  List opaqueOperations;
  List blendedOperations;
  Phase phase;
  mutable OperationList sortedOpaqueOps;
  mutable OperationList sortedBlendedOps;
  mutable bool sortedOpaque;
  mutable bool sortedBlended;
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
  /*! Queries this renderable for render operations.
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
