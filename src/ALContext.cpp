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

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace AL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Context::~Context(void)
{
  alutExit();
}

const Vector3& Context::getListenerPosition(void) const
{
  return listenerPosition;
}

void Context::setListenerPosition(const Vector3& newPosition)
{
  if (listenerPosition != newPosition)
  {
    alListenerfv(AL_POSITION, listenerPosition);
    listenerPosition = newPosition;
  }
}

const Vector3& Context::getListenerVelocity(void) const
{
  return listenerVelocity;
}

void Context::setListenerVelocity(const Vector3& newVelocity)
{
  if (listenerVelocity != newVelocity)
  {
    alListenerfv(AL_VELOCITY, listenerVelocity);
    listenerVelocity = newVelocity;
  }
}

bool Context::create(void)
{
  if (get())
    return true;

  Ptr<Context> context = new Context();
  if (!context->init())
    return false;

  set(context.detachObject());
  return true;
}

Context::Context(void):
  listenerPosition(Vector3::ZERO),
  listenerVelocity(Vector3::ZERO)
{
}

bool Context::init(void)
{
  if (!alutInit(NULL, NULL))
  {
    Log::writeError("Unable to initialize OpenAL: %s",
                    alutGetErrorString(alutGetError()));
    return false;
  }

  alListenerfv(AL_POSITION, listenerPosition);
  alListenerfv(AL_VELOCITY, listenerVelocity);

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
