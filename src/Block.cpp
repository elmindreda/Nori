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
#include <wendy/Block.h>

#include <cstring>
#include <cstdlib>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Block::Block(size_t initSize):
  size(0),
  data(NULL)
{
  resize(initSize);
}

Block::Block(const uint8* source, size_t sourceSize):
  size(0),
  data(NULL)
{
  resize(sourceSize);
  std::memcpy(data, source, sourceSize);
}

Block::Block(const Block& source):
  size(0),
  data(NULL)
{
  operator = (source);
}

Block::~Block()
{
  destroy();
}

void Block::resize(size_t newSize)
{
  if (newSize == 0)
    destroy();
  else
  {
    if (size == 0)
      data = (uint8*) std::malloc(newSize);
    else
    {
      uint8* newData = (uint8*) std::realloc(data, newSize);
      assert(newData);
      data = newData;
    }

    size = newSize;
  }
}

void Block::attach(uint8* newData, size_t newSize)
{
  destroy();

  data = newData;
  size = newSize;
}

uint8* Block::detach()
{
  uint8* detached = data;

  data = NULL;
  size = 0;

  return detached;
}

void Block::destroy()
{
  if (size > 0)
  {
    free(data);
    data = NULL;
    size = 0;
  }
}

Block::operator uint8* ()
{
  return data;
}

Block::operator const uint8* () const
{
  return data;
}

Block& Block::operator = (const Block& source)
{
  if (source.size == 0)
    destroy();
  else
  {
    resize(source.size);
    std::memcpy(data, source.data, source.size);
  }

  return *this;
}

size_t Block::getSize() const
{
  return size;
}

uint8* Block::getData()
{
  return data;
}

const uint8* Block::getData() const
{
  return data;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
