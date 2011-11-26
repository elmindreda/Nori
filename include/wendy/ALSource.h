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
#ifndef WENDY_ALSOURCE_H
#define WENDY_ALSOURCE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace AL
  {

///////////////////////////////////////////////////////////////////////

/*! @brief OpenAL audio source.
 *  @ingroup openal
 */
class Source : public RefObject
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
  ~Source();
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
  bool isStarted() const;
  /*! @return @c true if this source is in the Source::PAUSED state.
   */
  bool isPaused() const;
  /*! @return @c true if this source is in the Source::STOPPED state.
   */
  bool isStopped() const;
  /*! @return @c true if this source loops playback.
   */
  bool isLooping() const;
  /*! @return The state of this source.
   */
  State getState() const;
  /*! Sets whether this source loops playback.
   */
  void setLooping(bool newState);
  /*! @return The position of this source.
   */
  const vec3& getPosition() const;
  /*! Sets the position of this source.
   */
  void setPosition(const vec3& newPosition);
  /*! @return The velocity of this source.
   *
   *  @remarks The velocity doesn't affect the position of the source, but is
   *  used in combination with the listener velocity to calculate doppler shift.
   */
  const vec3& getVelocity() const;
  /*! Sets the velocity of this source.
   *
   *  @remarks The velocity doesn't affect the position of the source, but is
   *  used in combination with the listener velocity to calculate doppler shift.
   */
  void setVelocity(const vec3& newVelocity);
  /*! @return The gain of this source.
   */
  float getGain() const;
  /*! Sets the gain of this source.
   */
  void setGain(float newGain);
  /*! @return The pitch of this source.
   */
  float getPitch() const;
  /*! Sets the pitch of this source.
   */
  void setPitch(float newPitch);
  /*! @return The currently set buffer for this source, or @c NULL if no buffer
   *  is set.
   */
  Buffer* getBuffer() const;
  /*! Sets the buffer to be used by this source.
   *  @param[in] newBuffer The buffer to use, or @c NULL to detach the
   *  currently used buffer.
   */
  void setBuffer(Buffer* newBuffer);
  /*! @return The context within which this buffer was created.
   */
  Context& getContext() const;
  /*! Creates a source object within the specified context.
   */
  static Ref<Source> create(Context& context);
private:
  Source(Context& context);
  Source(const Source& source);
  bool init();
  Source& operator = (const Source& source);
  Context& context;
  unsigned int sourceID;
  bool looping;
  vec3 position;
  vec3 velocity;
  float gain;
  float pitch;
  Ref<Buffer> buffer;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_ALSOURCE_H*/
///////////////////////////////////////////////////////////////////////
