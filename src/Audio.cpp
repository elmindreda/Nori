///////////////////////////////////////////////////////////////////////
// Wendy audio library
// Copyright (c) 2013 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/Path.hpp>
#include <wendy/Resource.hpp>
#include <wendy/Sample.hpp>
#include <wendy/Audio.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <al.h>
#include <alc.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

ALenum convertToAL(SampleFormat format)
{
  switch (format)
  {
    case SAMPLE_MONO8:
      return AL_FORMAT_MONO8;
    case SAMPLE_MONO16:
      return AL_FORMAT_MONO16;
    case SAMPLE_STEREO8:
      return AL_FORMAT_STEREO8;
    case SAMPLE_STEREO16:
      return AL_FORMAT_STEREO16;
  }

  panic("Invalid OpenAL buffer data format %u", format);
}

size_t getFormatSize(SampleFormat format)
{
  switch (format)
  {
    case SAMPLE_MONO8:
      return 1;
    case SAMPLE_MONO16:
    case SAMPLE_STEREO8:
      return 2;
    case SAMPLE_STEREO16:
      return 4;
  }

  panic("Invalid OpenAL buffer data format %u", format);
}

const char* getErrorStringAL(ALenum error)
{
  switch (error)
  {
    case AL_INVALID_NAME:
      return "Invalid name parameter";
    case AL_INVALID_ENUM:
      return "Invalid enum parameter";
    case AL_INVALID_VALUE:
      return "Invalid enum parameter value";
    case AL_INVALID_OPERATION:
      return "Invalid operation";
    case AL_OUT_OF_MEMORY:
      return "Out of memory";
  }

  return "Unknown OpenAL error";
}

const char* getErrorStringALC(ALCenum error)
{
  switch (error)
  {
    case ALC_INVALID_DEVICE:
      return "Invalid device";
    case ALC_INVALID_CONTEXT:
      return "Invalid context";
    case ALC_INVALID_ENUM:
      return "Invalid enum parameter";
    case ALC_INVALID_VALUE:
      return "Invalid enum parameter value";
    case ALC_OUT_OF_MEMORY:
      return "Out of memory";
  }

  return "Unknown OpenAL error";
}

bool checkAL(const char* format, ...)
{
  ALenum error = alGetError();
  if (error == AL_NO_ERROR)
    return true;

  va_list vl;

  va_start(vl, format);
  String message = vlformat(format, vl);
  va_end(vl);

  logError("%s: %s", message.c_str(), getErrorStringAL(error));
  return false;
}

bool checkALC(const char* format, ...)
{
  ALCenum error = alcGetError(alcGetContextsDevice(alcGetCurrentContext()));
  if (error == ALC_NO_ERROR)
    return true;

  va_list vl;

  va_start(vl, format);
  String message = vlformat(format, vl);
  va_end(vl);

  logError("%s: %s", message.c_str(), getErrorStringALC(error));
  return false;
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

AudioBuffer::~AudioBuffer()
{
  if (m_bufferID)
    alDeleteBuffers(1, &m_bufferID);
}

bool AudioBuffer::isMono() const
{
  return m_format == SAMPLE_MONO8 || m_format == SAMPLE_MONO16;
}

bool AudioBuffer::isStereo() const
{
  return m_format == SAMPLE_STEREO8 || m_format == SAMPLE_STEREO16;
}

Ref<AudioBuffer> AudioBuffer::create(const ResourceInfo& info,
                                     AudioContext& context,
                                     const Sample& data)
{
  Ref<AudioBuffer> buffer = new AudioBuffer(info, context);
  if (!buffer->init(data))
    return NULL;

  return buffer;
}

Ref<AudioBuffer> AudioBuffer::read(AudioContext& context, const String& sampleName)
{
  ResourceCache& cache = context.cache();

  String name;
  name += "sample:";
  name += sampleName;

  if (Ref<AudioBuffer> buffer = cache.find<AudioBuffer>(name))
    return buffer;

  Ref<Sample> data = Sample::read(cache, sampleName);
  if (!data)
  {
    logError("Failed to read sample for buffer %s", name.c_str());
    return NULL;
  }

  return create(ResourceInfo(cache, name), context, *data);
}

AudioBuffer::AudioBuffer(const ResourceInfo& info, AudioContext& context):
  Resource(info),
  m_context(context),
  m_bufferID(0),
  m_duration(0.0)
{
}

bool AudioBuffer::init(const Sample& data)
{
  alGenBuffers(1, &m_bufferID);
  alBufferData(m_bufferID,
               convertToAL(data.format),
               &data.data[0], data.data.size(),
               data.frequency);

  if (!checkAL("Error during OpenAL buffer creation"))
    return false;

  m_format = data.format;
  m_duration = Time(data.data.size()) / (getFormatSize(m_format) * data.frequency);

  return true;
}

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

void AudioContext::makeCurrent()
{
  if (!alcMakeContextCurrent((ALCcontext*) m_handle))
    checkALC("Failed to make OpenAL context current");
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

void AudioContext::clearCurrentContext()
{
  alcMakeContextCurrent(nullptr);
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
