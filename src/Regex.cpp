///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#include <wendy/Config.hpp>
#include <wendy/Core.hpp>
#include <wendy/Regex.hpp>

#define PCRE_STATIC
#include <pcre.h>

#include <memory>

namespace wendy
{

RegexMatch::RegexMatch(const String& text, int* ranges, uint count)
{
  for (uint i = 0;  i < count;  i++)
  {
    m_strings.push_back(text.substr(ranges[i * 2], ranges[i * 2 + 1]));
    m_offsets.push_back(ranges[i * 2]);
  }
}

Regex::Regex(const String& source):
  m_object(nullptr)
{
  if (!init(source))
    throw Exception("Failed to compile regex");
}

Regex::~Regex()
{
  if (m_object)
    pcre_free(m_object);
}

bool Regex::matches(const String& text) const
{
  // NOTE: Static sizes are bad, but what is one to do?
  int results[300];

  int pairs = pcre_exec((pcre*) m_object, nullptr,
                        text.c_str(), int(text.length()), 0, 0,
                        results, int(sizeof(results) / sizeof(int)));
  if (pairs < 0)
  {
    if (pairs != PCRE_ERROR_NOMATCH)
      logError("Error when matching regex");

    return false;
  }

  // Check if the whole text was matched
  if (results[0] != 0 || (size_t) results[1] != text.length())
    return false;

  return true;
}

bool Regex::contains(const String& text) const
{
  if (!pcre_exec((pcre*) m_object, nullptr, text.c_str(), int(text.length()), 0, 0, nullptr, 0))
    return false;

  return true;
}

RegexMatch Regex::match(const String& text) const
{
  // NOTE: Static sizes are bad, but what is one to do?
  int ranges[300];

  int count = pcre_exec((pcre*) m_object, nullptr,
                        text.c_str(), int(text.length()), 0, 0,
                        ranges, int(sizeof(ranges) / sizeof(int)));
  if (count < 0)
  {
    if (count != PCRE_ERROR_NOMATCH)
      logError("Error when matching regex");

    return RegexMatch();
  }

  return RegexMatch(text, ranges, count);
}

Regex* Regex::create(const String& source)
{
  std::unique_ptr<Regex> regex(new Regex());
  if (!regex->init(source))
    return nullptr;

  return regex.release();
}

Regex::Regex():
  m_object(nullptr)
{
}

bool Regex::init(const String& source)
{
  const char* message = nullptr;
  int offset = 0;

  m_object = pcre_compile(source.c_str(), 0, &message, &offset, nullptr);
  if (!m_object)
  {
    logError("Failed to compile regex: %s", message);
    return false;
  }

  return true;
}

} /*namespace wendy*/

