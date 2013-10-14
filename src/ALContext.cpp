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

#include <wendy/Config.hpp>
#include <wendy/Core.hpp>

#include <wendy/ALContext.hpp>

#include <internal/ALHelper.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <al.h>
#include <alc.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

AudioContext::~AudioContext()
{
  if (m_handle)
  {
    alcMakeContextCurrent(nullptr);
    alcDestroyContext((ALCcontext*) m_handle);
  }

  if (m_device)
    alcCloseDevice((ALCdevice*) m_device);
}

void AudioContext::setListenerPosition(const vec3& newPosition)
{
  if (m_listenerPosition != newPosition)
  {
    m_listenerPosition = newPosition;
    alListenerfv(AL_POSITION, value_ptr(m_listenerPosition));

#if WENDY_DEBUG
    checkAL("Failed to set listener position");
#endif
  }
}

void AudioContext::setListenerVelocity(const vec3& newVelocity)
{
  if (m_listenerVelocity != newVelocity)
  {
    m_listenerVelocity = newVelocity;
    alListenerfv(AL_VELOCITY, value_ptr(m_listenerVelocity));

#if WENDY_DEBUG
    checkAL("Failed to set listener velocity");
#endif
  }
}

void AudioContext::setListenerRotation(const quat& newRotation)
{
  if (m_listenerRotation != newRotation)
  {
    m_listenerRotation = newRotation;

    const vec3 at = newRotation * vec3(0.f, 0.f, -1.f);
    const vec3 up = newRotation * vec3(0.f, 1.f, 0.f);

    const float orientation[] = { at.x, at.y, at.z, up.x, up.y, up.z };

    alListenerfv(AL_ORIENTATION, orientation);

#if WENDY_DEBUG
    checkAL("Failed to set listener rotation");
#endif
  }
}

void AudioContext::setListenerGain(float newGain)
{
  if (m_listenerGain != newGain)
  {
    m_listenerGain = newGain;
    alListenerfv(AL_GAIN, &m_listenerGain);

#if WENDY_DEBUG
    checkAL("Failed to set listener gain");
#endif
  }
}

AudioContext* AudioContext::create(ResourceCache& cache)
{
  Ptr<AudioContext> context(new AudioContext(cache));
  if (!context->init())
    return nullptr;

  return context.detachObject();
}

AudioContext::AudioContext(ResourceCache& cache):
  m_cache(cache),
  m_device(nullptr),
  m_handle(nullptr),
  m_listenerGain(1.f)
{
}

bool AudioContext::init()
{
  m_device = alcOpenDevice(nullptr);
  if (!m_device)
  {
    checkALC("Failed to open OpenAL device");
    return false;
  }

  m_handle = alcCreateContext((ALCdevice*) m_device, nullptr);
  if (!m_handle)
  {
    checkALC("Failed to create OpenAL context");
    return false;
  }

  if (!alcMakeContextCurrent((ALCcontext*) m_handle))
  {
    checkALC("Failed to make OpenAL context current");
    return false;
  }

  log("OpenAL context version %s created",
      (const char*) alGetString(AL_VERSION));

  log("OpenAL context renderer is %s by %s",
      (const char*) alGetString(AL_RENDERER),
      (const char*) alGetString(AL_VENDOR));

  log("OpenAL context uses device %s",
      (const char*) alcGetString((ALCdevice*) m_device, ALC_DEVICE_SPECIFIER));

  return true;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
