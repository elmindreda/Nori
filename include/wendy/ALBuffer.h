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

/*! @ingroup openal
 */
class BufferData
{
public:
  enum Format
  {
    MONO8,
    MONO16,
    STEREO8,
    STEREO16,
  };
  BufferData(const void* data, size_t size, Format format, float frequency);
  const void* data;
  size_t size;
  Format format;
  float frequency;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup openal
 */
class Buffer : public Resource
{
  friend class Source;
public:
  ~Buffer(void);
  Time getDuration(void) const;
  Context& getContext(void) const;
  static Ref<Buffer> create(const ResourceInfo& info, Context& context, const BufferData& data);
  static Ref<Buffer> read(Context& context, const Path& path);
private:
  Buffer(const ResourceInfo& info, Context& context);
  Buffer(const Buffer& source);
  bool init(const BufferData& data);
  Buffer& operator = (const Buffer& source);
  Context& context;
  unsigned int bufferID;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup openal
 */
class BufferReader : public ResourceReader
{
public:
  BufferReader(Context& context);
  Ref<Buffer> read(const Path& path);
private:
  Context& context;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_ALBUFFER_H*/
///////////////////////////////////////////////////////////////////////
