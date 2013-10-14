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
#include <wendy/ALBuffer.hpp>
#include <wendy/ALSource.hpp>

#include <internal/ALHelper.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <al.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

AudioSource::~AudioSource()
{
  if (m_sourceID)
    alDeleteSources(1, &m_sourceID);
}

void AudioSource::start()
{
  alSourcePlay(m_sourceID);

#if WENDY_DEBUG
  checkAL("Failed to start source");
#endif
}

void AudioSource::stop()
{
  alSourceStop(m_sourceID);

#if WENDY_DEBUG
  checkAL("Failed to stop source");
#endif
}

void AudioSource::pause()
{
  alSourcePause(m_sourceID);

#if WENDY_DEBUG
  checkAL("Failed to pause source");
#endif
}

void AudioSource::resume()
{
  alSourcePlay(m_sourceID);

#if WENDY_DEBUG
  checkAL("Failed to resume source");
#endif
}

AudioSource::State AudioSource::state() const
{
  ALenum state;
  alGetSourcei(m_sourceID, AL_SOURCE_STATE, &state);

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

void AudioSource::setLooping(bool newState)
{
  if (m_looping != newState)
  {
    m_looping = newState;
    alSourcei(m_sourceID, AL_LOOPING, m_looping);

#if WENDY_DEBUG
    checkAL("Failed to set source looping state");
#endif
  }
}

void AudioSource::setPosition(const vec3& newPosition)
{
  if (m_position != newPosition)
  {
    m_position = newPosition;
    alSourcefv(m_sourceID, AL_POSITION, value_ptr(m_position));

#if WENDY_DEBUG
    checkAL("Failed to set source position");
#endif
  }
}

void AudioSource::setVelocity(const vec3& newVelocity)
{
  if (m_velocity != newVelocity)
  {
    m_velocity = newVelocity;
    alSourcefv(m_sourceID, AL_VELOCITY, value_ptr(m_velocity));

#if WENDY_DEBUG
    checkAL("Failed to set source velocity");
#endif
  }
}

void AudioSource::setBuffer(AudioBuffer* newBuffer)
{
  if (m_buffer != newBuffer)
  {
    m_buffer = newBuffer;

    if (m_buffer)
      alSourcei(m_sourceID, AL_BUFFER, m_buffer->m_bufferID);
    else
      alSourcei(m_sourceID, AL_BUFFER, AL_NONE);

#if WENDY_DEBUG
    checkAL("Failed to set source buffer");
#endif
  }
}

void AudioSource::setGain(float newGain)
{
  if (m_gain != newGain)
  {
    m_gain = newGain;
    alSourcefv(m_sourceID, AL_GAIN, &m_gain);

#if WENDY_DEBUG
    checkAL("Failed to set source gain");
#endif
  }
}

void AudioSource::setPitch(float newPitch)
{
  if (m_pitch != newPitch)
  {
    m_pitch = newPitch;
    alSourcefv(m_sourceID, AL_PITCH, &m_pitch);

#if WENDY_DEBUG
    checkAL("Failed to set source pitch");
#endif
  }
}

Ref<AudioSource> AudioSource::create(AudioContext& context)
{
  Ref<AudioSource> source = new AudioSource(context);
  if (!source->init())
    return nullptr;

  return source;
}

AudioSource::AudioSource(AudioContext& context):
  m_context(context),
  m_sourceID(0),
  m_looping(false),
  m_gain(1.f),
  m_pitch(1.f)
{
}

bool AudioSource::init()
{
  alGenSources(1, &m_sourceID);

  if (!checkAL("Error during audio buffer creation"))
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
