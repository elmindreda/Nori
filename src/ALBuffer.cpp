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

#include <wendy/Config.hpp>
#include <wendy/Core.hpp>
#include <wendy/Path.hpp>
#include <wendy/Resource.hpp>
#include <wendy/Sample.hpp>

#include <wendy/ALContext.hpp>
#include <wendy/ALBuffer.hpp>

#include <internal/ALHelper.hpp>

#include <al.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace AL
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

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Buffer::~Buffer()
{
  if (m_bufferID)
    alDeleteBuffers(1, &m_bufferID);
}

bool Buffer::isMono() const
{
  return m_format == SAMPLE_MONO8 || m_format == SAMPLE_MONO16;
}

bool Buffer::isStereo() const
{
  return m_format == SAMPLE_STEREO8 || m_format == SAMPLE_STEREO16;
}

Ref<Buffer> Buffer::create(const ResourceInfo& info,
                           Context& context,
                           const Sample& data)
{
  Ref<Buffer> buffer = new Buffer(info, context);
  if (!buffer->init(data))
    return NULL;

  return buffer;
}

Ref<Buffer> Buffer::read(Context& context, const String& sampleName)
{
  ResourceCache& cache = context.cache();

  String name;
  name += "sample:";
  name += sampleName;

  if (Ref<Buffer> buffer = cache.find<Buffer>(name))
    return buffer;

  Ref<Sample> data = Sample::read(cache, sampleName);
  if (!data)
  {
    logError("Failed to read sample for buffer %s", name.c_str());
    return NULL;
  }

  return create(ResourceInfo(cache, name), context, *data);
}

Buffer::Buffer(const ResourceInfo& info, Context& context):
  Resource(info),
  m_context(context),
  m_bufferID(0),
  m_duration(0.0)
{
}

Buffer::Buffer(const Buffer& source):
  Resource(source),
  m_context(source.m_context)
{
  panic("OpenAL buffer objects may not be copied");
}

bool Buffer::init(const Sample& data)
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

Buffer& Buffer::operator = (const Buffer& source)
{
  panic("OpenAL buffer objects may not be assigned");
}

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
