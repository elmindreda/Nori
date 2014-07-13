///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
// Copyright (c) 2012 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <nori/Config.hpp>

#include <nori/Texture.hpp>
#include <nori/RenderBuffer.hpp>
#include <nori/Program.hpp>
#include <nori/RenderContext.hpp>

#include <GREG/greg.h>

#include <internal/OpenGL.hpp>

#include <algorithm>
#include <fstream>

#include <cstring>

namespace nori
{

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

bool checkGL(const char* format, ...)
{
  GLenum error = glGetError();
  if (error == GL_NO_ERROR)
    return true;

  va_list vl;

  va_start(vl, format);
  std::string message = vlformat(format, vl);
  va_end(vl);

  logError("%s: %s", message.c_str(), getErrorString(error));
  return false;
}

GLenum convertToGL(BufferType type)
{
  switch (type)
  {
    case VERTEX_BUFFER:
      return GL_ARRAY_BUFFER;
    case INDEX_BUFFER:
      return GL_ELEMENT_ARRAY_BUFFER;
  }

  panic("Invalid buffer type %u", type);
}

GLenum convertToGL(IndexType type)
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
           stringCast(format).c_str());
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

Preprocessor::Preprocessor(ResourceCache& cache):
  m_cache(cache)
{
}

void Preprocessor::parse(const char* name)
{
  const Path path = m_cache.findFile(name);
  if (path.isEmpty())
  {
    if (m_files.empty())
      logError("Failed to find shader %s", name);
    else
    {
      logError("%s:%u: Failed to find shader %s",
               m_files.back().name,
               m_files.back().line,
               name);
    }

    throw Exception("Failed to find shader file");
  }

  std::ifstream stream(path.name());
  if (stream.fail())
  {
    if (m_files.empty())
      logError("Failed to open shader file %s", path.name().c_str());
    else
    {
      logError("%s:%u: Failed to open shader file %s",
               m_files.back().name,
               m_files.back().line,
               path.name().c_str());
    }

    throw Exception("Failed to open shader file");
  }

  m_paths.push_back(path);

  std::string text;

  stream.seekg(0, std::ios::end);
  text.resize((size_t) stream.tellg());
  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());
  stream.close();

  parse(name, text.c_str());
}

void Preprocessor::parse(const char* name, const char* text)
{
  if (std::find(m_names.begin(), m_names.end(), name) != m_names.end())
    return;

  m_files.push_back(File(name, text));
  m_names.push_back(name);

  m_list += format("( file %u: %s )\n", (uint) m_names.size(), name);

  m_output.reserve(m_output.size() + std::strlen(text));
  appendToOutput(format("#line 0 %u /* entering %s */\n",
                        (uint) m_files.size(),
                        m_files.back().name).c_str());

  while (hasMore())
  {
    if (isMultiLineComment())
      parseMultiLineComment();
    else if (isSingleLineComment())
      parseSingleLineComment();
    else if (isNewLine())
      parseNewLine();
    else if (isWhitespace())
      parseWhitespace();
    else if (isCommand())
      parseCommand();
    else
    {
      advance(1);
      appendToOutput();
      setFirstOnLine(false);
    }
  }

  m_files.pop_back();

  if (!m_files.empty())
  {
    appendToOutput(format("\n#line %u %u /* returning to %s */",
                          m_files.back().line,
                          (uint) m_files.size(),
                          m_files.back().name).c_str());
  }
}

bool Preprocessor::hasVersion() const
{
  return !m_version.empty();
}

void Preprocessor::addLine()
{
  m_files.back().line++;
}

void Preprocessor::advance(size_t count)
{
  m_files.back().pos += count;
}

void Preprocessor::discard()
{
  m_files.back().base = m_files.back().pos;
}

void Preprocessor::appendToOutput()
{
  File& file = m_files.back();
  m_output.append(file.base, file.pos);
  file.base = file.pos;
}

void Preprocessor::appendToOutput(const char* text)
{
  m_output.append(text);
}

char Preprocessor::c(ptrdiff_t offset) const
{
  return m_files.back().pos[offset];
}

void Preprocessor::passWhitespace()
{
  while (isWhitespace())
    advance(1);
}

void Preprocessor::parseWhitespace()
{
  passWhitespace();
  appendToOutput();
}

void Preprocessor::parseNewLine()
{
  if (c(0) == '\r' && c(1) == '\n')
    advance(2);
  else
    advance(1);

  addLine();
  setFirstOnLine(true);
  appendToOutput();
}

