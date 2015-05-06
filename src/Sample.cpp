///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

#include <nori/Config.hpp>
#include <nori/Core.hpp>
#include <nori/Path.hpp>
#include <nori/Resource.hpp>
#include <nori/Sample.hpp>

#include <stb_vorbis.c>

namespace nori
{

Sample::Sample(const ResourceInfo& info,
               const char* data,
               size_t size,
               SampleFormat format,
               uint frequency):
  Resource(info),
  data(data, data + size),
  format(format),
  frequency(frequency)
{
}

Ref<Sample> Sample::read(ResourceCache& cache, const std::string& name)
{
  if (Sample* cached = cache.find<Sample>(name))
    return cached;

  const Path path = cache.findFile(name);
  if (path.isEmpty())
  {
    logError("Failed to find sample %s", name.c_str());
    return nullptr;
  }

  stb_vorbis* file;
  int channels, rate;
  short* samples;

  const int length = stb_vorbis_decode_filename(path.name().c_str(),
                                                &channels, &rate, &samples);
  if (length < 1)
  {
    logError("Failed to read audio file %s", path.name().c_str());
    return nullptr;
  }

  SampleFormat format;
  if (channels == 1)
    format = SAMPLE_MONO16;
  else
    format = SAMPLE_STEREO16;

  Ref<Sample> sample = new Sample(ResourceInfo(cache, name, path),
                                  (const char*) samples, length * sizeof(short),
                                  format, rate);

  free(samples);
  return sample;
}

} /*namespace nori*/

