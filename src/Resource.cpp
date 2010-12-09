///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2010 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/Resource.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

ResourceInfo::ResourceInfo(ResourceIndex& initIndex,
                           const Path& initPath):
  index(initIndex),
  path(initPath)
{
}

///////////////////////////////////////////////////////////////////////

Resource::Resource(const ResourceInfo& info):
  index(info.index),
  path(info.path)
{
  if (!path.asString().empty())
  {
    if (index.findResource(path))
      throw Exception("Duplicate path for resource");
  }

  index.resources.push_back(this);
}

Resource::Resource(const Resource& source):
  index(source.index)
{
  index.resources.push_back(this);
}

Resource::~Resource(void)
{
  index.resources.erase(std::find(index.resources.begin(),
                                  index.resources.end(),
                                  this));
}

const Path& Resource::getPath(void) const
{
  return path;
}

ResourceIndex& Resource::getIndex(void) const
{
  return index;
}

///////////////////////////////////////////////////////////////////////

ResourceIndex::~ResourceIndex(void)
{
  if (!resources.empty())
    throw Exception("Resource index destroyed with attached resources");
}

Resource* ResourceIndex::findResource(const Path& path) const
{
  for (List::const_iterator i = resources.begin();  i != resources.end();  i++)
  {
    if ((*i)->getPath() == path)
      return *i;
  }

  return NULL;
}

///////////////////////////////////////////////////////////////////////

ResourceReader::ResourceReader(ResourceIndex& index):
  index(index)
{
}

ResourceIndex& ResourceReader::getIndex(void) const
{
  return index;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
