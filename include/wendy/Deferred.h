///////////////////////////////////////////////////////////////////////
// Wendy deferred renderer
// Copyright (c) 2010 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_DEFERRED_H
#define WENDY_DEFERRED_H
///////////////////////////////////////////////////////////////////////

#include <wendy/RenderState.h>
#include <wendy/RenderScene.h>
#include <wendy/RenderPool.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace deferred
  {

///////////////////////////////////////////////////////////////////////

/*! @brief Shared program state for the deferred renderer.
 */
class SharedProgramState : public render::SharedProgramState
{
};

///////////////////////////////////////////////////////////////////////

/*! @brief Deferred renderer configuration.
 */
class Config
{
public:
  /*! Constructor.
   *  @param[in] width The desired width of the G-buffer.
   *  @param[in] height The desired height of the G-buffer.
   *  @param[in] pool The geometry pool to use.
   */
  Config(unsigned int width, unsigned int height, render::GeometryPool& pool);
  /*! The desired width of the G-buffer.
   */
  unsigned int width;
  /*! The desired height of the G-buffer.
   */
  unsigned int height;
  /*! The geometry pool to be used by the renderer.
   */
  Ref<render::GeometryPool> pool;
  /*! The shared program state to be used by the renderer.
   */
  Ref<SharedProgramState> state;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Deferred renderer.
 */
class Renderer : public render::System
{
public:
  /*! Renders the specified scene to the G-buffer of this renderer using the
   *  specified camera, then renders the lit scene to the current framebuffer
   *  using the lights in the specified scene.
   */
  void render(const render::Scene& scene, const render::Camera& camera);
  /*! @return The shared program state object used by this renderer.
   */
  SharedProgramState& getSharedProgramState();
  /*! @return The color buffer texture of the G-buffer of this renderer.
   */
  GL::Texture& getColorTexture() const;
  /*! @return The normal/specular buffer texture of the G-buffer of this renderer.
   */
  GL::Texture& getNormalTexture() const;
  /*! @return The depth buffer texture of the G-buffer of this renderer.
   */
  GL::Texture& getDepthTexture() const;
  /*! Creates a renderer object using the specified geometry pool and the
   *  specified configuration.
   *  @return The newly constructed renderer object, or @c NULL if an error
   *  occurred.
   */
  static Ref<Renderer> create(const Config& config);
private:
  Renderer(render::GeometryPool& pool);
  bool init(const Config& config);
  void renderLightQuad(const render::Camera& camera);
  void renderAmbientLight(const render::Camera& camera, const vec3& color);
  void renderLight(const render::Camera& camera, const render::Light& light);
  void renderOperations(const render::Queue& queue);
  Ref<GL::ImageFramebuffer> framebuffer;
  Ref<GL::Texture> depthTexture;
  Ref<GL::Texture> colorTexture;
  Ref<GL::Texture> normalTexture;
  GL::RenderState dirLightPass;
  GL::RenderState pointLightPass;
  GL::RenderState ambientLightPass;
  Ref<SharedProgramState> state;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace deferred*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEFERRED_H*/
///////////////////////////////////////////////////////////////////////
