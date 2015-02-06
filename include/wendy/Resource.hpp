///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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

#pragma once

namespace wendy
{

class ResourceCache;

class ResourceInfo
{
public:
  ResourceInfo(ResourceCache& cache,
               const std::string& name = std::string(),
               const Path& path = Path());
  ResourceCache& cache;
  std::string name;
  Path path;
};

class Resource
{
public:
  Resource(const ResourceInfo& info);
  Resource(const Resource& source);
  virtual ~Resource();
  Resource& operator = (const Resource& source);
  ResourceCache& cache() const { return m_cache; }
  const std::string& name() const { return m_name; }
  const Path& path() const { return m_path; }
private:
  ResourceCache& m_cache;
  std::string m_name;
  Path m_path;
};

class ResourceCache
{
  friend class Resource;
public:
  ~ResourceCache();
  bool addSearchPath(const Path& path);
  void removeSearchPath(const Path& path);
  Resource* findResource(const std::string& name) const;
  template <typename T>
  T* find(const std::string& name) const
  {
    Resource* cached = findResource(name);
    if (!cached)
      return nullptr;

    T* cast = dynamic_cast<T*>(cached);
    if (!cast)
    {
      logError("Resource \'%s\' exists as another type", name.c_str());
      return nullptr;
    }

    return cast;
  }
  Path findFile(const std::string& name) const;
  const std::vector<Path>& searchPaths() const { return m_paths; }
private:
  std::vector<Path> m_paths;
  std::vector<Resource*> m_resources;
};

template <typename T>
class ResourceReader
{
public:
  ResourceReader(ResourceCache& cache):
    cache(cache)
  {
  }
  Ref<T> read(const std::string& name)
  {
    if (T* cached = cache.find<T>(name))
      return cached;

    const Path path = cache.findFile(name);
    if (path.isEmpty())
    {
      logError("Failed to find resource \'%s\'", name.c_str());
      return nullptr;
    }

    return read(name, path);
  }
  virtual Ref<T> read(const std::string& name, const Path& path) = 0;
protected:
  ResourceCache& cache;
};

} /*namespace wendy*/

