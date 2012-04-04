///////////////////////////////////////////////////////////////////////
// Wendy core library
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
#ifndef WENDY_RESOURCE_H
#define WENDY_RESOURCE_H
///////////////////////////////////////////////////////////////////////

#include <fstream>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class ResourceCache;

///////////////////////////////////////////////////////////////////////

class ResourceInfo
{
public:
  ResourceInfo(ResourceCache& cache,
               const String& name = String(),
               const Path& path = Path());
  ResourceCache& cache;
  String name;
  Path path;
};

///////////////////////////////////////////////////////////////////////

class Resource : public RefObject
{
public:
  Resource(const ResourceInfo& info);
  Resource(const Resource& source);
  virtual ~Resource();
  Resource& operator = (const Resource& source);
  ResourceCache& getCache() const;
  const String& getName() const;
  const Path& getPath() const;
public:
  ResourceCache& cache;
  String name;
  Path path;
};

///////////////////////////////////////////////////////////////////////

class ResourceCache
{
  friend class Resource;
public:
  ~ResourceCache();
  bool addSearchPath(const Path& path);
  void removeSearchPath(const Path& path);
  Resource* findResource(const String& name) const;
  template <typename T>
  T* find(const String& name) const
  {
    Resource* cached = findResource(name);
    if (!cached)
      return NULL;

    T* cast = dynamic_cast<T*>(cached);
    if (!cast)
    {
      logError("Resource \'%s\' exists as another type", name.c_str());
      return NULL;
    }

    return cast;
  }
  Path findFile(const String& name) const;
  const PathList& getSearchPaths() const;
private:
  typedef std::vector<Resource*> List;
  PathList paths;
  List resources;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
class ResourceReader
{
public:
  ResourceReader(ResourceCache& initCache):
    cache(initCache)
  {
  }
  Ref<T> read(const String& name)
  {
    if (T* cached = cache.find<T>(name))
      return cached;

    const Path path = cache.findFile(name);
    if (path.isEmpty())
    {
      logError("Failed to find resource \'%s\'", name.c_str());
      return NULL;
    }

    return read(name, path);
  }
  virtual Ref<T> read(const String& name, const Path& path) = 0;
protected:
  ResourceCache& cache;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RESOURCE_H*/
///////////////////////////////////////////////////////////////////////
