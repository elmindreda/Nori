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
#include <wendy/Block.h>

#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLHelper.h>

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
    case GL_INVALID_FRAMEBUFFER_OPERATION:
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

GLenum convertToGL(IndexBuffer::Type type)
{
  switch (type)
  {
    case IndexBuffer::UINT8:
      return GL_UNSIGNED_BYTE;
    case IndexBuffer::UINT16:
      return GL_UNSIGNED_SHORT;
    case IndexBuffer::UINT32:
      return GL_UNSIGNED_INT;
  }

  panic("Invalid index buffer type %u", type);
}

GLenum convertToGL(VertexComponent::Type type)
{
  switch (type)
  {
    case VertexComponent::FLOAT32:
      return GL_FLOAT;
  }

  panic("Invalid vertex component type %u", type);
}

GLenum convertToGL(PixelFormat::Type type)
{
  switch (type)
  {
    case PixelFormat::UINT8:
      return GL_UNSIGNED_BYTE;
    case PixelFormat::UINT16:
      return GL_UNSIGNED_SHORT;
    case PixelFormat::UINT32:
      return GL_UNSIGNED_INT;

    case PixelFormat::FLOAT16:
    {
      if (!GLEW_ARB_texture_float || !GLEW_ARB_half_float_pixel)
      {
        logError("Half-precision floating point textures not supported; cannot convert pixel format type");
        return 0;
      }

      return GL_HALF_FLOAT_ARB;
    }

    case PixelFormat::FLOAT32:
    {
      if (!GLEW_ARB_texture_float)
      {
        logError("Floating point textures not supported; cannot convert pixel format type");
        return 0;
      }

      return GL_FLOAT;
    }

    default:
      logError("No OpenGL equivalent for pixel format type %u", type);
      return 0;
  }
}

GLenum convertToGL(PixelFormat::Semantic semantic)
{
  switch (semantic)
  {
    case PixelFormat::L:
      return GL_LUMINANCE;
    case PixelFormat::LA:
      return GL_LUMINANCE_ALPHA;
    case PixelFormat::RGB:
      return GL_RGB;
    case PixelFormat::RGBA:
      return GL_RGBA;
    case PixelFormat::DEPTH:
      return GL_DEPTH_COMPONENT;
    default:
      logError("No OpenGL equivalent for pixel format semantic %u", semantic);
      return 0;
  }
}

GLenum convertToGL(const PixelFormat& format, bool sRGB)
{
  switch (format.getType())
  {
    case PixelFormat::UINT8:
    {
      switch (format.getSemantic())
      {
        case PixelFormat::L:
        {
          if (sRGB)
            return GL_SLUMINANCE8;
          else
            return GL_LUMINANCE8;
        }

        case PixelFormat::LA:
        {
          if (sRGB)
            return GL_SLUMINANCE8_ALPHA8;
          else
            return GL_LUMINANCE8_ALPHA8;
        }

        case PixelFormat::RGB:
        {
          if (sRGB)
            return GL_SRGB8;
          else
            return GL_RGB8;
        }

        case PixelFormat::RGBA:
        {
          if (sRGB)
            return GL_SRGB8_ALPHA8;
          else
            return GL_RGBA8;
        }

        default:
          break;
      }

      break;
    }

    case PixelFormat::UINT16:
    {
      if (format.getSemantic() == PixelFormat::DEPTH)
        return GL_DEPTH_COMPONENT16;

      break;
    }

    case PixelFormat::UINT24:
    {
      if (format.getSemantic() == PixelFormat::DEPTH)
        return GL_DEPTH_COMPONENT24;

      break;
    }

    case PixelFormat::UINT32:
    {
      if (format.getSemantic() == PixelFormat::DEPTH)
        return GL_DEPTH_COMPONENT32;

      break;
    }

    case PixelFormat::FLOAT16:
    {
      if (!GLEW_ARB_texture_float || !GLEW_ARB_half_float_pixel)
      {
        logError("Half-precision floating point textures not supported; cannot convert pixel format");
        return 0;
      }

      switch (format.getSemantic())
      {
        case PixelFormat::L:
          return GL_LUMINANCE16F_ARB;
        case PixelFormat::LA:
          return GL_LUMINANCE_ALPHA16F_ARB;
        case PixelFormat::RGB:
          return GL_RGB16F_ARB;
        case PixelFormat::RGBA:
          return GL_RGBA16F_ARB;
        default:
          break;
      }

      break;
    }

    case PixelFormat::FLOAT32:
    {
      if (!GLEW_ARB_texture_float)
      {
        logError("Floating point textures not supported; cannot convert pixel format");
        return 0;
      }

      switch (format.getSemantic())
      {
        case PixelFormat::L:
          return GL_LUMINANCE32F_ARB;
        case PixelFormat::LA:
          return GL_LUMINANCE_ALPHA32F_ARB;
        case PixelFormat::RGB:
          return GL_RGB32F_ARB;
        case PixelFormat::RGBA:
          return GL_RGBA32F_ARB;
        default:
          break;
      }

      break;
    }

    default:
      break;
  }

  logError("No OpenGL equivalent for pixel format \'%s\'",
           format.asString().c_str());
  return 0;
}

GLenum convertToGL(TextureType type)
{
  switch (type)
  {
    case TEXTURE_1D:
      return GL_TEXTURE_1D;
    case TEXTURE_2D:
      return GL_TEXTURE_2D;
    case TEXTURE_3D:
      return GL_TEXTURE_3D;
    case TEXTURE_RECT:
      return GL_TEXTURE_RECTANGLE;
    case TEXTURE_CUBE:
      return GL_TEXTURE_CUBE_MAP;
  }

  panic("No OpenGL equivalent for texture type %u", type);
}

GLboolean getBoolean(GLenum token)
{
  GLboolean value;
  glGetBooleanv(token, &value);
  return value;
}

GLint getInteger(GLenum token)
{
  GLint value;
  glGetIntegerv(token, &value);
  return value;
}

GLfloat getFloat(GLenum token)
{
  GLfloat value;
  glGetFloatv(token, &value);
  return value;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
