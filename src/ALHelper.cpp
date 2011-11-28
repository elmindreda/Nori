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

#include <internal/ALHelper.h>

#include <al.h>

#include <cstdio>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace AL
  {

///////////////////////////////////////////////////////////////////////

namespace
{

const char* getErrorString(ALenum error)
{
  switch (error)
  {
    case AL_INVALID_NAME:
      return "Invalid name parameter";
    case AL_INVALID_ENUM:
      return "Invalid enum parameter";
    case AL_INVALID_VALUE:
      return "Invalid enum parameter value";
    case AL_INVALID_OPERATION:
      return "Invalid operation";
    case AL_OUT_OF_MEMORY:
      return "Out of memory";
  }

  return "Unknown OpenAL error";
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

bool checkAL(const char* format, ...)
{
  ALenum error = alGetError();
  if (error == AL_NO_ERROR)
    return true;

  va_list vl;
  char* message;
  int result;

  va_start(vl, format);
  result = vasprintf(&message, format, vl);
  va_end(vl);

  if (result < 0)
  {
    logError("Error formatting error message for OpenAL error %u", error);
    return false;
  }

  logError("%s: %s", message, getErrorString(error));

  std::free(message);
  return false;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace AL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
