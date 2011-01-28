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

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Path.h>
#include <wendy/Vector.h>
#include <wendy/Quaternion.h>
#include <wendy/Color.h>
#include <wendy/XML.h>

#define XML_STATIC
#include <expat.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace XML
  {

///////////////////////////////////////////////////////////////////////

namespace
{

String escapeString(const String& string)
{
  String result;
  result.reserve(string.size());

  for (String::const_iterator c = string.begin();  c != string.end();  c++)
  {
    switch (*c)
    {
      case '<':
        result.append("&lt;");
        break;
      case '>':
        result.append("&gt;");
        break;
      case '&':
        result.append("&amp;");
        break;
      default:
        result.append(1, *c);
        break;
    }
  }

  return result;
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Reader::Reader(void):
  parser(NULL)
{
}

Reader::~Reader(void)
{
}

bool Reader::read(std::istream& stream)
{
  parser = XML_ParserCreate(NULL);
  if (parser == NULL)
  {
    logError("Failed to create XML parser");
    return false;
  }

  XML_SetElementHandler((XML_Parser) parser, startElementHandler, endElementHandler);
  XML_SetCharacterDataHandler((XML_Parser) parser, characterDataHandler);
  XML_SetEndCdataSectionHandler((XML_Parser) parser, endCharacterDataHandler);
  XML_SetUserData((XML_Parser) parser, this);

  String line;
  bool result = true;

  while (std::getline(stream, line))
  {
    if (!XML_Parse((XML_Parser) parser, line.data(), line.length(), stream.eof()))
    {
      logError("Failed to parse XML: %s",
               XML_ErrorString(XML_GetErrorCode((XML_Parser) parser)));
      result = false;
    }

    if (!result)
      break;
  }

  XML_ParserFree((XML_Parser) parser);
  return result;
}

bool Reader::onBeginElement(const String& name)
{
  return true;
}

bool Reader::onEndElement(const String& name)
{
  return true;
}

bool Reader::onCDATA(const String& data)
{
  return true;
}

bool Reader::readBoolean(const String& name, bool defaultValue)
{
  const char* stringValue = findAttributeValue(name.c_str());
  if (!stringValue)
    return defaultValue;

  if (strcasecmp(stringValue, "true") == 0)
    return true;

  if (strcasecmp(stringValue, "false") == 0)
    return false;

  errno = 0;

  int value = strtol(stringValue, NULL, 0);
  if (errno == EINVAL)
    return defaultValue;

  return value ? true : false;
}

float Reader::readFloat(const String& name, float defaultValue)
{
  const char* stringValue = findAttributeValue(name.c_str());
  if (!stringValue)
    return defaultValue;

  errno = 0;

  float value = strtof(stringValue, NULL);
  if (errno == EINVAL)
    return defaultValue;

  return value;
}

int Reader::readInteger(const String& name, int defaultValue)
{
  const char* stringValue = findAttributeValue(name.c_str());
  if (!stringValue)
    return defaultValue;

  errno = 0;

  int value = strtol(stringValue, NULL, 0);
  if (errno == EINVAL)
    return defaultValue;

  return value;
}

String Reader::readString(const String& name, const String& defaultValue)
{
  if (const char* value = findAttributeValue(name.c_str()))
    return value;

  return defaultValue;
}

Reader::Reader(const Reader& source)
{
  // NOTE: Not implemented.
}

const char* Reader::findAttributeValue(const char* name)
{
  if (attributes == NULL)
    throw Exception("Cannot read attributes outside of an element");

  for (const char** a = attributes;  *a != NULL;  a += 2)
  {
    if (std::strcmp(a[0], name) == 0)
      return a[1];
  }

  return NULL;
}

void Reader::startElementHandler(void* userData, const char* name, const char** attributes)
{
  Reader* reader = static_cast<Reader*>(userData);
  reader->attributes = attributes;

  if (!reader->onBeginElement(name))
    XML_StopParser((XML_Parser) reader->parser, false);
}

void Reader::endElementHandler(void* userData, const char* name)
{
  Reader* reader = static_cast<Reader*>(userData);
  reader->attributes = NULL;


  if (!reader->onEndElement(name))
    XML_StopParser((XML_Parser) reader->parser, false);
}

void Reader::characterDataHandler(void* userData, const char* data, int length)
{
  Reader* reader = static_cast<Reader*>(userData);
  reader->characterData.append(data, length);
}

void Reader::endCharacterDataHandler(void* userData)
{
  Reader* reader = reinterpret_cast<Reader*>(userData);

  if (!reader->onCDATA(reader->characterData))
    XML_StopParser((XML_Parser) reader->parser, false);

  reader->characterData.clear();
}

Reader& Reader::operator = (const Reader& source)
{
  // NOTE: Not implemented.

  return *this;
}

///////////////////////////////////////////////////////////////////////

Writer::Writer(void):
  closed(true),
  simple(true)
{
}

void Writer::beginElement(const String& name)
{
  if (stack.empty())
    *stream << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>" << std::endl;
  else
    closeElement();

  const char space = ' ';

  for (unsigned int i = 0;  i < stack.size();  i++)
    *stream << space;

  *stream << '<' << name;

  stack.push(name);
  closed = false;
  simple = true;
}

void Writer::endElement(void)
{
  if (stack.empty())
    throw Exception("No element to end");

  String& name = stack.top();

  if (closed)
  {
    const char space = ' ';

    for (unsigned int i = 0;  i < stack.size() - 1;  i++)
      *stream << space;

    *stream << "</" << name << ">" << std::endl;
  }
  else
  {
    if (simple)
      *stream << "/>" << std::endl;
    else
      *stream << " />" << std::endl;

    closed = true;
  }

  stack.pop();
}

void Writer::beginCDATA(void)
{
  closeElement();
}

void Writer::endCDATA(void)
{
}

template <>
void Writer::addAttribute(const String& name, const bool& value)
{
  if (closed)
    throw Exception("Attribute added outside element");

  *stream << " " << name << "=\"" << (value ? 1 : 0) << "\"";
  simple = false;
}

template <>
void Writer::addAttribute(const String& name, const int& value)
{
  if (closed)
    throw Exception("Attribute added outside element");

  *stream << " " << name << "=\"" << value << "\"";
  simple = false;
}

template <>
void Writer::addAttribute(const String& name, const unsigned int& value)
{
  if (closed)
    throw Exception("Attribute added outside element");

  *stream << " " << name << "=\"" << value << "\"";
  simple = false;
}

template <>
void Writer::addAttribute(const String& name, const float& value)
{
  if (closed)
    throw Exception("Attribute added outside element");

  *stream << " " << name << "=\"" << value << "\"";
  simple = false;
}

template <>
void Writer::addAttribute(const String& name, const Time& value)
{
  if (closed)
    throw Exception("Attribute added outside element");

  *stream << " " << name << "=\"" << value << "\"";
  simple = false;
}

template <>
void Writer::addAttribute(const String& name, const String& value)
{
  if (closed)
    throw Exception("Attribute added outside element");

  *stream << " " << name << "=\"" << escapeString(value) << "\"";
  simple = false;
}

void Writer::setStream(std::ostream* newStream)
{
  stream = newStream;
}

std::ostream* Writer::getStream(void)
{
  return stream;
}

Writer::Writer(const Writer& source)
{
  // NOTE: Not implemented.
}

void Writer::closeElement(void)
{
  if (!closed)
  {
    if (simple)
      *stream << ">" << std::endl;
    else
      *stream << " >" << std::endl;

    closed = true;
    simple = true;
  }
}

Writer& Writer::operator = (const Writer& source)
{
  // NOTE: Not implemented.

  return *this;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace XML*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
