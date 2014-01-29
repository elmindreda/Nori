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
#ifndef WENDY_FORWARD_HPP
#define WENDY_FORWARD_HPP
///////////////////////////////////////////////////////////////////////

#include <wendy/RenderSystem.hpp>
#include <wendy/RenderState.hpp>
#include <wendy/RenderScene.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace forward
  {

///////////////////////////////////////////////////////////////////////

/*! @brief Forward renderer configuration.
 *  @ingroup renderer
 */
class Config
{
public:
  /*! Constructor.
   *  @param[in] context The render context to use.
   */
  Config(RenderContext& context);
  /*! The render context to be used by the renderer.
   */
  RenderContext& context;
  /*! The shared program state to be used by the renderer.
   */
  Ref<SharedProgramState> state;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Forward renderer.
 *  @ingroup renderer
 */
class Renderer : public RenderSystem
{
public:
  /*! Renders the specified scene to the current framebuffer using the
   *  specified camera.
   */
  void render(const Scene& scene, const Camera& camera);
  /*! @return The shared program state object used by this renderer.
   */
  SharedProgramState& sharedProgramState() { return *m_state; }
  /*! Creates a renderer object using the specified render context and the
   *  specified configuration.
   *  @return The newly constructed renderer object, or @c nullptr if an error
   *  occurred.
   */
  static Ref<Renderer> create(const Config& config);
private:
  Renderer(RenderContext& context);
  bool init(const Config& config);
  void renderOperations(const RenderQueue& queue);
  Ref<SharedProgramState> m_state;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace forward*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_FORWARD_HPP*/
///////////////////////////////////////////////////////////////////////
