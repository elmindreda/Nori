///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@home.se>
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

#include <moira/Config.h>
#include <moira/Core.h>
#include <moira/Color.h>
#include <moira/Vector.h>
#include <moira/Matrix.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLShader.h>
#include <wendy/GLLight.h>
#include <wendy/GLVertex.h>
#include <wendy/GLIndexBuffer.h>
#include <wendy/GLVertexBuffer.h>
#include <wendy/GLRender.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

RenderOperation::RenderOperation(void):
  vertexBuffer(NULL),
  indexBuffer(NULL),
  shader(NULL)
{
}

bool RenderOperation::operator < (const RenderOperation& other) const
{
  return (*shader) < (*other.shader);
}

///////////////////////////////////////////////////////////////////////

RenderQueue::RenderQueue(void):
  sorted(true)
{
}

void RenderQueue::addLight(Light& light)
{
  lights.push_back(&light);
}

void RenderQueue::addOperation(RenderOperation& operation)
{
  operations.push_back(operation);
  sorted = false;
}

void RenderQueue::removeOperations(void)
{
  operations.clear();
}

void RenderQueue::renderOperations(void)
{
  sortOperations();

  for (LightList::const_iterator i = lights.begin();  i != lights.end();  i++)
    (*i)->setEnabled(true);

  for (OperationList::const_iterator i = operations.begin();  i != operations.end();  i++)
  {
    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf((*i).transform);
    glPopAttrib();

    (*i).vertexBuffer->apply();
    if ((*i).indexBuffer)
      (*i).indexBuffer->apply();

    for (unsigned int pass = 0;  pass < (*i).shader->getPassCount();  pass++)
    {
      (*i).shader->applyPass(pass);

      if ((*i).indexBuffer)
        (*i).indexBuffer->render((*i).renderMode);
      else
        (*i).vertexBuffer->render((*i).renderMode);
    }

    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
  }

  for (LightList::const_iterator i = lights.begin();  i != lights.end();  i++)
    (*i)->setEnabled(false);
}

const RenderQueue::LightList& RenderQueue::getLights(void) const
{
  return lights;
}

const RenderQueue::OperationList& RenderQueue::getOperations(void) const
{
  return operations;
}

void RenderQueue::sortOperations(void)
{
  if (!sorted)
  {
    std::sort(operations.begin(), operations.end());
    sorted = true;
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
