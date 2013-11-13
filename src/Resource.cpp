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

#include <wendy/Config.hpp>

#include <wendy/Core.hpp>
#include <wendy/Path.hpp>
#include <wendy/Resource.hpp>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

ResourceInfo::ResourceInfo(ResourceCache& initCache,
                           const String& initName,
                           const Path& initPath):
  cache(initCache),
  name(initName),
  path(initPath)
{
}

///////////////////////////////////////////////////////////////////////

Resource::Resource(const ResourceInfo& info):
  m_cache(info.cache),
  m_name(info.name),
  m_path(info.path)
{
  if (!m_name.empty())
  {
    if (m_cache.findResource(m_name))
      panic("Duplicate name for resource %s", m_name.c_str());

    m_cache.m_resources.push_back(this);
  }
}

Resource::Resource(const Resource& source):
  m_cache(source.m_cache)
{
}

Resource::~Resource()
{
  if (!m_name.empty())
  {
    m_cache.m_resources.erase(std::find(m_cache.m_resources.begin(),
                                        m_cache.m_resources.end(),
                                        this));
  }
}

Resource& Resource::operator = (const Resource& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

ResourceCache::~ResourceCache()
{
  if (!m_resources.empty())
  {
    for (auto r : m_resources)
      logError("Resource %s not destroyed", r->name().c_str());

    panic("Resource cache destroyed with attached resources");
  }
}

bool ResourceCache::addSearchPath(const Path& path)
{
  if (!path.isDirectory())
  {
    logError("Resource search path %s does not exist",
             path.name().c_str());
    return false;
  }

  m_paths.push_back(path);
  return true;
}

void ResourceCache::removeSearchPath(const Path& path)
{
  m_paths.erase(std::find(m_paths.begin(), m_paths.end(), path));
}

Resource* ResourceCache::findResource(const String& name) const
{
  for (auto& r : m_resources)
  {
    if (r->name() == name)
      return r;
  }

  return nullptr;
}

Path ResourceCache::findFile(const String& name) const
{
  if (m_paths.empty())
  {
    const Path path(name);
    if (path.isFile())
      return path;
  }
  else
  {
    for (auto& path : m_paths)
    {
      const Path full(path + name);
      if (full.isFile())
        return full;
    }
  }

  return Path();
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
