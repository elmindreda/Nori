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
#ifndef WENDY_ALBUFFER_H
#define WENDY_ALBUFFER_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Path.h>
#include <wendy/Resource.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace AL
  {

///////////////////////////////////////////////////////////////////////

/*! @brief Audio sample data format enumeration.
 *  @ingroup openal
 */
enum BufferFormat
{
  FORMAT_MONO8,
  FORMAT_MONO16,
  FORMAT_STEREO8,
  FORMAT_STEREO16
};

///////////////////////////////////////////////////////////////////////

/*! @brief Audio sample data descriptor.
 *  @ingroup openal
 */
class BufferData
{
public:
  BufferData(const void* data, size_t size, BufferFormat format, unsigned long frequency);
  const void* data;
  size_t size;
  BufferFormat format;
  unsigned long frequency;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Audio sample data buffer.
 *  @ingroup openal
 */
class Buffer : public Resource
{
  friend class Source;
public:
  /*! Destructor.
   */
  ~Buffer();
  /*! @return @c true if this buffer contains mono data, otherwise @c false.
   */
  bool isMono() const;
  /*! @return @c true if this buffer contains stereo data, otherwise @c false.
   */
  bool isStereo() const;
  /*! @return The duration, in seconds, of this buffer.
   */
  Time getDuration() const;
  /*! @return The format of the data in this buffer.
   */
  BufferFormat getFormat() const;
  /*! @return The context within which this buffer was created.
   */
  Context& getContext() const;
  /*! Creates a buffer object within the specified context using the specified
   *  data.
   */
  static Ref<Buffer> create(const ResourceInfo& info,
                            Context& context,
                            const BufferData& data);
  /*! Creates a buffer object within the specified context using data from the
   *  file at the specified path.
   */
  static Ref<Buffer> read(Context& context, const String& name);
private:
  Buffer(const ResourceInfo& info, Context& context);
  Buffer(const Buffer& source);
  bool init(const BufferData& data);
  Buffer& operator = (const Buffer& source);
  Context& context;
  unsigned int bufferID;
  BufferFormat format;
  Time duration;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup openal
 */
class BufferReader : public ResourceReader<Buffer>
{
public:
  BufferReader(Context& context);
  using ResourceReader<Buffer>::read;
  Ref<Buffer> read(const String& name, const Path& path);
private:
  Context& context;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_ALBUFFER_H*/
///////////////////////////////////////////////////////////////////////
