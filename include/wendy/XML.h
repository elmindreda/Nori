///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_XML_H
#define WENDY_XML_H
///////////////////////////////////////////////////////////////////////

#include <stack>
#include <fstream>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace XML
  {

///////////////////////////////////////////////////////////////////////

class Reader
{
public:
  virtual ~Reader();
protected:
  Reader();
  bool read(std::istream& stream);
  virtual bool onBeginElement(const String& name) = 0;
  virtual bool onEndElement(const String& name) = 0;
  virtual bool onCDATA(const String& data);
  template <typename T>
  void readAttributes(T& value);
  template <typename T>
  void readAttributes(T& value, const T& defaultValue);
  bool readBoolean(const char* name, bool defaultValue = false);
  float readFloat(const char* name, float defaultValue = 0.f);
  int readInteger(const char* name, int defaultValue = 0);
  String readString(const char* name, const char* defaultValue = "");
private:
  Reader(const Reader& source);
  const char* findAttributeValue(const char* name);
  static void startElementHandler(void* userData,
                                  const char* name,
				  const char** attributes);
  static void endElementHandler(void* userData, const char* name);
  static void characterDataHandler(void* userData, const char* data, int length);
  static void endCharacterDataHandler(void* userData);
  Reader& operator = (const Reader& source);
  const char** attributes;
  String characterData;
  void* parser;
};

///////////////////////////////////////////////////////////////////////

class Writer
{
public:
  Writer();
protected:
  void beginElement(const String& name);
  void endElement();
  void beginCDATA();
  void endCDATA();
  template <typename T>
  void addAttribute(const char* name, const T& value);
  template <typename T>
  void addAttributes(const T& value);
  void setStream(std::ostream* newStream);
  std::ostream* getStream();
private:
  Writer(const Writer& source);
  void closeElement();
  Writer& operator = (const Writer& source);
  typedef std::stack<String> NameStack;
  NameStack stack;
  std::ostream* stream;
  bool closed;
  bool simple;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace XML*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_XML_H*/
///////////////////////////////////////////////////////////////////////
