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
#ifndef WENDY_GLPARSER_H
#define WENDY_GLPARSER_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

class ShaderPreprocessor
{
public:
  ShaderPreprocessor(ResourceCache& cache);
  void parse(const char* name);
  void parse(const char* name, const char* text);
  bool hasVersion() const;
  const String& getOutput() const;
  const String& getVersion() const;
  const String& getNameList() const;
private:
  void addLine();
  void advance(size_t offset);
  void discard();
  void appendToOutput();
  void appendToOutput(const char* text);
  char c(ptrdiff_t offset) const;
  void passWhitespace();
  void parseWhitespace();
  void parseNewLine();
  void parseSingleLineComment();
  void parseMultiLineComment();
  String passNumber();
  String passIdentifier();
  String passShaderName();
  void parseCommand();
  bool hasMore() const;
  bool isNewLine() const;
  bool isMultiLineComment() const;
  bool isSingleLineComment() const;
  bool isWhitespace() const;
  bool isCommand() const;
  bool isAlpha() const;
  bool isNumeric() const;
  bool isAlphaNumeric() const;
  bool isFirstOnLine() const;
  void setFirstOnLine(bool newState);
  class File;
  typedef std::vector<String> NameList;
  typedef std::vector<File> FileList;
  ResourceCache& cache;
  FileList files;
  NameList names;
  String output;
  String version;
  String list;
};

///////////////////////////////////////////////////////////////////////

class ShaderPreprocessor::File
{
public:
  File(const char* name, const char* text);
  const char* name;
  const char* text;
  const char* base;
  const char* pos;
  unsigned int line;
  bool first;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLPARSER_H*/
///////////////////////////////////////////////////////////////////////
