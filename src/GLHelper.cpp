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

#include <wendy/Config.hpp>

#include <wendy/Texture.hpp>
#include <wendy/RenderBuffer.hpp>
#include <wendy/Program.hpp>
#include <wendy/RenderContext.hpp>

#include <GREG/greg.h>

#include <internal/GLHelper.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

const char* getErrorString(uint error)
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

  va_start(vl, format);
  String message = vlformat(format, vl);
  va_end(vl);

  logError("%s: %s", message.c_str(), getErrorString(error));
  return false;
}

GLenum convertToGL(IndexBufferType type)
{
  switch (type)
  {
    case INDEX_UINT8:
      return GL_UNSIGNED_BYTE;
    case INDEX_UINT16:
      return GL_UNSIGNED_SHORT;
    case INDEX_UINT32:
      return GL_UNSIGNED_INT;
  }

  panic("Invalid index buffer type %u", type);
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
      return GL_HALF_FLOAT;
    case PixelFormat::FLOAT32:
      return GL_FLOAT;

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
  switch (format.type())
  {
    case PixelFormat::UINT8:
    {
      switch (format.semantic())
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
      if (format.semantic() == PixelFormat::DEPTH)
        return GL_DEPTH_COMPONENT16;

      break;
    }

    case PixelFormat::UINT24:
    {
      if (format.semantic() == PixelFormat::DEPTH)
        return GL_DEPTH_COMPONENT24;

      break;
    }

    case PixelFormat::UINT32:
    {
      if (format.semantic() == PixelFormat::DEPTH)
        return GL_DEPTH_COMPONENT32;

      break;
    }

    case PixelFormat::FLOAT16:
    {
      switch (format.semantic())
      {
        case PixelFormat::L:
        {
          if (!GREG_ARB_texture_float)
          {
            logError("Half-precision floating point textures not supported; "
                     "cannot convert pixel format");
            return 0;
          }

          return GL_LUMINANCE16F_ARB;
        }

        case PixelFormat::LA:
        {
          if (!GREG_ARB_texture_float)
          {
            logError("Half-precision floating point textures not supported; "
                     "cannot convert pixel format");
            return 0;
          }

          return GL_LUMINANCE_ALPHA16F_ARB;
        }

        case PixelFormat::RGB:
          return GL_RGB16F;
        case PixelFormat::RGBA:
          return GL_RGBA16F;
        default:
          break;
      }

      break;
    }

    case PixelFormat::FLOAT32:
    {
      switch (format.semantic())
      {
        case PixelFormat::L:
        {
          if (!GREG_ARB_texture_float)
          {
            logError("Floating point textures not supported; cannot convert pixel format");
            return 0;
          }

          return GL_LUMINANCE32F_ARB;
        }

        case PixelFormat::LA:
        {
          if (!GREG_ARB_texture_float)
          {
            logError("Floating point textures not supported; cannot convert pixel format");
            return 0;
          }

          return GL_LUMINANCE_ALPHA32F_ARB;
        }

        case PixelFormat::RGB:
          return GL_RGB32F;
        case PixelFormat::RGBA:
          return GL_RGBA32F;
        default:
          break;
      }

      break;
    }

    default:
      break;
  }

  logError("No OpenGL equivalent for pixel format %s",
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

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
