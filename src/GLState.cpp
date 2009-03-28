///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/GLState.h>

#define GLEW_STATIC
#include <GL/glew.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

void StencilState::apply(void) const
{
  // NOTE: Yes, I know this is huge.  You don't need to point it out.

  if (cache.dirty)
  {
    force();
    return;
  }
  
  if (data.enabled)
  {
    if (!cache.enabled)
    {
      glEnable(GL_STENCIL_TEST);
      cache.enabled = data.enabled;
    }

    if (data.function != cache.function ||
        data.reference != cache.reference ||
        data.writeMask != cache.writeMask)
    {
      glStencilFunc(data.function, data.reference, data.writeMask);
      cache.function = data.function;
      cache.reference = data.reference;
      cache.writeMask = data.writeMask;
    }

    if (data.stencilFailed != cache.stencilFailed ||
        data.depthFailed != cache.depthFailed ||
        data.depthPassed != cache.depthPassed)
    {
      glStencilOp(data.stencilFailed, data.depthFailed, data.depthPassed);
      cache.stencilFailed = data.stencilFailed;
      cache.depthFailed = data.depthFailed;
      cache.depthPassed = data.depthPassed;
    }
  }
  else
  {
    if (cache.enabled)
    {
      glDisable(GL_STENCIL_TEST);
      cache.enabled = data.enabled;
    }
  }

#if _DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeError("Error when applying stencil state: %s", gluErrorString(error));
#endif

  data.dirty = false;
}

bool StencilState::isEnabled(void) const
{
  return data.enabled;
}

GLenum StencilState::getFunction(void) const
{
  return data.function;
}

GLenum StencilState::getStencilFailOperation(void) const
{
  return data.stencilFailed;
}

GLenum StencilState::getDepthFailOperation(void) const
{
  return data.depthFailed;
}

GLenum StencilState::getDepthPassOperation(void) const
{
  return data.depthPassed;
}

unsigned int StencilState::getReference(void) const
{
  return data.reference;
}

unsigned int StencilState::getWriteMask(void) const
{
  return data.writeMask;
}

void StencilState::setEnabled(bool newState)
{
  data.enabled = newState;
  data.dirty = true;
}

void StencilState::setFunction(GLenum newFunction)
{
  data.function = newFunction;
  data.dirty = true;
}

void StencilState::setReference(unsigned int newReference)
{
  data.reference = newReference;
  data.dirty = true;
}

void StencilState::setWriteMask(unsigned int newMask)
{
  data.writeMask = newMask;
  data.dirty = true;
}

void StencilState::setOperations(GLenum stencilFailed,
                                 GLenum depthFailed,
                                 GLenum depthPassed)
{
  data.stencilFailed = stencilFailed;
  data.depthFailed = depthFailed;
  data.depthPassed = depthPassed;
  data.dirty = true;
}

void StencilState::setDefaults(void)
{
  data.setDefaults();
}

void StencilState::invalidateCache(void)
{
  cache.dirty = true;
}

void StencilState::force(void) const
{
  cache = data;

  if (data.enabled)
    glEnable(GL_STENCIL_TEST);
  else
    glDisable(GL_STENCIL_TEST);

  glStencilFunc(data.function, data.reference, data.writeMask);
  glStencilOp(data.stencilFailed, data.depthFailed, data.depthPassed);

#if _DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeWarning("Error when forcing stencil state: %s", gluErrorString(error));
#endif

  cache.dirty = data.dirty = false;
}

StencilState::Data StencilState::cache;

///////////////////////////////////////////////////////////////////////

StencilState::Data::Data(void)
{
  setDefaults();
}

void StencilState::Data::setDefaults(void)
{
  dirty = true;
  enabled = false;
  function = GL_ALWAYS;
  reference = 0;
  writeMask = ~0;
  stencilFailed = GL_KEEP;
  depthFailed = GL_KEEP;
  depthPassed = GL_KEEP;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
