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

#include <wendy/Config.h>

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

namespace
{

struct OpaqueOperationComparator
{
  inline bool operator () (const Operation* x, const Operation* y)
  {
    if (x->hash != y->hash)
      return x->hash < y->hash;

    return x->distance < y->distance;
  }
};

struct BlendedOperationComparator
{
  inline bool operator () (const Operation* x, const Operation* y)
  {
    return x->distance > y->distance;
  }
};

/* Hash function used in the ELF executable format
 */
unsigned int hashString(const String& string)
{
  unsigned int hash = 0;
  unsigned int temp;

  for (String::const_iterator c = string.begin();  c != string.end();  c++)
  {
    hash = (hash << 4) + *c;

    if (temp = hash & 0xf0000000)
      hash ^= temp >> 24;

    hash &= ~temp;
  }

  return hash;
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Operation::Operation(void):
  technique(NULL),
  distance(0.f)
{
}

///////////////////////////////////////////////////////////////////////

Queue::Queue(const Camera& initCamera, Light* initLight):
  camera(initCamera),
  light(initLight),
  phase(COLLECT_GEOMETRY),
  sortedOpaque(false),
  sortedBlended(false)
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

void Queue::addOperation(const Operation& operation)
{
  if (operation.technique->isBlending())
  {
    blendedOperations.push_back(operation);
    blendedOperations.back().hash = hashString(operation.technique->getName());
    sortedBlended = false;
  }
  else
  {
    opaqueOperations.push_back(operation);
    opaqueOperations.back().hash = hashString(operation.technique->getName());
    sortedOpaque = false;
  }
}

void Queue::removeOperations(void)
{
  opaqueOperations.clear();
  sortedOpaque = false;

  blendedOperations.clear();
  sortedBlended = false;
}

void Queue::render(const String& passName) const
{
  camera.apply();

  renderOperations(getOpaqueOperations(), passName);
  renderOperations(getBlendedOperations(), passName);
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

const OperationList& Queue::getOpaqueOperations(void) const
{
  if (!sortedOpaque)
  {
    const List& operations = opaqueOperations;

    sortedOpaqueOps.clear();
    sortedOpaqueOps.reserve(opaqueOperations.size());
    for (List::const_iterator o = operations.begin();  o != operations.end();  o++)
      sortedOpaqueOps.push_back(&(*o));

    OpaqueOperationComparator comparator;
    std::sort(sortedOpaqueOps.begin(),
              sortedOpaqueOps.end(),
	      comparator);

    sortedOpaque = true;
  }

  return sortedOpaqueOps;
}

const OperationList& Queue::getBlendedOperations(void) const
{
  if (!sortedBlended)
  {
    const List& operations = blendedOperations;

    sortedBlendedOps.clear();
    sortedBlendedOps.reserve(blendedOperations.size());
    for (List::const_iterator o = operations.begin();  o != operations.end();  o++)
      sortedBlendedOps.push_back(&(*o));

    BlendedOperationComparator comparator;
    std::sort(sortedBlendedOps.begin(),
              sortedBlendedOps.end(),
	      comparator);

    sortedBlended = true;
  }

  return sortedBlendedOps;
}

const LightState& Queue::getLights(void) const
{
  return lights;
}

void Queue::renderOperations(const OperationList& operations, const String& passName) const
{
  GL::Context* context = GL::Context::get();
  if (!context)
    throw Exception("Cannot render render queue without an OpenGL context");

  for (OperationList::const_iterator o = operations.begin();  o != operations.end();  o++)
  {
    const Operation& operation = **o;

    for (unsigned int i = 0;  i < operation.technique->getPassCount();  i++)
    {
      const Pass& pass = operation.technique->getPass(i);
      if (pass.getName() != passName)
	continue;

      pass.apply();

      context->setModelMatrix(operation.transform);
      context->render(operation.range);
    }
  }
}

///////////////////////////////////////////////////////////////////////

Renderable::~Renderable(void)
{
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
