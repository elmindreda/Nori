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
#ifndef WENDY_BIMAP_H
#define WENDY_BIMAP_H
///////////////////////////////////////////////////////////////////////

#include <vector>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief Bidirectional map template.
 *
 *  This is a performance-wise highly suboptimal, but syntactically delicious,
 *  way of maintaining a bidirectional mapping between values, which is often
 *  useful when you wish to hide platform- or API-specific constants and
 *  enumerations, and need to convert back and forth between them and you
 *  public enumeration values.
 *
 *  @remarks For this class to work, the types must be distinguishable by the
 *  compiler.
 */
template <typename X, typename Y>
class Bimap
{
public:
  Bimap();
  Bimap(const X& defaultX, const Y& defaultY);
  X& operator [] (const Y& value);
  const X& operator [] (const Y& value) const;
  Y& operator [] (const X& value);
  const Y& operator [] (const X& value) const;
  bool isEmpty() const;
  bool hasKey(const X& key) const;
  bool hasKey(const Y& key) const;
  void setDefaults(const X& defaultX, const Y& defaultY);
private:
  struct Entry
  {
    X x;
    Y y;
  };
  typedef std::vector<Entry> EntryList;
  EntryList entries;
  Entry defaults;
};

///////////////////////////////////////////////////////////////////////

template <typename X, typename Y>
inline Bimap<X,Y>::Bimap()
{
}

template <typename X, typename Y>
inline Bimap<X,Y>::Bimap(const X& defaultX, const Y& defaultY)
{
  setDefaults(defaultX, defaultY);
}

template <typename X, typename Y>
inline X& Bimap<X,Y>::operator [] (const Y& key)
{
  for (auto i = entries.begin();  i != entries.end();  i++)
  {
    if ((*i).y == key)
      return (*i).x;
  }

  entries.push_back(Entry());
  Entry& entry = entries.back();

  entry.y = key;
  return entry.x;
}

template <typename X, typename Y>
inline const X& Bimap<X,Y>::operator [] (const Y& key) const
{
  for (auto i = entries.begin();  i != entries.end();  i++)
  {
    if ((*i).y == key)
      return (*i).x;
  }

  return defaults.x;
}

template <typename X, typename Y>
inline Y& Bimap<X,Y>::operator [] (const X& key)
{
  for (auto i = entries.begin();  i != entries.end();  i++)
  {
    if ((*i).x == key)
      return (*i).y;
  }

  entries.push_back(Entry());
  Entry& entry = entries.back();

  entry.x = key;
  return entry.y;
}

template <typename X, typename Y>
inline const Y& Bimap<X,Y>::operator [] (const X& key) const
{
  for (auto i = entries.begin();  i != entries.end();  i++)
  {
    if ((*i).x == key)
      return (*i).y;
  }

  return defaults.y;
}

template <typename X, typename Y>
inline bool Bimap<X,Y>::isEmpty() const
{
  return entries.empty();
}

template <typename X, typename Y>
inline bool Bimap<X,Y>::hasKey(const X& key) const
{
  for (auto i = entries.begin();  i != entries.end();  i++)
  {
    if ((*i).x == key)
      return true;
  }

  return false;
}

template <typename X, typename Y>
inline bool Bimap<X,Y>::hasKey(const Y& key) const
{
  for (auto i = entries.begin();  i != entries.end();  i++)
  {
    if ((*i).y == key)
      return true;
  }

  return false;
}

template <typename X, typename Y>
inline void Bimap<X,Y>::setDefaults(const X& defaultX, const Y& defaultY)
{
  defaults.x = defaultX;
  defaults.y = defaultY;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_BIMAP_H*/
///////////////////////////////////////////////////////////////////////
