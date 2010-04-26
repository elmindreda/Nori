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

// NOTE: This is the correct place to put replacements implementations
//       of any missing system and library calls.

#include <wendy/Config.h>
#include <wendy/Portability.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if WENDY_HAVE_STRING_H
#include <string.h>
#endif

#if WENDY_HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef _MSC_VER

float log2f(float x)
{
  return (float) log(x) / (float) log(2.f);
}

float fminf(float x, float y)
{
  if (x < y)
    return x;
  else
    return y;
}

float fmaxf(float x, float y)
{
  if (x > y)
    return x;
  else
    return y;
}

#endif /*_MSC_VER*/

#if !WENDY_HAVE_VASPRINTF

int vasprintf(char** result, const char* format, va_list vl)
{
  char buffer[65536];

  if (vsnprintf(buffer, sizeof(buffer), format, vl) < 0)
    buffer[sizeof(buffer) - 1] = '\0';

  size_t length = ::strlen(buffer);
  *result = (char*) ::malloc(length + 1);
  strcpy(*result, buffer);

  return (int) length;
}

#endif /*WENDY_HAVE_VASPRINTF*/

#if !WENDY_HAVE_STRTOF

float strtof(const char* nptr, char** endptr)
{
  return (float) strtod(nptr, endptr);
}

#endif /*WENDY_HAVE_STRTOF*/

#if !WENDY_HAVE_STRLCAT

size_t strlcat(char* target, const char* source, size_t size)
{
  size_t target_length, source_length, append_length;

  target_length = strlen(target);
  source_length = strlen(source);

  if (target_length >= size)
    return size;

  if (source_length == 0)
    return target_length;

  append_length = size - (target_length + 1);
  if (append_length > source_length)
    append_length = source_length;

  if (append_length > 0)
    strncat(target, source, append_length);

  return target_length + source_length;
}

#endif /*WENDY_HAVE_STRLCAT*/

#if !WENDY_HAVE_STRLCPY

size_t strlcpy(char* target, const char* source, size_t size)
{
  size_t source_length, copy_length;

  source_length = strlen(source);

  if (size == 0)
    return source_length;

  if (source_length < size - 1)
    copy_length = source_length;
  else
    copy_length = size - 1;

  strncpy(target, source, copy_length);

  if (copy_length >= size)
    target[size - 1] = '\0';

  return source_length;
}

#endif /*WENDY_HAVE_STRLCPY*/

///////////////////////////////////////////////////////////////////////
