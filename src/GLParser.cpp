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

bool Parser::parse(const char* name)
{
  const Path path = cache.findFile(name);
  if (path.isEmpty())
  {
    logError("Failed to find shader \'%s\'", name);
    return false;
  }

  std::ifstream stream(path.asString().c_str());
  if (stream.fail())
    throw Exception("Failed to open file");

  String text;

  stream.seekg(0, std::ios::end);
  text.resize((size_t) stream.tellg());
  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());
  stream.close();

  return parse(name, text.c_str());
}

bool Parser::parse(const char* name, const char* text)
{
  if (std::find(names.begin(), names.end(), name) != names.end())
    return true;

  output.reserve(output.size() + std::strlen(text));

  files.push_back(File(name, text));
  names.push_back(name);

  while (c(0))
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
  return true;
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

char Parser::c(ptrdiff_t offset) const
{
  const File& file = files.back();
  return file.pos[offset];
}

void Parser::parseWhitespace()
{
  while (isWhitespace())
    advance(1);
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

  while (c(0))
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

  while (c(0))
  {
    if (c(0) == '*' && c(1) == '/')
    {
      advance(2);
      appendToOutput();
      return;
    }
    else if (isNewLine())
      parseNewLine();
    else
      advance(1);
  }

  throw Exception("Expected end of comment");
}

String Parser::parseNumber()
{
  if (!isNumeric())
    throw Exception("Expected numeral");

  String number;

  while (isNumeric())
  {
    number.append(1, c(0));
    advance(1);
  }

  return number;
}

String Parser::parseIdentifier()
{
  if (!isAlpha())
    throw Exception("Expected identifier");

  String identifier;

  while (isAlphaNumeric())
  {
    identifier.append(1, c(0));
    advance(1);
  }

  return identifier;
}

String Parser::parseFileName()
{
  char terminator;
  if (c(0) == '<')
    terminator = '>';
  else if (c(0) == '\"')
    terminator = '\"';
  else
    throw Exception("Expected < or \" after #include");

  advance(1);

  String name;

  while (c(0))
  {
    if (isNewLine())
      break;
    else if (c(0) == terminator)
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

  throw Exception("Expected terminator after filename");
}

void Parser::parseCommand()
{
  advance(1);
  setFirstOnLine(false);

  parseWhitespace();
  const String command = parseIdentifier();
  parseWhitespace();

  if (command == "include")
  {
    const String name = parseFileName();
    discard();
    parse(name.c_str());
  }

  while (c(0))
  {
    if (isNewLine() || isSingleLineComment() || isMultiLineComment())
      break;

    advance(1);
  }

  appendToOutput();
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
