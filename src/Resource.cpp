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

ResourceInfo::ResourceInfo(ResourceCache& initCache,
                           const Path& initPath):
  cache(initCache),
  path(initPath)
{
}

///////////////////////////////////////////////////////////////////////

Resource::Resource(const ResourceInfo& info):
  cache(info.cache),
  path(info.path)
{
  if (!path.isEmpty())
  {
    if (cache.findResource(path))
      panic("Duplicate path for resource \'%s\'", path.asString().c_str());

    cache.resources.push_back(this);
  }
}

Resource::Resource(const Resource& source):
  cache(source.cache)
{
}

Resource::~Resource()
{
  if (!path.isEmpty())
  {
    cache.resources.erase(std::find(cache.resources.begin(),
                                    cache.resources.end(),
                                    this));
  }
}

Resource& Resource::operator = (const Resource& source)
{
  return *this;
}

const Path& Resource::getPath() const
{
  return path;
}

ResourceCache& Resource::getCache() const
{
  return cache;
}

///////////////////////////////////////////////////////////////////////

ResourceCache::~ResourceCache()
{
  if (!resources.empty())
    panic("Resource cache destroyed with attached resources");
}

bool ResourceCache::addSearchPath(const Path& path)
{
  if (!path.isDirectory())
  {
    logError("Resource search path \'%s\' does not exist",
             path.asString().c_str());
    return false;
  }

  if (std::find(paths.begin(), paths.end(), path) == paths.end())
    paths.push_back(path);

  return true;
}

void ResourceCache::removeSearchPath(const Path& path)
{
  paths.erase(std::find(paths.begin(), paths.end(), path));
}

Resource* ResourceCache::findResource(const Path& path) const
{
  for (List::const_iterator i = resources.begin();  i != resources.end();  i++)
  {
    if ((*i)->getPath() == path)
      return *i;
  }

  return NULL;
}

bool ResourceCache::openFile(std::ifstream& stream, const Path& path) const
{
  const Path full = findFile(path);
  if (full.isEmpty())
    return false;

  stream.open(full.asString().c_str(), std::ios::in | std::ios::binary);
  if (stream.fail())
    return false;

  return true;
}

Path ResourceCache::findFile(const Path& path) const
{
  if (paths.empty())
  {
    if (path.isFile())
      return path;
  }
  else
  {
    for (PathList::const_iterator p = paths.begin();  p != paths.end();  p++)
    {
      Path full = *p + path.asString();
      if (full.isFile())
        return full;
    }
  }

  return Path();
}

const PathList& ResourceCache::getSearchPaths() const
{
  return paths;
}

///////////////////////////////////////////////////////////////////////

ResourceReader::ResourceReader(ResourceCache& initCache):
  cache(initCache)
{
}

ResourceCache& ResourceReader::getCache() const
{
  return cache;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
