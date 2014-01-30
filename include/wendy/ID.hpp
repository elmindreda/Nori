///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2014 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_ID_HPP
#define WENDY_ID_HPP
//////////////////////////////////////////////////////////////////////

#include <algorithm>

//////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

enum IDBucket
{
  ID_BUCKET_ALLOCATED,
  ID_BUCKET_RELEASED,
  ID_BUCKET_UNUSED
};

///////////////////////////////////////////////////////////////////////

/*! @brief Generic ID pool.
 */
template <typename T, uint margin = 100>
class IDPool
{
public:
  IDPool(T first = 0):
    next(first)
  {
  }
  T allocateID()
  {
    if (released.size() > margin)
    {
      const T id = released.back();
      released.pop_back();
      return id;
    }

    return next++;
  }
  void releaseID(T id)
  {
    released.insert(released.begin(), id);
  }
  IDBucket bucketOf(T id)
  {
    if (id >= next)
      return ID_BUCKET_UNUSED;

    if (std::find(released.begin(), released.end(), id) != released.end())
      return ID_BUCKET_RELEASED;

    return ID_BUCKET_ALLOCATED;
  }
private:
  std::vector<T> released;
  T next;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_ID_HPP*/
///////////////////////////////////////////////////////////////////////
