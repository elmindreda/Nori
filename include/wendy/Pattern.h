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
#ifndef WENDY_PATTERN_H
#define WENDY_PATTERN_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief Regular expression match descriptor.
 */
class PatternMatch
{
  friend class Pattern;
public:
  /*! @return The number of substrings in this match descriptor.
   */
  size_t getCount() const;
  /*! @param[in] index The index of the desired substring.
   *  @return The offset, in characters, of the specified substring.
   */
  size_t getOffset(uint index = 0) const;
  /*! @param[in] index The index of the desired substring.
   *  @return The desired substring.
   */
  const String& asString(uint index = 0) const;
private:
  PatternMatch(const String& text, int* ranges, uint count);
  std::vector<int> offsets;
  std::vector<String> strings;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Regular expression.
 *
 *  This class encapsulates a PCRE regular expression.
 */
class Pattern
{
public:
  /*! Constructor.
   */
  Pattern(const String& source);
  /*! Destructor.
   */
  ~Pattern();
  /*! Checks whether this pattern matches the entire specified text.
   *  @param[in] text The text to match this pattern against.
   *  @return @c true if this pattern matched the entire specified text,
   *          otherwise @c false.
   */
  bool matches(const String& text) const;
  /*! Checks whether this pattern matches any part of the specified text.
   *  @param[in] text The text to match this pattern against.
   *  @return @c true if this pattern matched any part of the specified text,
   *          otherwise @c false.
   */
  bool contains(const String& text) const;
  /*! Checks whether this pattern matches any part of the specified text.
   *  @param[in] text The text to match this pattern against.
   *  @return A match descriptor, or @c NULL if no match was found.
   */
  PatternMatch* match(const String& text) const;
  /*! Creates a pattern object with the specified pattern expression.
   *  @param[in] source The pattern expression to use.
   *  @return The newly created pattern object.
   */
  static Pattern* create(const String& source);
private:
  Pattern();
  Pattern(const Pattern& source);
  bool init(const String& source);
  Pattern& operator = (const Pattern& source);
  void* object;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_PATTERN_H*/
///////////////////////////////////////////////////////////////////////
