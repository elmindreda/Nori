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

#include <wendy/ALContext.h>
#include <wendy/ALBuffer.h>

#include <internal/ALHelper.h>

#include <al.h>

#include <vorbis/vorbisfile.h>

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

  panic("Invalid OpenAL buffer data format %u", format);
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

  panic("Invalid OpenAL buffer data format %u", format);
}

const char* getErrorString(int error)
{
  switch (error)
  {
    case OV_EREAD:
      return "A read from media returned an error";
    case OV_ENOTVORBIS:
      return "Bitstream does not contain any Vorbis data";
    case OV_EVERSION:
      return "Vorbis version mismatch";
    case OV_EBADHEADER:
      return "Invalid Vorbis bitstream header";
    case OV_EFAULT:
      return "Internal logic fault; indicates a bug or heap/stack corruption";
  }

  return "Unknown vorbisfile error";
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

BufferData::BufferData(const void* initData,
                       size_t initSize,
                       BufferFormat initFormat,
                       unsigned long initFrequency):
  data(initData),
  size(initSize),
  format(initFormat),
  frequency(initFrequency)
{
}

///////////////////////////////////////////////////////////////////////

Buffer::~Buffer()
{
  if (bufferID)
    alDeleteBuffers(1, &bufferID);
}

bool Buffer::isMono() const
{
  if (format == FORMAT_MONO8 || format == FORMAT_MONO16)
    return true;

  return false;
}

bool Buffer::isStereo() const
{
  if (format == FORMAT_STEREO8 || format == FORMAT_STEREO16)
    return true;

  return false;
}

Time Buffer::getDuration() const
{
  return duration;
}

BufferFormat Buffer::getFormat() const
{
  return format;
}

Context& Buffer::getContext() const
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

Ref<Buffer> Buffer::read(Context& context, const String& name)
{
  BufferReader reader(context);
  return reader.read(name);
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
  panic("OpenAL buffer objects may not be copied");
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
  panic("OpenAL buffer objects may not be assigned");
}

///////////////////////////////////////////////////////////////////////

BufferReader::BufferReader(Context& initContext):
  ResourceReader<Buffer>(initContext.getCache()),
  context(initContext)
{
}

Ref<Buffer> BufferReader::read(const String& name, const Path& path)
{
  int result;
  OggVorbis_File file;

  result = ov_fopen(path.asString().c_str(), &file);
  if (result)
  {
    logError("Failed to open audio file \'%s\': %s",
             name.c_str(),
             getErrorString(result));
    return NULL;
  }

  if (ov_streams(&file) > 1)
  {
    ov_clear(&file);
    logError("Audio file \'%s\' has an unsupported number of bitstreams",
             name.c_str());
    return NULL;
  }

  const vorbis_info* info = ov_info(&file, -1);
  if (!info)
  {
    ov_clear(&file);
    logError("Failed to retrieve Vorbis info for audio file \'%s\'",
             name.c_str());
    return NULL;
  }

  if (info->channels > 2)
  {
    ov_clear(&file);
    logError("Audio file \'%s\' has an unsupported number of channels",
             name.c_str());
    return NULL;
  }

  BufferFormat format;
  if (info->channels == 1)
    format = FORMAT_MONO16;
  else
    format = FORMAT_STEREO16;

  std::vector<char> samples;

#if WENDY_WORDS_BIGENDIAN
  const int endian = 1;
#else
  const int endian = 0;
#endif

  for (;;)
  {
    int bitstream;
    char scratch[4096];

    result = ov_read(&file, scratch, sizeof(scratch), endian, 2, 1, &bitstream);
    if (result < 0)
    {
      ov_clear(&file);
      logError("Error when reading audio file \'%s\': %s",
              name.c_str(),
              getErrorString(result));
      return NULL;
    }
    else if (result > 0)
      samples.insert(samples.end(), scratch, scratch + result);
    else
      break;
  }

  ov_clear(&file);

  const BufferData data(&samples[0], samples.size(), format, info->rate);

  return Buffer::create(ResourceInfo(cache, name, path), context, data);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
