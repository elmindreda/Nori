///////////////////////////////////////////////////////////////////////
// Wendy OpenAL library
// Copyright (c) 2007 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/OpenAL.h>
#include <wendy/ALContext.h>

#include <glm/gtc/type_ptr.hpp>

#include <alut.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace AL
  {

///////////////////////////////////////////////////////////////////////

Context::~Context(void)
{
  alutExit();
}

const vec3& Context::getListenerPosition(void) const
{
  return listenerPosition;
}

void Context::setListenerPosition(const vec3& newPosition)
{
  if (listenerPosition != newPosition)
  {
    alListenerfv(AL_POSITION, value_ptr(listenerPosition));
    listenerPosition = newPosition;

#if WENDY_DEBUG
    checkAL("Failed to set listener position");
#endif
  }
}

const vec3& Context::getListenerVelocity(void) const
{
  return listenerVelocity;
}

void Context::setListenerVelocity(const vec3& newVelocity)
{
  if (listenerVelocity != newVelocity)
  {
    alListenerfv(AL_VELOCITY, value_ptr(listenerVelocity));
    listenerVelocity = newVelocity;

#if WENDY_DEBUG
    checkAL("Failed to set listener velocity");
#endif
  }
}

ResourceIndex& Context::getIndex(void) const
{
  return index;
}

bool Context::createSingleton(ResourceIndex& index)
{
  Ptr<Context> context(new Context(index));
  if (!context->init())
    return false;

  set(context.detachObject());
  return true;
}

Context::Context(ResourceIndex& initIndex):
  index(initIndex)
{
}

Context::Context(const Context& source):
  index(source.index)
{
  // NOTE: Not implemented.
}

bool Context::init(void)
{
  if (!alutInit(NULL, NULL))
  {
    logError("Failed to initialize OpenAL: %s",
             alutGetErrorString(alutGetError()));
    return false;
  }

  alListenerfv(AL_POSITION, value_ptr(listenerPosition));
  alListenerfv(AL_VELOCITY, value_ptr(listenerVelocity));

  return true;
}

Context& Context::operator = (const Context& source)
{
  // NOTE: Not implemented.

  return *this;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
