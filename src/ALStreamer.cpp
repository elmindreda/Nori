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
#include <wendy/ALSource.h>
#include <wendy/ALStreamer.h>

#include <ogg/ogg.h>

#include <vorbis/vorbisfile.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace AL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{

size_t libvorbisIoRead(void* data, size_t size, size_t count, void* source)
{
  Stream* stream = reinterpret_cast<Stream*>(source);

  return stream->read(data, size * count) / size;
}

int libvorbisIoSeek(void* source, ogg_int64_t offset, int whence)
{
  Stream* stream = reinterpret_cast<Stream*>(source);

  if (!stream->isSeekable())
    return -1;

  if (whence == SEEK_CUR)
    offset += stream->getPosition();
  else if (whence == SEEK_END)
    offset += stream->getPosition();

  if (!stream->setPosition(offset))
    return -1;

  return 0;
}

int libvorbisIoClose(void* source)
{
  Stream* stream = reinterpret_cast<Stream*>(source);

  delete stream;
}

long libvorbisIoTell(void* source)
{
  Stream* stream = reinterpret_cast<Stream*>(source);

  return stream->getPosition();
}

}

///////////////////////////////////////////////////////////////////////

Streamer::~Streamer(void)
{
}

Streamer* Streamer::createInstance(const Path& path, const String& name)
{
  Ptr<Streamer> buffer = new Streamer(name);
  if (!buffer->init(path))
    return NULL;

  return buffer.detachObject();
}

Streamer* Streamer::createInstance(Stream& stream, const String& name)
{
  Ptr<Streamer> buffer = new Streamer(name);
  if (!buffer->init(stream))
    return NULL;

  return buffer.detachObject();
}

Streamer::Streamer(const String& name):
  Managed<Streamer>(name)
{
}

bool Streamer::init(const Path& path)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create audio buffer without OpenAL context");
    return false;
  }

  return true;
}

bool Streamer::init(Stream& stream)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create audio buffer without OpenAL context");
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
