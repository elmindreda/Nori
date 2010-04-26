///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/Portability.h>
#include <wendy/Core.h>

#if WENDY_HAVE_STDARG_H
#include <stdarg.h>
#endif

#include <stdio.h>

#include <cstdlib>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Exception::Exception(const char* initMessage):
  message(initMessage)
{
}

const char* Exception::what(void) const throw()
{
  return message;
}

///////////////////////////////////////////////////////////////////////

bool RefBase::unreferenced(RefObject* object)
{
  return object->count == 0;
}

void RefBase::increment(RefObject* object)
{
  object->count++;
}

void RefBase::decrement(RefObject* object)
{
  object->count--;
}

///////////////////////////////////////////////////////////////////////

RefObject::RefObject(void):
  count(0)
{
}

RefObject::RefObject(const RefObject& source):
  count(0)
{
}

RefObject::~RefObject(void)
{
}

RefObject& RefObject::operator = (const RefObject& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

Log::~Log(void)
{
}

void Log::writeError(const char* format, ...)
{
  va_list vl;
  char* message;
  int result;

  va_start(vl, format);
  result = vasprintf(&message, format, vl);
  va_end(vl);

  if (result < 0)
    return;

  if (Log* log = Log::get())
    log->write(ERROR, "Error: %s", message);
  else
    fprintf(stderr, "Error: %s\n", message);

  std::free(message);
}

void Log::writeWarning(const char* format, ...)
{
  va_list vl;
  char* message;
  int result;

  va_start(vl, format);
  result = vasprintf(&message, format, vl);
  va_end(vl);

  if (result < 0)
    return;

  if (Log* log = Log::get())
    log->write(WARNING, "Warning: %s", message);
  else
    fprintf(stderr, "Warning: %s\n", message);

  std::free(message);
}

void Log::write(const char* format, ...)
{
  va_list vl;
  char* message;
  int result;

  va_start(vl, format);
  result = vasprintf(&message, format, vl);
  va_end(vl);

  if (result < 0)
    return;

  if (Log* log = Log::get())
    log->write(INFORMATION, "%s", message);
  else
    fprintf(stderr, "%s\n", message);

  std::free(message);
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
