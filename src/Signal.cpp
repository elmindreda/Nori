///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/Signal.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

////////////////////////////////////////////////////////////////////////

SignalSlot::SignalSlot(Trackable* initObject):
  object(initObject)
{
  if (object)
    object->slots.push_front(this);
}

SignalSlot::~SignalSlot(void)
{
  if (object)
    object->slots.remove(this);
}

////////////////////////////////////////////////////////////////////////

Trackable::Trackable(void)
{
}

Trackable::Trackable(const Trackable& source)
{
}

Trackable::~Trackable(void)
{
  while (!slots.empty())
    delete slots.front();
}

Trackable& Trackable::operator = (const Trackable& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
