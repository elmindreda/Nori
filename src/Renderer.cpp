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

#include <wendy/Config.hpp>

#include <wendy/Core.hpp>
#include <wendy/Time.hpp>
#include <wendy/Profile.hpp>
#include <wendy/Transform.hpp>
#include <wendy/Primitive.hpp>
#include <wendy/Frustum.hpp>
#include <wendy/Camera.hpp>

#include <wendy/Pass.hpp>
#include <wendy/Material.hpp>
#include <wendy/RenderQueue.hpp>
#include <wendy/Renderer.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

void Renderer::render(const RenderQueue& queue, const Camera& camera)
{
  ProfileNodeCall call("Renderer::render");

  m_context.setCurrentSharedProgramState(m_state);

  const Recti& viewportArea = m_context.viewportArea();
  m_state->setViewportSize(float(viewportArea.size.x),
                           float(viewportArea.size.y));

  m_state->setProjectionMatrix(camera.projectionMatrix());
  m_state->setViewMatrix(camera.viewTransform());

  if (camera.isPerspective())
  {
    m_state->setCameraProperties(camera.transform().position,
                                 camera.FOV(),
                                 camera.aspectRatio(),
                                 camera.nearZ(),
                                 camera.farZ());
  }

  renderOperations(queue.opaqueBucket());
  renderOperations(queue.blendedBucket());

  m_context.setCurrentSharedProgramState(nullptr);
}

void Renderer::setSharedProgramState(SharedProgramState* newState)
{
  if (newState)
    m_state = newState;
  else
    m_state = new SharedProgramState();
}

Ref<Renderer> Renderer::create(RenderContext& context)
{
  Ref<Renderer> renderer(new Renderer(context));
  if (!renderer->init())
    return nullptr;

  return renderer;
}

Renderer::Renderer(RenderContext& context):
  m_context(context)
{
}

bool Renderer::init()
{
  setSharedProgramState(nullptr);
  return true;
}

void Renderer::renderOperations(const RenderBucket& bucket)
{
  const auto& operations = bucket.operations();

  for (auto k : bucket.keys())
  {
    const RenderOpKey key(k);
    const RenderOp& op = operations[key.index];

    m_state->setModelMatrix(op.transform);
    op.state->apply();

    m_context.render(op.range);
  }
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
