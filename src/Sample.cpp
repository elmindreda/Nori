///////////////////////////////////////////////////////////////////////
// Wendy OpenAL library
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/Path.h>
#include <wendy/Resource.h>
#include <wendy/Sample.h>

#include <vorbis/vorbisfile.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

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

Sample::Sample(const ResourceInfo& info,
               const char* initData,
               size_t initSize,
               SampleFormat initFormat,
               unsigned long initFrequency):
  Resource(info),
  data(initData, initData + initSize),
  format(initFormat),
  frequency(initFrequency)
{
}

Ref<Sample> Sample::read(ResourceCache& cache, const String& name)
{
  SampleReader reader(cache);
  return reader.read(name);
}

///////////////////////////////////////////////////////////////////////

SampleReader::SampleReader(ResourceCache& cache):
  ResourceReader(cache)
{
}

Ref<Sample> SampleReader::read(const String& name, const Path& path)
{
  int result;
  OggVorbis_File file;

  result = ov_fopen(path.asString().c_str(), &file);
  if (result)
  {
    logError("Failed to open audio file \'%s\': %s",
             path.asString().c_str(),
             getErrorString(result));
    return NULL;
  }

  if (ov_streams(&file) > 1)
  {
    ov_clear(&file);
    logError("Audio file \'%s\' has an unsupported number of bitstreams",
             path.asString().c_str());
    return NULL;
  }

  const vorbis_info* info = ov_info(&file, -1);
  if (!info)
  {
    ov_clear(&file);
    logError("Failed to retrieve Vorbis info for audio file \'%s\'",
             path.asString().c_str());
    return NULL;
  }

  if (info->channels > 2)
  {
    ov_clear(&file);
    logError("Audio file \'%s\' has an unsupported number of channels",
             path.asString().c_str());
    return NULL;
  }

  SampleFormat format;
  if (info->channels == 1)
    format = SAMPLE_MONO16;
  else
    format = SAMPLE_STEREO16;

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
              path.asString().c_str(),
              getErrorString(result));
      return NULL;
    }
    else if (result > 0)
      samples.insert(samples.end(), scratch, scratch + result);
    else
      break;
  }

  ov_clear(&file);

  return new Sample(ResourceInfo(cache, name, path),
                    &samples[0],
                    samples.size(),
                    format,
                    info->rate);
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
