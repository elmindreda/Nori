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
#include <wendy/ALBuffer.h>
#include <wendy/ALSource.h>

#include <glm/gtc/type_ptr.hpp>

#include <al.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace AL
  {

///////////////////////////////////////////////////////////////////////

Source::~Source()
{
  if (sourceID)
    alDeleteSources(1, &sourceID);
}

void Source::start()
{
  alSourcePlay(sourceID);

#if WENDY_DEBUG
  checkAL("Failed to start source");
#endif
}

void Source::stop()
{
  alSourceStop(sourceID);

#if WENDY_DEBUG
  checkAL("Failed to stop source");
#endif
}

void Source::pause()
{
  alSourcePause(sourceID);

#if WENDY_DEBUG
  checkAL("Failed to pause source");
#endif
}

void Source::resume()
{
  alSourcePlay(sourceID);

#if WENDY_DEBUG
  checkAL("Failed to resume source");
#endif
}

bool Source::isStarted() const
{
  return getState() == STARTED;
}

bool Source::isPaused() const
{
  return getState() == PAUSED;
}

bool Source::isStopped() const
{
  return getState() == STOPPED;
}

bool Source::isLooping() const
{
  return looping;
}

Source::State Source::getState() const
{
  ALenum state;
  alGetSourcei(sourceID, AL_SOURCE_STATE, &state);

#if WENDY_DEBUG
  checkAL("Failed to get source state");
#endif

  switch (state)
  {
    case AL_INITIAL:
    case AL_STOPPED:
      return STOPPED;
    case AL_PLAYING:
      return STARTED;
    case AL_PAUSED:
      return PAUSED;
  }

  panic("Unknown OpenAL source state %u", state);
}

void Source::setLooping(bool newState)
{
  if (looping != newState)
  {
    looping = newState;
    alSourcei(sourceID, AL_LOOPING, looping);

#if WENDY_DEBUG
    checkAL("Failed to set source looping state");
#endif
  }
}

const vec3& Source::getPosition() const
{
  return position;
}

void Source::setPosition(const vec3& newPosition)
{
  if (position != newPosition)
  {
    position = newPosition;
    alSourcefv(sourceID, AL_POSITION, value_ptr(position));

#if WENDY_DEBUG
    checkAL("Failed to set source position");
#endif
  }
}

const vec3& Source::getVelocity() const
{
  return velocity;
}

void Source::setVelocity(const vec3& newVelocity)
{
  if (velocity != newVelocity)
  {
    velocity = newVelocity;
    alSourcefv(sourceID, AL_VELOCITY, value_ptr(velocity));

#if WENDY_DEBUG
    checkAL("Failed to set source velocity");
#endif
  }
}

Buffer* Source::getBuffer() const
{
  return buffer;
}

void Source::setBuffer(Buffer* newBuffer)
{
  if (buffer != newBuffer)
  {
    buffer = newBuffer;

    if (buffer)
      alSourcei(sourceID, AL_BUFFER, buffer->bufferID);
    else
      alSourcei(sourceID, AL_BUFFER, AL_NONE);

#if WENDY_DEBUG
    checkAL("Failed to set source buffer");
#endif
  }
}

float Source::getGain() const
{
  return gain;
}

void Source::setGain(float newGain)
{
  if (gain != newGain)
  {
    gain = newGain;
    alSourcefv(sourceID, AL_GAIN, &gain);

#if WENDY_DEBUG
    checkAL("Failed to set source gain");
#endif
  }
}

float Source::getPitch() const
{
  return pitch;
}

void Source::setPitch(float newPitch)
{
  if (pitch != newPitch)
  {
    pitch = newPitch;
    alSourcefv(sourceID, AL_PITCH, &pitch);

#if WENDY_DEBUG
    checkAL("Failed to set source pitch");
#endif
  }
}

Context& Source::getContext() const
{
  return context;
}

Ref<Source> Source::create(Context& context)
{
  Ref<Source> source = new Source(context);
  if (!source->init())
    return NULL;

  return source;
}

Source::Source(Context& initContext):
  context(initContext),
  sourceID(0),
  looping(false),
  gain(1.f),
  pitch(1.f)
{
}

bool Source::init()
{
  alGenSources(1, &sourceID);

  if (!checkAL("Error during audio buffer creation"))
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
