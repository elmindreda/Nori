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
#include <wendy/GLBuffer.h>

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
class GeometryPool;
class Camera;
class Scene;

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
class SortKey
{
public:
  static SortKey makeOpaqueKey(uint8_t layer, uint16_t state, float depth);
  static SortKey makeBlendedKey(uint8_t layer, float depth);
  union
  {
    uint64_t value;
    struct
    {
      unsigned layer : 8;
      unsigned state : 16;
      unsigned depth : 24;
      unsigned index : 16;
    };
  };
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
typedef std::vector<SortKey> SortKeyList;

///////////////////////////////////////////////////////////////////////

/*! @brief Render operation in the 3D pipeline.
 *  @ingroup renderer
 *
 *  This represents a single render operation, including render state, a
 *  primitive range and a local-to-world transformation.
 *
 *  @remarks Note that this class does not include any references to a camera.
 *  The camera transformation is handled by the Camera class.
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
  const GL::RenderState* state;
  /*! The local-to-world transformation.  Leave this set to identity if the
   *  geometry already is in world space.
   */
  Mat4 transform;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
typedef std::vector<Operation> OperationList;

///////////////////////////////////////////////////////////////////////

/*! @brief Render operation queue.
 *  @ingroup renderer
 *
 *  @remarks To avoid thrashing the heap, keep your queue objects around
 *  between frames when possible.
 *
 *  @remarks Each queue can only contain 65536 render operations.
 */
class Queue
{
public:
  /*! Constructor.
   */
  Queue(void);
  /*! Adds a render operation in this render queue.
   */
  void addOperation(const Operation& operation, SortKey key);
  /*! Destroys all render operations in this render queue.
   */
  void removeOperations(void);
  /*! @return The render operations in this render queue.
   */
  const OperationList& getOperations(void) const;
  /*! @return The sor keys in this render queue.
   */
  const SortKeyList& getSortKeys(void) const;
private:
  OperationList operations;
  mutable SortKeyList keys;
  mutable bool sorted;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
class Scene : public LightState
{
public:
  Scene(GeometryPool& pool, Technique::Type type);
  void addOperation(const Operation& operation, float depth, uint16_t layer = 0);
  void createOperations(const Transform3& transform,
                        const GL::PrimitiveRange& range,
                        const Material& material,
                        float depth);
  void removeOperations(void);
  GeometryPool& getGeometryPool(void) const;
  Queue& getOpaqueQueue(void);
  const Queue& getOpaqueQueue(void) const;
  Queue& getBlendedQueue(void);
  const Queue& getBlendedQueue(void) const;
  Technique::Type getTechniqueType(void) const;
  void setTechniqueType(Technique::Type newType);
private:
  GeometryPool& pool;
  Technique::Type type;
  Queue opaqueQueue;
  Queue blendedQueue;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Abstract renderable object.
 *  @ingroup renderer
 *
 *  This is the interface for objects able to be rendered through render
 *  operations.
 */
class Renderable
{
public:
  /*! Destructor.
   */
  virtual ~Renderable(void);
  /*! Queries this renderable for render operations.
   *  @param[in,out] scene The render scene where the operations are to
   *  be created.
   *  @param[in] camera The camera for which operations are requested.
   *  @param[in] transform The local-to-world transform.
   */
  virtual void enqueue(Scene& scene,
                       const Camera& camera,
                       const Transform3& transform) const = 0;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERQUEUE_H*/
///////////////////////////////////////////////////////////////////////
