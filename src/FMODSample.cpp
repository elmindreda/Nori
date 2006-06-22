///////////////////////////////////////////////////////////////////////
// Wendy FMOD library
// Copyright (c) 2004 Camilla Berglund <elmindreda@home.se>
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

#include <moira/Config.h>
#include <moira/Core.h>
#include <moira/Log.h>
#include <moira/Stream.h>

#include <wendy/Config.h>
#include <wendy/FMOD.h>
#include <wendy/FMODSample.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace FMOD
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Sample::Sample(void):
  started(false),
  stream(NULL),
  channel(0)
{
}

Sample::~Sample(void)
{
  stop();
  
  if (stream)
  {
    FSOUND_Stream_Close(stream);
    stream = NULL;
  }
}

void Sample::start(void)
{
  stop();

  channel = FSOUND_Stream_Play(FSOUND_FREE, stream);
  started = true;
}

void Sample::stop(void)
{
  if (started)
  {
    FSOUND_Stream_Stop(stream);
    channel = 0;
    started = false;
  }
}

void Sample::pause(void)
{
  if (!started || isPaused())
    return;
  
  FSOUND_SetPaused(channel, true);
}

void Sample::resume(void)
{
  if (!started || !isPaused())
    return;
  
  FSOUND_SetPaused(channel, false);
}

bool Sample::isStarted(void) const
{
  return started;
}

bool Sample::isPaused(void) const
{
  return FSOUND_GetPaused(channel) ? true : false;
}

Time Sample::getTime(void) const
{
  return FSOUND_Stream_GetTime(stream) / 1000.0;
}

void Sample::setTime(Time newTime)
{
  FSOUND_Stream_SetTime(stream, (int) (newTime * 1000.0));
}

bool Sample::init(const Path& path)
{
  Ptr<Stream> file = FileStream::createInstance(path, Stream::READABLE);
  if (!file)
    return false;

  buffer.resize(file->getSize());
  if (!file->readItems(buffer.getData(), buffer.getSize()))
    return false;

  file = NULL;

  stream = FSOUND_Stream_Open((const char*) buffer.getData(), FSOUND_LOADMEMORY, 0, buffer.getSize());
  if (!stream)
  {
    Log::writeError("Unable to load music file as any known format");
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace FMOD*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
