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

#include <wendy/OpenGL.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLConvert.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

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

  Log::writeError("Invalid index buffer type %u", type);
  return 0;
}

GLenum convertToGL(VertexComponent::Type type)
{
  switch (type)
  {
    case VertexComponent::DOUBLE:
      return GL_DOUBLE;
    case VertexComponent::FLOAT:
      return GL_FLOAT;
    case VertexComponent::INT:
      return GL_INT;
  }

  Log::writeError("Invalid vertex component type %u", type);
  return 0;
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
  }

  Log::writeError("No OpenGL equivalent for pixel format type %u", type);
  return 0;
}

GLenum convertToGL(const PixelFormat& format)
{
  switch (format.getType())
  {
    case PixelFormat::UINT8:
    {
      switch (format.getSemantic())
      {
        case PixelFormat::R:
          return GL_LUMINANCE8;
        case PixelFormat::RG:
          return GL_LUMINANCE8_ALPHA8;
        case PixelFormat::RGBA:
          return GL_RGBA8;
      }

      break;
    }

    case PixelFormat::UINT16:
    {
      if (format.getSemantic() == PixelFormat::DEPTH)
        return GL_DEPTH_COMPONENT16_ARB;

      break;
    }

    case PixelFormat::UINT24:
    {
      if (format.getSemantic() == PixelFormat::DEPTH)
        return GL_DEPTH_COMPONENT24_ARB;

      break;
    }

    case PixelFormat::UINT32:
    {
      if (format.getSemantic() == PixelFormat::DEPTH)
        return GL_DEPTH_COMPONENT32_ARB;

      break;
    }
  }

  Log::writeError("No OpenGL equivalent for pixel format \'%s\'",
                  format.asString().c_str());
  return 0;
}

GLenum convertToGenericGL(const PixelFormat& format)
{
  switch (format.getType())
  {
    case PixelFormat::UINT8:
    {
      switch (format.getSemantic())
      {
        case PixelFormat::R:
          return GL_LUMINANCE;
        case PixelFormat::RG:
          return GL_LUMINANCE_ALPHA;
        case PixelFormat::RGBA:
          return GL_RGBA;
      }

      break;
    }

    case PixelFormat::UINT16:
    {
      if (format.getSemantic() == PixelFormat::DEPTH)
        return GL_DEPTH_COMPONENT;

      break;
    }

    case PixelFormat::UINT24:
    {
      if (format.getSemantic() == PixelFormat::DEPTH)
        return GL_DEPTH_COMPONENT;

      break;
    }

    case PixelFormat::UINT32:
    {
      if (format.getSemantic() == PixelFormat::DEPTH)
        return GL_DEPTH_COMPONENT;

      break;
    }
  }

  Log::writeError("No OpenGL equivalent for pixel format \'%s\'",
                  format.asString().c_str());
  return 0;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
