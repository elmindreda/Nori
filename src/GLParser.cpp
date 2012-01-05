///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Path.h>
#include <wendy/Resource.h>

#include <internal/GLParser.h>

#include <algorithm>
#include <fstream>

#include <cstring>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

ShaderPreprocessor::ShaderPreprocessor(ResourceCache& initCache):
  cache(initCache)
{
}

void ShaderPreprocessor::parse(const char* name)
{
  const Path path = cache.findFile(name);
  if (path.isEmpty())
  {
    if (files.empty())
      logError("Failed to find shader \'%s\'", name);
    else
    {
      const File& file = files.back();
      logError("%s:%u: Failed to find shader \'%s\'",
               file.name,
               file.line,
               name);
    }

    throw Exception("Failed to find shader file");
  }

  std::ifstream stream(path.asString().c_str());
  if (stream.fail())
  {
    if (files.empty())
      logError("Failed to open shader file \'%s\'", path.asString().c_str());
    else
    {
      const File& file = files.back();
      logError("%s:%u: Failed to open shader file \'%s\'",
               file.name,
               file.line,
               path.asString().c_str());
    }

    throw Exception("Failed to open shader file");
  }

  String text;

  stream.seekg(0, std::ios::end);
  text.resize((size_t) stream.tellg());
  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());
  stream.close();

  parse(name, text.c_str());
}

void ShaderPreprocessor::parse(const char* name, const char* text)
{
  if (std::find(names.begin(), names.end(), name) != names.end())
    return;

  files.push_back(File(name, text));
  names.push_back(name);

  output.reserve(output.size() + std::strlen(text));
  appendToOutput(format("#line 0 %u /* entering %s */\n",
                        (unsigned int) files.size(),
                        files.back().name).c_str());

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

  files.pop_back();

  if (!files.empty())
  {
    appendToOutput(format("\n#line %u %u /* returning to %s */",
                          files.back().line,
                          (unsigned int) files.size(),
                          files.back().name).c_str());
  }
}

const String& ShaderPreprocessor::getOutput() const
{
  return output;
}

bool ShaderPreprocessor::hasVersion() const
{
  return !version.empty();
}

const String& ShaderPreprocessor::getVersion() const
{
  return version;
}

const ShaderNameList& ShaderPreprocessor::getNameList() const
{
  return names;
}

void ShaderPreprocessor::addLine()
{
  files.back().line++;
}

void ShaderPreprocessor::advance(size_t count)
{
  files.back().pos += count;
}

void ShaderPreprocessor::discard()
{
  files.back().base = files.back().pos;
}

void ShaderPreprocessor::appendToOutput()
{
  File& file = files.back();
  output.append(file.base, file.pos);
  file.base = file.pos;
}

void ShaderPreprocessor::appendToOutput(const char* text)
{
  output.append(text);
}

char ShaderPreprocessor::c(ptrdiff_t offset) const
{
  return files.back().pos[offset];
}

void ShaderPreprocessor::passWhitespace()
{
  while (isWhitespace())
    advance(1);
}

void ShaderPreprocessor::parseWhitespace()
{
  passWhitespace();
  appendToOutput();
}

void ShaderPreprocessor::parseNewLine()
{
  if (c(0) == '\r' && c(1) == '\n')
    advance(2);
  else
    advance(1);

  addLine();
  setFirstOnLine(true);
  appendToOutput();
}

void ShaderPreprocessor::parseSingleLineComment()
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

void ShaderPreprocessor::parseMultiLineComment()
{
  advance(2);
  setFirstOnLine(false);

  for (;;)
  {
    if (!hasMore())
    {
      const File& file = files.back();
      logError("%s:%u: Unexpected end of file in multi-line comment",
               file.name,
               file.line);

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

String ShaderPreprocessor::passNumber()
{
  if (!isNumeric())
  {
    const File& file = files.back();
    logError("%s:%u: Expected number", file.name, file.line);

    throw Exception("Expected number");
  }

  String number;

  while (isNumeric())
  {
    number.append(1, c(0));
    advance(1);
  }

  return number;
}

String ShaderPreprocessor::passIdentifier()
{
  if (!isAlpha())
  {
    const File& file = files.back();
    logError("%s:%u: Expected identifier", file.name, file.line);

    throw Exception("Expected identifier");
  }

  String identifier;

  while (isAlphaNumeric())
  {
    identifier.append(1, c(0));
    advance(1);
  }

  return identifier;
}

String ShaderPreprocessor::passShaderName()
{
  char terminator;
  if (c(0) == '<')
    terminator = '>';
  else if (c(0) == '\"')
    terminator = '\"';
  else
  {
    const File& file = files.back();
    logError("%s:%u: Expected \'<\' or \'\"\' after #include",
             file.name,
             file.line);

    throw Exception("Expected \'<\' or \'\"\' after #include");
  }

  advance(1);

  String name;

  while (hasMore())
  {
    if (!hasMore() || isNewLine())
    {
      const File& file = files.back();
      logError("%s:%u: Expected \'%c\' after shader name",
               file.name,
               file.line,
               terminator);

      throw Exception("Expected \'<\' or \'\"\' after shader name");
    }

    if (c(0) == terminator)
    {
      advance(1);
      return name;
    }
    else
    {
      name.append(1, c(0));
      advance(1);
    }
  }
}

void ShaderPreprocessor::parseCommand()
{
  advance(1);
  setFirstOnLine(false);
  passWhitespace();

  const String command = passIdentifier();
  if (command == "include")
  {
    passWhitespace();
    const String name = passShaderName();
    discard();
    parse(name.c_str());
  }
  else if (command == "version")
  {
    if (!version.empty())
    {
      logError("%s:%u: Duplicate #version directive",
               files.back().name,
               files.back().line);

      throw Exception("Duplicate #version directive");
    }

    passWhitespace();
    version = passNumber();
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

bool ShaderPreprocessor::hasMore() const
{
  return c(0) != '\0';
}

bool ShaderPreprocessor::isNewLine() const
{
  return c(0) == '\r' || c(0) == '\n';
}

bool ShaderPreprocessor::isMultiLineComment() const
{
  return c(0) == '/' && c(1) == '*';
}

bool ShaderPreprocessor::isSingleLineComment() const
{
  return c(0) == '/' && c(1) == '/';
}

bool ShaderPreprocessor::isWhitespace() const
{
  return c(0) == ' ' || c(0) == '\t';
}

bool ShaderPreprocessor::isCommand() const
{
  return isFirstOnLine() && c(0) == '#';
}

bool ShaderPreprocessor::isAlpha() const
{
  return c(0) >= 'a' && c(0) <= 'z' || c(0) >= 'A' && c(0) <= 'Z';
}

bool ShaderPreprocessor::isNumeric() const
{
  return c(0) >= '0' && c(0) <= '9';
}

bool ShaderPreprocessor::isAlphaNumeric() const
{
  return isAlpha() || isNumeric();
}

bool ShaderPreprocessor::isFirstOnLine() const
{
  return files.back().first;
}

void ShaderPreprocessor::setFirstOnLine(bool newState)
{
  files.back().first = newState;
}

///////////////////////////////////////////////////////////////////////

ShaderPreprocessor::File::File(const char* name, const char* text):
  name(name),
  text(text),
  base(text),
  pos(text),
  line(1),
  first(true)
{
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
