///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#pragma once

#include <wendy/Core.hpp>
#include <wendy/Time.hpp>
#include <wendy/Path.hpp>
#include <wendy/Resource.hpp>
#include <wendy/Sample.hpp>

namespace wendy
{

class AudioContext;

/*! @brief Audio sample data buffer.
 *  @ingroup audio
 */
class AudioBuffer : public Resource, public RefObject
{
  friend class AudioSource;
public:
  /*! Destructor.
   */
  ~AudioBuffer();
  /*! @return @c true if this buffer contains mono data, otherwise @c false.
   */
  bool isMono() const;
  /*! @return @c true if this buffer contains stereo data, otherwise @c false.
   */
  bool isStereo() const;
  /*! @return The duration, in seconds, of this buffer.
   */
  Time duration() const { return m_duration; }
  /*! @return The format of the data in this buffer.
   */
  SampleFormat format() const { return m_format; }
  /*! @return The context within which this buffer was created.
   */
  AudioContext& context() const { return m_context; }
  /*! Creates a buffer object within the specified context using the specified
   *  data.
   */
  static Ref<AudioBuffer> create(const ResourceInfo& info,
                                 AudioContext& context,
                                 const Sample& data);
  static Ref<AudioBuffer> read(AudioContext& context, const std::string& sampleName);
private:
  AudioBuffer(const ResourceInfo& info, AudioContext& context);
  AudioBuffer(const AudioBuffer&) = delete;
  bool init(const Sample& data);
  AudioBuffer& operator = (const AudioBuffer&) = delete;
  AudioContext& m_context;
  uint m_bufferID;
  SampleFormat m_format;
  Time m_duration;
};

/*! @brief Audio source.
 *  @ingroup audio
 */
class AudioSource : public RefObject
{
public:
  /*! Audio source state enumeration.
   */
  enum State
  {
    /*! The source is playing the currently set buffer.
     */
    STARTED,
    /*! The source is playing but paused.
     */
    PAUSED,
    /*! The source is stopped or has never been played.
     */
    STOPPED
  };
  /*! Destructor.
   */
  ~AudioSource();
  /*! Starts this source playing the currently set buffer.
   */
  void start();
  /*! Stops the playing of this source.
   */
  void stop();
  /*! Pauses the playing of this source.
   *
   *  @remarks This has no effect unless this source was previously playing.
   */
  void pause();
  /*! Resumes the playing of this source.
   *
   *  @remarks This has no effect unless this source was previously paused.
   */
  void resume();
  /*! @return @c true if this source is in the Source::STARTED state.
   */
  bool isStarted() const { return state() == STARTED; }
  /*! @return @c true if this source is in the Source::PAUSED state.
   */
  bool isPaused() const { return state() == PAUSED; }
  /*! @return @c true if this source is in the Source::STOPPED state.
   */
  bool isStopped() const { return state() == STOPPED; }
  /*! @return @c true if this source loops playback.
   */
  bool isLooping() const { return m_looping; }
  /*! @return The state of this source.
   */
  State state() const;
  /*! Sets whether this source loops playback.
   */
  void setLooping(bool newState);
  /*! @return The position of this source.
   */
  const vec3& position() const { return m_position; }
  /*! Sets the position of this source.
   */
  void setPosition(const vec3& newPosition);
  /*! @return The velocity of this source.
   *
   *  @remarks The velocity doesn't affect the position of the source, but is
   *  used in combination with the listener velocity to calculate doppler shift.
   */
  const vec3& velocity() const { return m_velocity; }
  /*! Sets the velocity of this source.
   *
   *  @remarks The velocity doesn't affect the position of the source, but is
   *  used in combination with the listener velocity to calculate doppler shift.
   */
  void setVelocity(const vec3& newVelocity);
  /*! @return The gain of this source.
   */
  float gain() const { return m_gain; }
  /*! Sets the gain of this source.
   */
  void setGain(float newGain);
  /*! @return The pitch of this source.
   */
  float pitch() const { return m_pitch; }
  /*! Sets the pitch of this source.
   */
  void setPitch(float newPitch);
  /*! @return The currently set buffer for this source, or @c nullptr if no
   *  buffer is set.
   */
  AudioBuffer* buffer() const { return m_buffer; }
  /*! Sets the buffer to be used by this source.
   *  @param[in] newBuffer The buffer to use, or @c nullptr to detach the
   *  currently used buffer.
   */
  void setBuffer(AudioBuffer* newBuffer);
  /*! @return The context within which this buffer was created.
   */
  AudioContext& context() const { return m_context; }
  /*! Creates a source object within the specified context.
   */
  static Ref<AudioSource> create(AudioContext& context);
private:
  AudioSource(AudioContext& context);
  AudioSource(const AudioSource&) = delete;
  bool init();
  AudioSource& operator = (const AudioSource&) = delete;
  AudioContext& m_context;
  uint m_sourceID;
  bool m_looping;
  vec3 m_position;
  vec3 m_velocity;
  float m_gain;
  float m_pitch;
  Ref<AudioBuffer> m_buffer;
};

/*! @brief Audio context.
 *  @ingroup audio
 */
class AudioContext
{
public:
  /*! Destructor.
   */
  ~AudioContext();
  /*! @return The position of the context listener.
   */
  const vec3& listenerPosition() const { return m_listenerPosition; }
  /*! Sets the position of the context listener.
   */
  void setListenerPosition(const vec3& newPosition);
  /*! @return The velocity of the context listener.
   *
   *  @remarks The velocity doesn't affect the position of the source, but is
   *  used in combination with the source velocity to calculate doppler shift.
   */
  const vec3& listenerVelocity() const { return m_listenerVelocity; }
  /*! Sets the velocity of the context listener.
   *
   *  @remarks The velocity doesn't affect the position of the source, but is
   *  used in combination with the source velocity to calculate doppler shift.
   */
  void setListenerVelocity(const vec3& newVelocity);
  /*! @return The gain of the context listener.
   */
  const quat& listenerRotation() const { return m_listenerRotation; }
  /*! Sets the rotation of the context listener.
   */
  void setListenerRotation(const quat& newRotation);
  /*! @return The gain of the context listener.
   */
  float listenerGain() const { return m_listenerGain; }
  /*! Sets the listener gain of this context.
   */
  void setListenerGain(float newGain);
  /*! @return The resource cache used by this context.
   */
  ResourceCache& cache() const { return m_cache; }
  /*! Creates the context singleton object.
   *  @param[in] cache The resource cache to use.
   *  @return @c true if successful, or @c false otherwise.
   */
  static std::unique_ptr<AudioContext> create(ResourceCache& cache);
private:
  AudioContext(ResourceCache& cache);
  AudioContext(const AudioContext&) = delete;
  bool init();
  AudioContext& operator = (const AudioContext&) = delete;
  ResourceCache& m_cache;
  void* m_device;
  void* m_handle;
  vec3 m_listenerPosition;
  vec3 m_listenerVelocity;
  quat m_listenerRotation;
  float m_listenerGain;
};

} /*namespace wendy*/

