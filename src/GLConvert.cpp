///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2009 Camilla Berglund <elmindreda@elmindreda.org>
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

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLConvert.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Bimap<PixelFormat, GLenum> formatMap;
Bimap<PixelFormat, GLenum> genericFormatMap;

GLenum convertToGL(LockType type)
{
  switch (type)
  {
    case LOCK_READ_ONLY:
      return GL_READ_ONLY_ARB;
    case LOCK_WRITE_ONLY:
      return GL_WRITE_ONLY_ARB;
    case LOCK_READ_WRITE:
      return GL_READ_WRITE_ARB;
    default:
      throw Exception("Invalid buffer lock type");
  }
}

GLenum convertToGL(VertexBuffer::Usage usage)
{
  switch (usage)
  {
    case VertexBuffer::STATIC:
      return GL_STATIC_DRAW_ARB;
    case VertexBuffer::STREAM:
      return GL_STREAM_DRAW_ARB;
    case VertexBuffer::DYNAMIC:
      return GL_DYNAMIC_DRAW_ARB;
    default:
      throw Exception("Invalid vertex buffer usage");
  }
}

GLenum convertToGL(IndexBuffer::Usage usage)
{
  switch (usage)
  {
    case IndexBuffer::STATIC:
      return GL_STATIC_DRAW_ARB;
    case IndexBuffer::STREAM:
      return GL_STREAM_DRAW_ARB;
    case IndexBuffer::DYNAMIC:
      return GL_DYNAMIC_DRAW_ARB;
    default:
      throw Exception("Invalid index buffer usage");
  }
}

GLenum convertToGL(PrimitiveType type)
{
  switch (type)
  {
    case POINT_LIST:
      return GL_POINTS;
    case LINE_LIST:
      return GL_LINES;
    case LINE_STRIP:
      return GL_LINE_STRIP;
    case LINE_LOOP:
      return GL_LINE_LOOP;
    case TRIANGLE_LIST:
      return GL_TRIANGLES;
    case TRIANGLE_STRIP:
      return GL_TRIANGLE_STRIP;
    case TRIANGLE_FAN:
      return GL_TRIANGLE_FAN;
    default:
      throw Exception("Invalid primitive type");
  }
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
    default:
      throw Exception("Invalid vertex component type");
  }
}

GLenum convertToGL(IndexBuffer::Type type)
{
  switch (type)
  {
    case IndexBuffer::UINT:
      return GL_UNSIGNED_INT;
    case IndexBuffer::USHORT:
      return GL_UNSIGNED_SHORT;
    case IndexBuffer::UBYTE:
      return GL_UNSIGNED_BYTE;
    default:
      throw Exception("Invalid index buffer type");
  }
}

GLenum convertToGL(const PixelFormat& format)
{
  if (formatMap.isEmpty())
  {
    formatMap[PixelFormat::R8] = GL_ALPHA8;
    formatMap[PixelFormat::RG8] = GL_LUMINANCE8_ALPHA8;
    formatMap[PixelFormat::RGB8] = GL_RGB8;
    formatMap[PixelFormat::RGBA8] = GL_RGBA8;
    formatMap[PixelFormat::DEPTH16] = GL_DEPTH_COMPONENT16_ARB;
    formatMap[PixelFormat::DEPTH24] = GL_DEPTH_COMPONENT24_ARB;
    formatMap[PixelFormat::DEPTH32] = GL_DEPTH_COMPONENT32_ARB;
    formatMap.setDefaults(PixelFormat(), 0);
  }

  return formatMap[format];
}

GLenum convertToGenericGL(const PixelFormat& format)
{
  if (genericFormatMap.isEmpty())
  {
    genericFormatMap[PixelFormat::R8] = GL_ALPHA;
    genericFormatMap[PixelFormat::RG8] = GL_LUMINANCE_ALPHA;
    genericFormatMap[PixelFormat::RGB8] = GL_RGB;
    genericFormatMap[PixelFormat::RGBA8] = GL_RGBA;
    genericFormatMap[PixelFormat::DEPTH16] = GL_DEPTH_COMPONENT;
    genericFormatMap[PixelFormat::DEPTH24] = GL_DEPTH_COMPONENT;
    genericFormatMap[PixelFormat::DEPTH32] = GL_DEPTH_COMPONENT;
    genericFormatMap.setDefaults(PixelFormat(), 0);
  }

  return genericFormatMap[format];
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
