///////////////////////////////////////////////////////////////////////
// Wendy default renderer
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderQueue.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{

struct OperationComparator
{
  inline bool operator () (const Operation* x, const Operation* y)
  {
    return *x < *y;
  }
};

}

///////////////////////////////////////////////////////////////////////

Operation::Operation(void):
  technique(NULL),
  distance(0.f)
{
}

bool Operation::operator < (const Operation& other) const
{
  // Sort blending operations by distance
  if (blending && other.blending)
    return distance > other.distance;

  // ...and then by pass
  return (*technique) < (*other.technique);
}

///////////////////////////////////////////////////////////////////////

Queue::Queue(const Camera& initCamera, Light* initLight):
  camera(initCamera),
  light(initLight),
  phase(COLLECT_GEOMETRY),
  sorted(false)
{
}

void Queue::attachLight(Light& light)
{
  lights.attachLight(light);
}

void Queue::detachLights(void)
{
  lights.detachLights();
}

Operation& Queue::createOperation(void)
{
  sorted = false;

  operations.push_front(Operation());
  return operations.front();
}

void Queue::destroyOperations(void)
{
  operations.clear();
  sorted = false;
}

void Queue::render(const String& passName) const
{
  GL::Renderer* renderer = GL::Renderer::get();
  if (!renderer)
    throw Exception("Cannot render render queue without a renderer");

  camera.apply();
  lights.apply();

  const OperationList& operations = getOperations();

  for (OperationList::const_iterator o = operations.begin();  o != operations.end();  o++)
  {
    const Operation& operation = **o;

    for (unsigned int i = 0;  i < operation.technique->getPassCount();  i++)
    {
      const Pass& pass = operation.technique->getPass(i);
      if (pass.getName() != passName)
	continue;

      pass.apply();

      renderer->setModelMatrix(operation.transform);
      renderer->render(operation.range);
    }
  }
}

Queue::Phase Queue::getPhase(void) const
{
  return phase;
}

void Queue::setPhase(Phase newPhase)
{
  phase = newPhase;
}

const Camera& Queue::getCamera(void) const
{
  return camera;
}

Light* Queue::getActiveLight(void) const
{
  return light;
}

const OperationList& Queue::getOperations(void) const
{
  if (!sorted)
  {
    sortedOperations.clear();
    sortedOperations.reserve(operations.size());
    for (List::const_iterator i = operations.begin();  i != operations.end();  i++)
    {
      (*i).blending = (*i).technique->isBlending();
      sortedOperations.push_back(&(*i));
    }

    OperationComparator comparator;
    std::sort(sortedOperations.begin(),
              sortedOperations.end(),
	      comparator);

    sorted = true;
  }

  return sortedOperations;
}

const LightState& Queue::getLights(void) const
{
  return lights;
}

///////////////////////////////////////////////////////////////////////

Renderable::~Renderable(void)
{
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
