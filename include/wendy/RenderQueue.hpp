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
#ifndef WENDY_RENDERQUEUE_HPP
#define WENDY_RENDERQUEUE_HPP
///////////////////////////////////////////////////////////////////////

#include <wendy/RenderContext.hpp>
#include <wendy/Texture.hpp>
#include <wendy/RenderBuffer.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class RenderQueue;

///////////////////////////////////////////////////////////////////////

/*! @brief Abstract renderable object.
 *  @ingroup renderer
 *
 *  This is the interface for objects able to be rendered through render
 *  operations.
 */
class Renderable : public RefObject
{
public:
  /*! Destructor.
   */
  virtual ~Renderable();
  /*! Queries this renderable for render operations.
   *  @param[in,out] queue The render queue where the operations are to
   *  be created.
   *  @param[in] camera The camera for which operations are requested.
   *  @param[in] transform The local-to-world transform.
   */
  virtual void enqueue(RenderQueue& queue,
                       const Camera& camera,
                       const Transform3& transform) const = 0;
  /*! Returns the local space bounds of this renderable.
   */
  virtual Sphere bounds() const = 0;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
enum LightType
{
  DIRECTIONAL,
  POINT,
  SPOTLIGHT
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
struct LightData
{
  LightType type;
  float radius;
  vec3 color;
  vec3 position;
  vec3 direction;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
class Light : public Renderable
{
public:
  Light();
  void enqueue(RenderQueue& queue,
               const Camera& camera,
               const Transform3& transform) const override;
  Sphere bounds() const override;
  LightType type() const { return m_type; }
  void setType(LightType newType);
  float radius() const { return m_radius; }
  void setRadius(float newRadius);
  const vec3& color() const { return m_color; }
  void setColor(const vec3& newColor);
private:
  LightType m_type;
  float m_radius;
  vec3 m_color;
};

///////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

/*! @ingroup renderer
 */
class RenderOpKey
{
public:
  static RenderOpKey makeOpaqueKey(uint8 layer, uint16 state, float depth);
  static RenderOpKey makeBlendedKey(uint8 layer, float depth);
  RenderOpKey(): value(0) { }
  RenderOpKey(uint64 value): value(value) { }
  operator uint64 () const { return value; }
  union
  {
    uint64 value;
    struct
    {
      unsigned index : 16;
      unsigned depth : 24;
      unsigned state : 16;
      unsigned layer : 8;
    };
  };
};

#pragma pack(pop)

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
class RenderOp
{
public:
  /*! Constructor.
   */
  RenderOp();
  /*! The primitive range to render.
   */
  PrimitiveRange range;
  /*! The render technique to use.
   */
  const Pass* state;
  /*! The local-to-world transformation.  Leave this set to identity if the
   *  geometry already is in world space.
   */
  mat4 transform;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Render operation bucket.
 *  @ingroup renderer
 *
 *  @remarks Each queue can only contain 65536 render operations.
 */
class RenderBucket
{
public:
  /*! Constructor.
   */
  RenderBucket();
  /*! Adds a render operation in this render queue.
   */
  void addOperation(const RenderOp& operation, RenderOpKey key);
  /*! Destroys all render operations in this render queue.
   */
  void removeOperations();
  /*! @return The render operations in this render queue.
   */
  const std::vector<RenderOp>& operations() const { return m_operations; }
  /*! @return The sor keys in this render queue.
   */
  const std::vector<uint64>& keys() const;
private:
  std::vector<RenderOp> m_operations;
  mutable std::vector<uint64> m_keys;
  mutable bool m_sorted;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Render operation queue.
 *  @ingroup renderer
 *
 *  @remarks To avoid thrashing the heap, keep your bucket objects around
 *  between frames when possible.
 */
class RenderQueue
{
public:
  RenderQueue(RenderContext& context, RenderPhase phase = RENDER_DEFAULT);
  void addOperation(const RenderOp& operation, float depth, uint8 layer = 0);
  void createOperations(const mat4& transform,
                        const PrimitiveRange& range,
                        const Material& material,
                        float depth);
  void removeOperations();
  void addLight(const LightData& light);
  void removeLights();
  const std::vector<LightData>& lights() const { return m_lights; }
  const vec3& ambientIntensity() const { return m_ambient; }
  void setAmbientIntensity(const vec3& newIntensity);
  RenderContext& context() const { return m_context; }
  RenderBucket& opaqueBucket() { return m_opaqueBucket; }
  const RenderBucket& opaqueBucket() const { return m_opaqueBucket; }
  RenderBucket& blendedBucket() { return m_blendedBucket; }
  const RenderBucket& blendedBucket() const { return m_blendedBucket; }
  RenderPhase phase() const { return m_phase; }
  void setPhase(RenderPhase newPhase);
private:
  RenderContext& m_context;
  RenderPhase m_phase;
  RenderBucket m_opaqueBucket;
  RenderBucket m_blendedBucket;
  std::vector<LightData> m_lights;
  vec3 m_ambient;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERQUEUE_HPP*/
///////////////////////////////////////////////////////////////////////
