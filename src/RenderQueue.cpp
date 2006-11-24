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
#include <wendy/GLShader.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLLight.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLPass.h>

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

Group::Group(GL::Light* initLight):
  light(initLight)
{
}

Operation& Group::createOperation(void)
{
  sorted = false;

  operations.push_front(Operation());
  return operations.front();
}

void Group::destroyOperations(void)
{
  operations.clear();
}

GL::Light* Group::getLight(void) const
{
  return light;
}

const OperationList& Group::getOperations(void) const
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

///////////////////////////////////////////////////////////////////////

Queue::Queue(const Camera& initCamera):
  camera(initCamera)
{
}

void Queue::attachLight(GL::Light& light)
{
  if (findGroup(light))
    return;

  lightGroups.push_front(Group(&light));
}

void Queue::detachLights(void)
{
  lightGroups.clear();
}

Operation& Queue::createOperation(void)
{
  return defaultGroup.createOperation();
}

Operation& Queue::createLightOperation(GL::Light& light)
{
  Group* group = findGroup(light);
  if (!group)
  {
    // NOTE: An exception is thrown here because attaching lights and
    //       registering rendering operations on those lights will usually be
    //       done by entirely different parts of the code, and if they don't
    //       agree on the set of lights in use for a particular queue, then
    //       that's most likely a bug.
    throw Exception("Cannot create render operation on non-attached lights");
  }

  return group->createOperation();
}

void Queue::destroyOperations(void)
{
  defaultGroup.destroyOperations();
  for (GroupList::iterator i = lightGroups.begin();  i != lightGroups.end();  i++)
    (*i).destroyOperations();
}

const Camera& Queue::getCamera(void) const
{
  return camera;
}

unsigned int Queue::getLightCount(void) const
{
  return lightGroups.size();
}

Group& Queue::getDefaultGroup(void)
{
  return defaultGroup;
}

const Group& Queue::getDefaultGroup(void) const
{
  return defaultGroup;
}

Group& Queue::getLightGroup(unsigned int index)
{
  GroupList::iterator group = lightGroups.begin();
  std::advance(group, index);
  return *group;
}

const Group& Queue::getLightGroup(unsigned int index) const
{
  GroupList::const_iterator group = lightGroups.begin();
  std::advance(group, index);
  return *group;
}

/*
void Queue::renderGroup(const Group& group) const
{
  const OperationList& operations = group.getOperations();
  
  for (unsigned int i = 0;  i < operations.size();  i++)
  {
    const Operation& operation = *operations[i];

    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(operation.transform);
    glPopAttrib();

    for (unsigned int j = 0;  j < operation.style->getPassCount();  j++)
    {
      const RenderPass& pass = operation.style->getPass(j);
      if (!pass.getGroupName().empty())
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

    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
  }
}
*/

Group* Queue::findGroup(GL::Light& light)
{
  for (GroupList::iterator i = lightGroups.begin();  i != lightGroups.end();  i++)
  {
    if ((*i).getLight() == &light)
      return &(*i);
  }

  return NULL;
}

const Group* Queue::findGroup(GL::Light& light) const
{
  for (GroupList::const_iterator i = lightGroups.begin();  i != lightGroups.end();  i++)
  {
    if ((*i).getLight() == &light)
      return &(*i);
  }

  return NULL;
}

///////////////////////////////////////////////////////////////////////

Renderable::~Renderable(void)
{
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