void Preprocessor::parseSingleLineComment()
{
  advance(2);
  setFirstOnLine(false);

  while (hasMore())
  {
    if (isNewLine())
      break;
    else
      advance(1);
  }

  appendToOutput();
}

void Preprocessor::parseMultiLineComment()
{
  advance(2);
  setFirstOnLine(false);

  for (;;)
  {
    if (!hasMore())
    {
      logError("%s:%u: Unexpected end of file in multi-line comment",
               m_files.back().name,
               m_files.back().line);

      throw Exception("Unexpected end of file in multi-line comment");
    }

    if (c(0) == '*' && c(1) == '/')
    {
      advance(2);
      break;
    }
    else if (isNewLine())
      parseNewLine();
    else
      advance(1);
  }

  appendToOutput();
}

std::string Preprocessor::passNumber()
{
  if (!isNumeric())
  {
    logError("%s:%u: Expected number", m_files.back().name, m_files.back().line);

    throw Exception("Expected number");
  }

  std::string number;

  while (isNumeric())
  {
    number.append(1, c(0));
    advance(1);
  }

  return number;
}

std::string Preprocessor::passIdentifier()
{
  if (!isAlpha())
  {
    logError("%s:%u: Expected identifier",
             m_files.back().name,
             m_files.back().line);

    throw Exception("Expected identifier");
  }

  std::string identifier;

  while (isAlphaNumeric())
  {
    identifier.append(1, c(0));
    advance(1);
  }

  return identifier;
}

std::string Preprocessor::passShaderName()
{
  char terminator;
  if (c(0) == '<')
    terminator = '>';
  else if (c(0) == '\"')
    terminator = '\"';
  else
  {
    logError("%s:%u: Expected < or \" after #include",
             m_files.back().name,
             m_files.back().line);

    throw Exception("Expected < or \" after #include");
  }

  advance(1);

  std::string name;

  while (hasMore())
  {
    if (!hasMore() || isNewLine())
    {
      logError("%s:%u: Expected %c after shader name",
               m_files.back().name,
               m_files.back().line,
               terminator);

      throw Exception("Expected < or \" after shader name");
    }

    if (c(0) == terminator)
    {
      advance(1);
      break;
    }
    else
    {
      name.append(1, c(0));
      advance(1);
    }
  }

  return name;
}

void Preprocessor::parseCommand()
{
  advance(1);
  setFirstOnLine(false);
  passWhitespace();

  const std::string command = passIdentifier();
  if (command == "include")
  {
    passWhitespace();
    const std::string name = passShaderName();
    discard();
    parse(name.c_str());
  }
  else if (command == "version")
  {
    if (!m_version.empty())
    {
      logError("%s:%u: Duplicate #version directive",
               m_files.back().name,
               m_files.back().line);

      throw Exception("Duplicate #version directive");
    }

    passWhitespace();
    m_version = passNumber();
    discard();
  }

  while (hasMore())
  {
    if (isNewLine() || isSingleLineComment() || isMultiLineComment())
      break;

    advance(1);
  }

  appendToOutput();
}

bool Preprocessor::hasMore() const
{
  return c(0) != '\0';
}

bool Preprocessor::isNewLine() const
{
  return c(0) == '\r' || c(0) == '\n';
}

bool Preprocessor::isMultiLineComment() const
{
  return c(0) == '/' && c(1) == '*';
}

bool Preprocessor::isSingleLineComment() const
{
  return c(0) == '/' && c(1) == '/';
}

bool Preprocessor::isWhitespace() const
{
  return c(0) == ' ' || c(0) == '\t';
}

bool Preprocessor::isCommand() const
{
  return isFirstOnLine() && c(0) == '#';
}

bool Preprocessor::isAlpha() const
{
  return (c(0) >= 'a' && c(0) <= 'z') || (c(0) >= 'A' && c(0) <= 'Z');
}

bool Preprocessor::isNumeric() const
{
  return c(0) >= '0' && c(0) <= '9';
}

bool Preprocessor::isAlphaNumeric() const
{
  return isAlpha() || isNumeric();
}

bool Preprocessor::isFirstOnLine() const
{
  return m_files.back().first;
}

void Preprocessor::setFirstOnLine(bool newState)
{
  m_files.back().first = newState;
}

Preprocessor::File::File(const char* name, const char* text):
  name(name),
  text(text),
  base(text),
  pos(text),
  line(1),
  first(true)
{
}

} /*namespace nori*/

