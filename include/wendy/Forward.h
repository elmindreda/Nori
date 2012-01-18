///////////////////////////////////////////////////////////////////////
// Wendy forward renderer
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_FORWARD_H
#define WENDY_FORWARD_H
///////////////////////////////////////////////////////////////////////

#include <wendy/RenderPool.h>
#include <wendy/RenderSystem.h>
#include <wendy/RenderState.h>
#include <wendy/RenderScene.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace forward
  {

///////////////////////////////////////////////////////////////////////

/*! @brief Shared program state for the forward renderer.
 *  @ingroup renderer
 */
class SharedProgramState : public render::SharedProgramState
{
};

///////////////////////////////////////////////////////////////////////

/*! @brief Forward renderer configuration.
 *  @ingroup renderer
 */
class Config
{
public:
  /*! Constructor.
   *  @param[in] pool The geometry pool to use.
   */
  Config(render::GeometryPool& pool);
  /*! The geometry pool to be used by the renderer.
   */
  Ref<render::GeometryPool> pool;
  /*! The shared program state to be used by the renderer.
   */
  Ref<SharedProgramState> state;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Forward renderer.
 *  @ingroup renderer
 */
class Renderer : public render::System
{
public:
  /*! Renders the specified scene to the current framebuffer using the
   *  specified camera.
   */
  void render(const render::Scene& scene, const render::Camera& camera);
  /*! @return The shared program state object used by this renderer.
   */
  SharedProgramState& getSharedProgramState();
  /*! Creates a renderer object using the specified geometry pool and the
   *  specified configuration.
   *  @return The newly constructed renderer object, or @c NULL if an error
   *  occurred.
   */
  static Ref<Renderer> create(const Config& config);
private:
  Renderer(render::GeometryPool& pool);
  bool init(const Config& config);
  void renderOperations(const render::Queue& queue);
  void releaseObjects();
  Ref<SharedProgramState> state;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace forward*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_FORWARD_H*/
///////////////////////////////////////////////////////////////////////
