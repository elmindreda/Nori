///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2010 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/OpenGL.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <cstdlib>
#include <cstdio>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

namespace
{

const char* getErrorString(unsigned int error)
{
  switch (error)
  {
    case GL_NO_ERROR:
      return "no error";
    case GL_INVALID_ENUM:
      return "invalid enum";
    case GL_INVALID_VALUE:
      return "invalid value";
    case GL_INVALID_OPERATION:
      return "invalid operation";
    case GL_STACK_OVERFLOW:
      return "stack overflow";
    case GL_STACK_UNDERFLOW:
      return "stack underflow";
    case GL_OUT_OF_MEMORY:
      return "out of memory";
    case GL_INVALID_FRAMEBUFFER_OPERATION_EXT:
      return "invalid framebuffer operation";
  }

  logError("Unknown OpenGL error %u", error);
  return "UNKNOWN ERROR";
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

bool checkGL(const char* format, ...)
{
  GLenum error = glGetError();
  if (error == GL_NO_ERROR)
    return true;

  va_list vl;
  char* message;
  int result;

  va_start(vl, format);
  result = vasprintf(&message, format, vl);
  va_end(vl);

  if (result < 0)
  {
    logError("Error formatting error message for OpenGL error %u", error);
    return false;
  }

  logError("%s: %s", message, getErrorString(error));

  std::free(message);
  return false;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
