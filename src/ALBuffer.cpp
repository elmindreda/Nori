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

#include <alut.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace AL
  {

///////////////////////////////////////////////////////////////////////

namespace
{

ALenum convertToAL(BufferFormat format)
{
  switch (format)
  {
    case FORMAT_MONO8:
      return AL_FORMAT_MONO8;
    case FORMAT_MONO16:
      return AL_FORMAT_MONO16;
    case FORMAT_STEREO8:
      return AL_FORMAT_STEREO8;
    case FORMAT_STEREO16:
      return AL_FORMAT_STEREO16;
  }

  logError("Invalid OpenAL buffer data format %u", format);
  return 0;
}

BufferFormat convertFormat(ALenum format)
{
  switch (format)
  {
    case AL_FORMAT_MONO8:
      return FORMAT_MONO8;
    case AL_FORMAT_MONO16:
      return FORMAT_MONO16;
    case AL_FORMAT_STEREO8:
      return FORMAT_STEREO8;
    case AL_FORMAT_STEREO16:
      return FORMAT_STEREO16;
  }

  logError("Unsupported OpenAL buffer format %u", format);
  return BufferFormat(0);
}

size_t getFormatSize(BufferFormat format)
{
  switch (format)
  {
    case FORMAT_MONO8:
      return 1;
    case FORMAT_MONO16:
    case FORMAT_STEREO8:
      return 2;
    case FORMAT_STEREO16:
      return 4;
  }

  logError("Invalid OpenAL buffer data format %u", format);
  return 0;
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

BufferData::BufferData(const void* initData,
                       size_t initSize,
                       BufferFormat initFormat,
                       float initFrequency):
  data(initData),
  size(initSize),
  format(initFormat),
  frequency(initFrequency)
{
}

///////////////////////////////////////////////////////////////////////

Buffer::~Buffer(void)
{
  if (bufferID)
    alDeleteBuffers(1, &bufferID);
}

bool Buffer::isMono(void) const
{
  if (format == FORMAT_MONO8 || format == FORMAT_MONO16)
    return true;

  return false;
}

bool Buffer::isStereo(void) const
{
  if (format == FORMAT_STEREO8 || format == FORMAT_STEREO16)
    return true;

  return false;
}

Time Buffer::getDuration(void) const
{
  return duration;
}

BufferFormat Buffer::getFormat(void) const
{
  return format;
}

Context& Buffer::getContext(void) const
{
  return context;
}

Ref<Buffer> Buffer::create(const ResourceInfo& info, Context& context, const BufferData& data)
{
  Ref<Buffer> buffer = new Buffer(info, context);
  if (!buffer->init(data))
    return NULL;

  return buffer;
}

Ref<Buffer> Buffer::read(Context& context, const Path& path)
{
  BufferReader reader(context);
  return reader.read(path);
}

Buffer::Buffer(const ResourceInfo& info, Context& initContext):
  Resource(info),
  context(initContext),
  bufferID(0),
  duration(0.0)
{
}

Buffer::Buffer(const Buffer& source):
  Resource(source),
  context(source.context)
{
  // NOTE: Not implemented.
}

bool Buffer::init(const BufferData& data)
{
  alGenBuffers(1, &bufferID);
  alBufferData(bufferID,
               convertToAL(data.format),
               data.data, data.size,
               data.frequency);

  if (!checkAL("Error during OpenAL buffer creation"))
    return false;

  format = data.format;

  duration = float(data.size) / (getFormatSize(format) * data.frequency);

  return true;
}

Buffer& Buffer::operator = (const Buffer& source)
{
  // NOTE: Not implemented.

  return *this;
}

///////////////////////////////////////////////////////////////////////

BufferReader::BufferReader(Context& initContext):
  ResourceReader(initContext.getIndex()),
  context(initContext)
{
}

Ref<Buffer> BufferReader::read(const Path& path)
{
  if (Resource* cache = getIndex().findResource(path))
    return dynamic_cast<Buffer*>(cache);

  ALenum format;
  ALsizei size;
  ALfloat frequency;

  void* mem = alutLoadMemoryFromFile(path.asString().c_str(),
                                     &format,
                                     &size,
                                     &frequency);
  if (!mem)
  {
    logError("Failed to load audio file \'%s\': %s",
             path.asString().c_str(),
             alutGetErrorString(alutGetError()));
    return false;
  }

  ResourceInfo info(getIndex(), path);
  BufferData data(mem, size, convertFormat(format), frequency);
  Ref<Buffer> buffer = Buffer::create(info, context, data);

  std::free(mem);

  return buffer;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
