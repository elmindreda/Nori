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

#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderPool.h>
#include <wendy/RenderScene.h>
#include <wendy/RenderSprite.h>

#include <wendy/Forward.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace forward
  {

///////////////////////////////////////////////////////////////////////

void Renderer::render(const render::Scene& scene, const render::Camera& camera)
{
  GL::Context& context = pool.getContext();
  context.setCurrentSharedProgramState(state);

  state->setViewMatrix(camera.getViewTransform());
  state->setPerspectiveProjectionMatrix(camera.getFOV(),
                                        camera.getAspectRatio(),
                                        camera.getMinDepth(),
                                        camera.getMaxDepth());
  state->setCameraProperties(camera.getTransform().position,
                             camera.getFOV(),
                             camera.getAspectRatio(),
                             camera.getMinDepth(),
                             camera.getMaxDepth());

  renderOperations(scene.getOpaqueQueue());
  renderOperations(scene.getBlendedQueue());

  context.setCurrentSharedProgramState(NULL);
}

SharedProgramState& Renderer::getSharedProgramState()
{
  return *state;
}

render::GeometryPool& Renderer::getGeometryPool()
{
  return pool;
}

Renderer* Renderer::create(render::GeometryPool& pool, const Config& config)
{
  Ptr<Renderer> renderer(new Renderer(pool));
  if (!renderer->init(config))
    return NULL;

  return renderer.detachObject();
}

Renderer::Renderer(render::GeometryPool& initPool):
  pool(initPool)
{
}

bool Renderer::init(const Config& config)
{
  GL::Context& context = pool.getContext();

  if (config.state)
    state = config.state;
  else
    state = new SharedProgramState();

  state->reserveSupported(context);

  return true;
}

void Renderer::renderOperations(const render::Queue& queue)
{
  GL::Context& context = pool.getContext();
  const render::SortKeyList& keys = queue.getSortKeys();
  const render::OperationList& operations = queue.getOperations();

  for (render::SortKeyList::const_iterator k = keys.begin();  k != keys.end();  k++)
  {
    const render::Operation& op = operations[k->index];

    state->setModelMatrix(op.transform);
    op.state->apply();

    context.render(op.range);
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace forward*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
