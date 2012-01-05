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

Parser::Parser(ResourceCache& initCache):
  cache(initCache)
{
}

void Parser::parse(const char* name)
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

void Parser::parse(const char* name, const char* text)
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

const String& Parser::getOutput() const
{
  return output;
}

const Parser::NameList& Parser::getNameList() const
{
  return names;
}

void Parser::addLine()
{
  File& file = files.back();
  file.line++;
}

void Parser::advance(size_t count)
{
  File& file = files.back();
  file.pos += count;
}

void Parser::discard()
{
  File& file = files.back();
  file.base = file.pos;
}

void Parser::appendToOutput()
{
  File& file = files.back();
  output.append(file.base, file.pos);
  file.base = file.pos;
}

void Parser::appendToOutput(const char* text)
{
  output.append(text);
}

char Parser::c(ptrdiff_t offset) const
{
  const File& file = files.back();
  return file.pos[offset];
}

void Parser::passWhitespace()
{
  while (isWhitespace())
    advance(1);
}

void Parser::parseWhitespace()
{
  passWhitespace();
  appendToOutput();
}

void Parser::parseNewLine()
{
  if (c(0) == '\r' && c(1) == '\n')
    advance(2);
  else
    advance(1);

  addLine();
  setFirstOnLine(true);
  appendToOutput();
}

void Parser::parseSingleLineComment()
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

void Parser::parseMultiLineComment()
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

String Parser::passNumber()
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

String Parser::passIdentifier()
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

String Parser::passFileName()
{
  char terminator;
  if (c(0) == '<')
    terminator = '>';
  else if (c(0) == '\"')
    terminator = '\"';
  else
  {
    const File& file = files.back();
    logError("%s:%u: Expected \'<\' or \'\"\' after \'#include\'",
             file.name,
             file.line);

    throw Exception("Expected \'<\' or \'\"\' after \'#include\'");
  }

  advance(1);

  String name;

  while (hasMore())
  {
    if (!hasMore() || isNewLine())
    {
      const File& file = files.back();
      logError("%s:%u: Expected \'%c\' after filename",
               file.name,
               file.line,
               terminator);

      throw Exception("Expected \'<\' or \'\"\' after filename");
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

void Parser::parseCommand()
{
  advance(1);
  setFirstOnLine(false);

  passWhitespace();
  const String command = passIdentifier();
  passWhitespace();

  if (command == "include")
  {
    const String name = passFileName();
    discard();
    parse(name.c_str());
  }

  while (hasMore())
  {
    if (isNewLine() || isSingleLineComment() || isMultiLineComment())
      break;

    advance(1);
  }

  appendToOutput();
}

bool Parser::hasMore() const
{
  return c(0) != '\0';
}

bool Parser::isNewLine() const
{
  return c(0) == '\r' || c(0) == '\n';
}

bool Parser::isMultiLineComment() const
{
  return c(0) == '/' && c(1) == '*';
}

bool Parser::isSingleLineComment() const
{
  return c(0) == '/' && c(1) == '/';
}

bool Parser::isWhitespace() const
{
  return c(0) == ' ' || c(0) == '\t';
}

bool Parser::isCommand() const
{
  return isFirstOnLine() && c(0) == '#';
}

bool Parser::isAlpha() const
{
  return c(0) >= 'a' && c(0) <= 'z' || c(0) >= 'A' && c(0) <= 'Z';
}

bool Parser::isNumeric() const
{
  return c(0) >= '0' && c(0) <= '9';
}

bool Parser::isAlphaNumeric() const
{
  return isAlpha() || isNumeric();
}

bool Parser::isFirstOnLine() const
{
  const File& file = files.back();
  return file.first;
}

void Parser::setFirstOnLine(bool newState)
{
  File& file = files.back();
  file.first = newState;
}

///////////////////////////////////////////////////////////////////////

Parser::File::File(const char* name, const char* text):
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
