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

Source::~Source(void)
{
  if (sourceID)
    alDeleteSources(1, &sourceID);
}

void Source::start(void)
{
  stop();

  alSourcePlay(sourceID);
  started = true;
}

void Source::stop(void)
{
  if (started)
  {
    alSourceStop(sourceID);

    started = false;
    paused = false;
  }
}

void Source::pause(void)
{
  if (!started || paused)
    return;

  alSourcePause(sourceID);
  paused = true;
}

void Source::resume(void)
{
  if (!started || !paused)
    return;

  alSourcePlay(sourceID);
  paused = false;
}

bool Source::isStarted(void) const
{
  return started;
}

bool Source::isPaused(void) const
{
  return paused;
}

bool Source::isLooping(void) const
{
  return looping;
}

void Source::setLooping(bool newState)
{
  if (looping != newState)
  {
    alSourcei(sourceID, AL_LOOPING, newState);
    looping = newState;
  }
}

const vec3& Source::getPosition(void) const
{
  return position;
}

void Source::setPosition(const vec3& newPosition)
{
  if (position != newPosition)
  {
    position = newPosition;
    alSourcefv(sourceID, AL_POSITION, value_ptr(position));
  }
}

const vec3& Source::getVelocity(void) const
{
  return velocity;
}

void Source::setVelocity(const vec3& newVelocity)
{
  if (velocity != newVelocity)
  {
    velocity = newVelocity;
    alSourcefv(sourceID, AL_VELOCITY, value_ptr(velocity));
  }
}

Buffer* Source::getBuffer(void) const
{
  return buffer;
}

void Source::setBuffer(Buffer* newBuffer)
{
  buffer = newBuffer;

  if (buffer)
    alSourcei(sourceID, AL_BUFFER, buffer->bufferID);
  else
    alSourcei(sourceID, AL_BUFFER, AL_NONE);
}

Context& Source::getContext(void) const
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
  started(false),
  paused(false),
  looping(false)
{
}

bool Source::init(void)
{
  alGenSources(1, &sourceID);

  if (!checkAL("Error during audio buffer creation"))
    return false;

  alSourcei(sourceID, AL_LOOPING, looping);
  alSourcefv(sourceID, AL_POSITION, value_ptr(position));
  alSourcefv(sourceID, AL_VELOCITY, value_ptr(velocity));

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
