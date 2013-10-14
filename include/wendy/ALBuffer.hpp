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
#ifndef WENDY_ALBUFFER_HPP
#define WENDY_ALBUFFER_HPP
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.hpp>
#include <wendy/Path.hpp>
#include <wendy/Resource.hpp>
#include <wendy/Sample.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief Audio sample data buffer.
 *  @ingroup openal
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
  static Ref<AudioBuffer> read(AudioContext& context, const String& sampleName);
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

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_ALBUFFER_HPP*/
///////////////////////////////////////////////////////////////////////
