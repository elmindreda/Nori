///////////////////////////////////////////////////////////////////////
// Wendy core library
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

#include <wendy/Config.h>
#include <wendy/Core.h>
#include <wendy/Pattern.h>

#define PCRE_STATIC
#include <pcre.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

PatternMatch::PatternMatch(const String& text, int* ranges, uint count)
{
  for (uint i = 0;  i < count;  i++)
  {
    m_strings.push_back(text.substr(ranges[i * 2], ranges[i * 2 + 1]));
    m_offsets.push_back(ranges[i * 2]);
  }
}

///////////////////////////////////////////////////////////////////////

Pattern::Pattern(const String& source):
  m_object(NULL)
{
  if (!init(source))
    throw Exception("Failed to compile PCRE pattern");
}

Pattern::~Pattern()
{
  if (m_object)
    pcre_free(m_object);
}

bool Pattern::matches(const String& text) const
{
  // NOTE: Static sizes are bad, but what is one to do?
  int results[300];

  int pairs = pcre_exec((pcre*) m_object, NULL,
                        text.c_str(), text.length(), 0, 0,
                        results, sizeof(results) / sizeof(int));
  if (pairs < 0)
  {
    if (pairs != PCRE_ERROR_NOMATCH)
      logError("Error when matching pattern");

    return false;
  }

  // Check if the whole text was matched
  if (results[0] != 0 || (size_t) results[1] != text.length())
    return false;

  return true;
}

bool Pattern::contains(const String& text) const
{
  if (!pcre_exec((pcre*) m_object, NULL, text.c_str(), text.length(), 0, 0, NULL, 0))
    return false;

  return true;
}

PatternMatch* Pattern::match(const String& text) const
{
  // NOTE: Static sizes are bad, but what is one to do?
  int ranges[300];

  int count = pcre_exec((pcre*) m_object, NULL,
                        text.c_str(), text.length(), 0, 0,
                        ranges, sizeof(ranges) / sizeof(int));
  if (count < 0)
  {
    if (count != PCRE_ERROR_NOMATCH)
      logError("Error when matching pattern");

    return NULL;
  }

  return new PatternMatch(text, ranges, count);
}

Pattern* Pattern::create(const String& source)
{
  Ptr<Pattern> pattern(new Pattern());
  if (!pattern->init(source))
    return NULL;

  return pattern.detachObject();
}

Pattern::Pattern():
  m_object(NULL)
{
}

Pattern::Pattern(const Pattern& source)
{
  panic("Pattern objects may not be copied");
}

bool Pattern::init(const String& source)
{
  const char* message = NULL;
  int offset = 0;

  m_object = pcre_compile(source.c_str(), 0, &message, &offset, NULL);
  if (!m_object)
  {
    logError("Failed to compile PCRE pattern: %s", message);
    return false;
  }

  return true;
}

Pattern& Pattern::operator = (const Pattern& source)
{
  panic("Pattern objects may not be copied");
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
