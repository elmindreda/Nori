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
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLLight.h>
#include <wendy/GLShader.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderStyle.h>
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
  vertexBuffer(NULL),
  indexBuffer(NULL),
  technique(NULL),
  start(0),
  count(0),
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

Queue::Queue(const Camera& initCamera,
             GL::Light* initLight,
	     const String& initName):
  camera(initCamera),
  light(initLight),
  name(initName),
  sorted(false)
{
}

void Queue::attachLight(GL::Light& light)
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

void Queue::render(void) const
{
  GL::Renderer* renderer = GL::Renderer::get();
  if (!renderer)
    throw Exception("Cannot render render queue without a renderer");

  camera.begin();
  lights.apply();

  const OperationList& operations = getOperations();
  
  for (unsigned int i = 0;  i < operations.size();  i++)
  {
    const Operation& operation = *operations[i];

    renderer->pushTransform(operation.transform);

    for (unsigned int j = 0;  j < operation.technique->getPassCount();  j++)
    {
      const GL::Pass& pass = operation.technique->getPass(j);
      if (!pass.getName().empty())
	continue;

      pass.apply();

      if (operation.indexBuffer)
        operation.indexBuffer->render(*(operation.vertexBuffer),
	                              operation.renderMode,
	                              operation.start,
				      operation.count);
      else
        operation.vertexBuffer->render(operation.renderMode,
	                               operation.start,
				       operation.count);
    }

    renderer->popTransform();
  }

  camera.end();
}

const String& Queue::getName(void) const
{
  return name;
}

const Camera& Queue::getCamera(void) const
{
  return camera;
}

GL::Light* Queue::getActiveLight(void) const
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

const GL::LightState& Queue::getLights(void) const
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
