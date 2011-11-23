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
#include <wendy/Core.h>

#include <wendy/OpenAL.h>
#include <wendy/ALContext.h>

#include <glm/gtc/type_ptr.hpp>

#include <al.h>
#include <alc.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace AL
  {

///////////////////////////////////////////////////////////////////////

Context::~Context()
{
  if (context)
  {
    alcMakeContextCurrent(NULL);
    alcDestroyContext((ALCcontext*) context);
  }

  if (device)
    alcCloseDevice((ALCdevice*) device);
}

const vec3& Context::getListenerPosition() const
{
  return listenerPosition;
}

void Context::setListenerPosition(const vec3& newPosition)
{
  if (listenerPosition != newPosition)
  {
    listenerPosition = newPosition;
    alListenerfv(AL_POSITION, value_ptr(listenerPosition));

#if WENDY_DEBUG
    checkAL("Failed to set listener position");
#endif
  }
}

const vec3& Context::getListenerVelocity() const
{
  return listenerVelocity;
}

void Context::setListenerVelocity(const vec3& newVelocity)
{
  if (listenerVelocity != newVelocity)
  {
    listenerVelocity = newVelocity;
    alListenerfv(AL_VELOCITY, value_ptr(listenerVelocity));

#if WENDY_DEBUG
    checkAL("Failed to set listener velocity");
#endif
  }
}

const quat& Context::getListenerRotation() const
{
  return listenerRotation;
}

void Context::setListenerRotation(const quat& newRotation)
{
  if (listenerRotation != newRotation)
  {
    listenerRotation = newRotation;

    const vec3 at = newRotation * vec3(0.f, 0.f, -1.f);
    const vec3 up = newRotation * vec3(0.f, 1.f, 0.f);

    const float orientation[] = { at.x, at.y, at.z, up.x, up.y, up.z };

    alListenerfv(AL_ORIENTATION, orientation);

#if WENDY_DEBUG
    checkAL("Failed to set listener rotation");
#endif
  }
}

float Context::getListenerGain() const
{
  return listenerGain;
}

void Context::setListenerGain(float newGain)
{
  if (listenerGain != newGain)
  {
    listenerGain = newGain;
    alListenerfv(AL_GAIN, &listenerGain);

#if WENDY_DEBUG
    checkAL("Failed to set listener gain");
#endif
  }
}

ResourceCache& Context::getCache() const
{
  return cache;
}

bool Context::createSingleton(ResourceCache& cache)
{
  Ptr<Context> context(new Context(cache));
  if (!context->init())
    return false;

  set(context.detachObject());
  return true;
}

Context::Context(ResourceCache& initCache):
  cache(initCache),
  device(NULL),
  context(NULL),
  listenerGain(1.f)
{
}

Context::Context(const Context& source):
  cache(source.cache)
{
  panic("OpenAL contexts may not be copied");
}

bool Context::init()
{
  device = alcOpenDevice(NULL);
  if (!device)
  {
    logError("Failed to open OpenAL device");
    return false;
  }

  context = alcCreateContext((ALCdevice*) device, NULL);
  if (!context)
  {
    logError("Failed to create OpenAL context");
    return false;
  }

  if (!alcMakeContextCurrent((ALCcontext*) context))
  {
    logError("Failed to make OpenAL context current");
    return false;
  }

  log("OpenAL context version %s created",
      (const char*) alGetString(AL_VERSION));

  log("OpenAL context renderer is %s by %s",
      (const char*) alGetString(AL_RENDERER),
      (const char*) alGetString(AL_VENDOR));

  return true;
}

Context& Context::operator = (const Context& source)
{
  panic("OpenAL contexts may not be assigned");
}

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
