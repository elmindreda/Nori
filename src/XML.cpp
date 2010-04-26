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
#include <wendy/Block.h>
#include <wendy/Path.h>
#include <wendy/Stream.h>
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

bool Reader::read(Stream& stream)
{
  parser = XML_ParserCreate(NULL);
  if (parser == NULL)
  {
    Log::writeError("Failed to create XML parser");
    return false;
  }

  XML_SetElementHandler((XML_Parser) parser, startElementHandler, endElementHandler);
  XML_SetCharacterDataHandler((XML_Parser) parser, characterDataHandler);
  XML_SetEndCdataSectionHandler((XML_Parser) parser, endCharacterDataHandler);
  XML_SetUserData((XML_Parser) parser, this);

  TextStream textStream(stream, false);

  String text;
  text.reserve(65536);

  bool result = true;

  while (textStream.readText(text, text.capacity()))
  {
    if (!XML_Parse((XML_Parser) parser, text.c_str(), (unsigned int) text.length(), textStream.isEOF()))
    {
      Log::writeError("Failed to parse XML: %s", XML_ErrorString(XML_GetErrorCode((XML_Parser) parser)));
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

template <>
void Reader::readAttributes(Vec2& value)
{
  value.x = readFloat("x", 0.f);
  value.y = readFloat("y", 0.f);
}

template <>
void Reader::readAttributes(Vec3& value)
{
  value.x = readFloat("x", 0.f);
  value.y = readFloat("y", 0.f);
  value.z = readFloat("z", 0.f);
}

template <>
void Reader::readAttributes(Vec4& value)
{
  value.x = readFloat("x", 0.f);
  value.y = readFloat("y", 0.f);
  value.z = readFloat("z", 0.f);
  value.w = readFloat("w", 0.f);
}

template <>
void Reader::readAttributes(ColorRGB& value)
{
  value.r = readFloat("r", 0.f);
  value.g = readFloat("g", 0.f);
  value.b = readFloat("b", 0.f);
}

template <>
void Reader::readAttributes(ColorRGBA& value)
{
  value.r = readFloat("r", 0.f);
  value.g = readFloat("g", 0.f);
  value.b = readFloat("b", 0.f);
  value.a = readFloat("a", 0.f);
}

template <>
void Reader::readAttributes(Vec2& value, const Vec2& defaultValue)
{
  value.x = readFloat("x", defaultValue.x);
  value.y = readFloat("y", defaultValue.y);
}

template <>
void Reader::readAttributes(Vec3& value, const Vec3& defaultValue)
{
  value.x = readFloat("x", defaultValue.x);
  value.y = readFloat("y", defaultValue.y);
  value.z = readFloat("z", defaultValue.z);
}

template <>
void Reader::readAttributes(Vec4& value, const Vec4& defaultValue)
{
  value.x = readFloat("x", defaultValue.x);
  value.y = readFloat("y", defaultValue.y);
  value.z = readFloat("z", defaultValue.z);
  value.w = readFloat("w", defaultValue.w);
}

template <>
void Reader::readAttributes(Quat& value, const Quat& defaultValue)
{
  value.x = readFloat("x", defaultValue.x);
  value.y = readFloat("y", defaultValue.y);
  value.z = readFloat("z", defaultValue.z);
  value.w = readFloat("w", defaultValue.w);
}

template <>
void Reader::readAttributes(ColorRGB& value, const ColorRGB& defaultValue)
{
  value.r = readFloat("r", defaultValue.r);
  value.g = readFloat("g", defaultValue.g);
  value.b = readFloat("b", defaultValue.b);
}

template <>
void Reader::readAttributes(ColorRGBA& value, const ColorRGBA& defaultValue)
{
  value.r = readFloat("r", defaultValue.r);
  value.g = readFloat("g", defaultValue.g);
  value.b = readFloat("b", defaultValue.b);
  value.a = readFloat("a", defaultValue.a);
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
  Reader* reader = reinterpret_cast<Reader*>(userData);
  reader->attributes = attributes;

  if (!reader->onBeginElement(name))
    XML_StopParser((XML_Parser) reader->parser, false);
}

void Reader::endElementHandler(void* userData, const char* name)
{
  Reader* reader = reinterpret_cast<Reader*>(userData);
  reader->attributes = NULL;

  if (!reader->onEndElement(name))
    XML_StopParser((XML_Parser) reader->parser, false);
}

void Reader::characterDataHandler(void* userData, const char* data, int length)
{
  Reader* reader = reinterpret_cast<Reader*>(userData);
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
    stream->writeLine("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>");
  else
    closeElement();

  const char space = ' ';

  for (unsigned int i = 0;  i < stack.size();  i++)
    stream->writeItem(space);

  stream->writeText("<%s", name.c_str());

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
      stream->writeItem(space);
    stream->writeLine("</%s>", name.c_str());
  }
  else
  {
    if (simple)
      stream->writeLine("/>");
    else
      stream->writeLine(" />");

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

  stream->writeText(" %s=\"%u\"", name.c_str(), value ? 1 : 0);
  simple = false;
}

template <>
void Writer::addAttribute(const String& name, const int& value)
{
  if (closed)
    throw Exception("Attribute added outside element");

  stream->writeText(" %s=\"%i\"", name.c_str(), value);
  simple = false;
}

template <>
void Writer::addAttribute(const String& name, const unsigned int& value)
{
  if (closed)
    throw Exception("Attribute added outside element");

  stream->writeText(" %s=\"%u\"", name.c_str(), value);
  simple = false;
}

template <>
void Writer::addAttribute(const String& name, const float& value)
{
  if (closed)
    throw Exception("Attribute added outside element");

  stream->writeText(" %s=\"%f\"", name.c_str(), value);
  simple = false;
}

template <>
void Writer::addAttribute(const String& name, const Time& value)
{
  if (closed)
    throw Exception("Attribute added outside element");

  stream->writeText(" %s=\"%f\"", name.c_str(), value);
  simple = false;
}

template <>
void Writer::addAttribute(const String& name, const String& value)
{
  if (closed)
    throw Exception("Attribute added outside element");

  stream->writeText(" %s=\"%s\"", name.c_str(), escapeString(value).c_str());
  simple = false;
}

template <>
void Writer::addAttributes(const ColorRGB& value)
{
  addAttribute("r", value.r);
  addAttribute("g", value.g);
  addAttribute("b", value.b);
}

template <>
void Writer::addAttributes(const ColorRGBA& value)
{
  addAttribute("r", value.r);
  addAttribute("g", value.g);
  addAttribute("b", value.b);
  addAttribute("a", value.a);
}

template <>
void Writer::addAttributes(const Vec2& value)
{
  addAttribute("x", value.x);
  addAttribute("y", value.y);
}

template <>
void Writer::addAttributes(const Vec3& value)
{
  addAttribute("x", value.x);
  addAttribute("y", value.y);
  addAttribute("z", value.z);
}

template <>
void Writer::addAttributes(const Vec4& value)
{
  addAttribute("x", value.x);
  addAttribute("y", value.y);
  addAttribute("z", value.z);
  addAttribute("w", value.w);
}

void Writer::setStream(Stream* newStream)
{
  if (newStream)
    stream = new TextStream(*newStream, false);
  else
    stream = NULL;
}

TextStream* Writer::getStream(void)
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
      stream->writeLine(">");
    else
      stream->writeLine(" >");

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
