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

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Timer.h>
#include <wendy/Profile.h>

#include <wendy/RenderPool.h>
#include <wendy/RenderState.h>
#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderScene.h>
#include <wendy/RenderSprite.h>

#include <wendy/Forward.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace forward
  {

///////////////////////////////////////////////////////////////////////

Config::Config(render::GeometryPool& initPool):
  pool(&initPool)
{
}

///////////////////////////////////////////////////////////////////////

void Renderer::render(const render::Scene& scene, const render::Camera& camera)
{
  ProfileNodeCall call("forward::Renderer::render");

  GL::Context& context = getContext();
  context.setCurrentSharedProgramState(state);

  const Recti& viewportArea = context.getViewportArea();
  state->setViewportSize(float(viewportArea.size.x),
                         float(viewportArea.size.y));

  if (camera.isOrtho())
    state->setOrthoProjectionMatrix(camera.getOrthoVolume());
  else
  {
    state->setPerspectiveProjectionMatrix(camera.getFOV(),
                                          camera.getAspectRatio(),
                                          camera.getNearZ(),
                                          camera.getFarZ());
    state->setCameraProperties(camera.getTransform().position,
                               camera.getFOV(),
                               camera.getAspectRatio(),
                               camera.getNearZ(),
                               camera.getFarZ());
  }

  state->setViewMatrix(camera.getViewTransform());

  renderOperations(scene.getOpaqueQueue());
  renderOperations(scene.getBlendedQueue());

  context.setCurrentSharedProgramState(NULL);

  releaseObjects();
}

SharedProgramState& Renderer::getSharedProgramState()
{
  return *state;
}

Ref<Renderer> Renderer::create(const Config& config)
{
  if (!config.pool)
  {
    logError("Cannot create forward renderer without a geometry pool");
    return NULL;
  }

  Ptr<Renderer> renderer(new Renderer(*config.pool));
  if (!renderer->init(config))
    return NULL;

  return renderer.detachObject();
}

Renderer::Renderer(render::GeometryPool& pool):
  render::System(pool, render::System::FORWARD)
{
}

bool Renderer::init(const Config& config)
{
  GL::Context& context = getContext();

  if (config.state)
    state = config.state;
  else
    state = new SharedProgramState();

  state->reserveSupported(context);

  return true;
}

void Renderer::renderOperations(const render::Queue& queue)
{
  GL::Context& context = getContext();
  const render::SortKeyList& keys = queue.getSortKeys();
  const render::OperationList& operations = queue.getOperations();

  for (auto k = keys.begin();  k != keys.end();  k++)
  {
    const render::SortKey key(*k);
    const render::Operation& op = operations[key.index];

    state->setModelMatrix(op.transform);
    op.state->apply();

    context.render(op.range);
  }
}

void Renderer::releaseObjects()
{
  GL::Context& context = getContext();

  context.setCurrentProgram(NULL);
  context.setCurrentVertexBuffer(NULL);
  context.setCurrentIndexBuffer(NULL);

  for (size_t i = 0;  i < context.getTextureUnitCount();  i++)
  {
    context.setActiveTextureUnit(i);
    context.setCurrentTexture(NULL);
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace forward*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
