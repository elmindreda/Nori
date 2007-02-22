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

#include <moira/Moira.h>

#include <wendy/Config.h>
#include <wendy/OpenAL.h>
#include <wendy/ALContext.h>
#include <wendy/ALBuffer.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace AL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Buffer::~Buffer(void)
{
  if (bufferID)
    alDeleteBuffers(1, &bufferID);
}

Buffer* Buffer::createInstance(const Path& path, const String& name)
{
  Ptr<Buffer> buffer = new Buffer(name);
  if (!buffer->init(path))
    return NULL;

  return buffer.detachObject();
}

Buffer* Buffer::createInstance(Stream& stream, const String& name)
{
  Ptr<Buffer> buffer = new Buffer(name);
  if (!buffer->init(stream))
    return NULL;

  return buffer.detachObject();
}

Buffer::Buffer(const String& name):
  Managed<Buffer>(name),
  bufferID(0)
{
}

bool Buffer::init(const Path& path)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create audio buffer without OpenAL context");
    return false;
  }

  bufferID = alutCreateBufferFromFile(path.asString().c_str());
  if (bufferID == AL_NONE)
  {
    Log::writeError("Error during audio buffer creation: %s",
                    alutGetErrorString(alutGetError()));
    return false;
  }

  return true;
}

bool Buffer::init(Stream& stream)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create audio buffer without OpenAL context");
    return false;
  }

  Block data;
  data.resize(stream.getSize());

  if (!stream.readItems(data.getData(), data.getSize()))
  {
    Log::writeError("Failed to read audio data from stream");
    return false;
  }

  bufferID = alutCreateBufferFromFileImage(data, data.getSize());
  if (bufferID == AL_NONE)
  {
    Log::writeError("Error during audio buffer creation: %s",
                    alutGetErrorString(alutGetError()));
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
