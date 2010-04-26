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

#include <vector>
#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

template <typename T>
class ResourceCodec : public Managed<ResourceCodec<T> >
{
public:
  typedef std::vector<String> SuffixList;
  inline ResourceCodec(const String& name = "");
  inline void addSuffix(const String& suffix);
  inline void removeSuffixes(void);
  inline bool supportsSuffix(const String& suffix) const;
  inline T* find(const Path& directory, const String& name);
  inline virtual T* read(const Path& path, const String& name = "") = 0;
  inline virtual T* read(Stream& stream, const String& name = "") = 0;
  inline virtual bool write(const Path& path, const T& resource) = 0;
  inline virtual bool write(Stream& stream, const T& resource) = 0;
  inline const SuffixList& getSuffixes(void) const;
private:
  SuffixList suffixes;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Generic resource base class.
 */
template <typename T>
class Resource : public Managed<T>
{
  friend class ResourceCodec<T>;
public:
  /*! Constructor.
   */
public:
  inline Resource(const String& name);
  inline const Path& getSourcePath(void) const;
  static inline T* readInstance(const String& name);
  static inline T* readInstance(const Path& path, const String& name = "");
  static inline T* readInstance(Stream& stream, const String& name = "");
  static inline bool writeInstance(const Path& path, const T& resource);
  static inline bool addSearchPath(const Path& path);
  /*! Removes all search paths from this resource type.
   */
  static inline void removeSearchPaths(void);
private:
  typedef std::vector<Path> PathList;
  Path path;
  static PathList paths;
};

///////////////////////////////////////////////////////////////////////

template <typename T, typename S>
class DerivedResource : public Resource<T>
{
public:
  inline DerivedResource(const String& name);
  static inline T* readInstance(const String& name);
  static inline T* readInstance(const Path& path, const String& name = "");
  static inline T* readInstance(Stream& stream, const String& name = "");
};

///////////////////////////////////////////////////////////////////////

template <typename T>
inline ResourceCodec<T>::ResourceCodec(const String& name):
  Managed<ResourceCodec<T> >(name)
{
}

template <typename T>
inline void ResourceCodec<T>::addSuffix(const String& suffix)
{
  SuffixList::iterator i = std::find(suffixes.begin(), suffixes.end(), suffix);
  if (i == suffixes.end())
    suffixes.push_back(suffix);
  else
    Log::writeWarning("Duplicate suffix %s not added", suffix.c_str());
}

template <typename T>
inline void ResourceCodec<T>::removeSuffixes(void)
{
  suffixes.clear();
}

template <typename T>
inline bool ResourceCodec<T>::supportsSuffix(const String& suffix) const
{
  for (SuffixList::const_iterator i = suffixes.begin();  i != suffixes.end();  i++)
  {
    if (*i == suffix)
      return true;
  }

  return false;
}

template <typename T>
inline T* ResourceCodec<T>::find(const Path& directory, const String& name)
{
  for (SuffixList::const_iterator i = suffixes.begin();  i != suffixes.end();  i++)
  {
    Path path;
    String filename;

    filename.append(name + '.' + *i);

    path = directory + filename;
    if (path.isFile())
    {
      if (T* instance = read(path, name))
	return instance;
    }

    filename.append(".gz");

    path = directory + filename;
    if (path.isFile())
    {
      if (T* instance = read(path, name))
	return instance;
    }
  }

  return NULL;
}

template <typename T>
inline T* ResourceCodec<T>::read(const Path& path, const String& name)
{
  Ptr<Stream> stream;

  if (path.getSuffix() == "gz")
    stream = GzipStream::createInstance(path, Stream::READABLE);
  else
    stream = FileStream::createInstance(path, Stream::READABLE);

  if (!stream)
    return NULL;

  T* resource = read(*stream, name);
  if (resource)
    resource->path = path;

  return resource;
}

template <typename T>
inline T* ResourceCodec<T>::read(Stream& stream, const String& name)
{
  return NULL;
}

template <typename T>
inline bool ResourceCodec<T>::write(const Path& path, const T& resource)
{
  Ptr<Stream> file;
  file = FileStream::createInstance(path, Stream::WRITABLE | Stream::OVERWRITE);
  if (!file)
    return false;

  return write(*file, resource);
}

template <typename T>
inline bool ResourceCodec<T>::write(Stream& stream, const T& resource)
{
  return false;
}

template <typename T>
inline const typename ResourceCodec<T>::SuffixList& ResourceCodec<T>::getSuffixes(void) const
{
  return suffixes;
}

///////////////////////////////////////////////////////////////////////

template <typename T>
inline Resource<T>::Resource(const String& name):
  Managed<T>(name)
{
}

template <typename T>
inline const Path& Resource<T>::getSourcePath(void) const
{
  return path;
}

template <typename T>
inline T* Resource<T>::readInstance(const String& name)
{
  if (name.empty())
  {
    Log::writeError("Resource name empty");
    return NULL;
  }

  if (T* instance = Managed<T>::findInstance(name))
    return instance;

  typedef typename ResourceCodec<T>::InstanceList CodecList;

  CodecList codecs;
  ResourceCodec<T>::getInstances(codecs);

  if (codecs.empty())
  {
    Log::writeWarning("No codecs found for reading");
    return NULL;
  }

  for (typename PathList::const_iterator p = paths.begin();  p != paths.end();  p++)
  {
    for (typename CodecList::const_iterator i = codecs.begin();  i != codecs.end();  i++)
    {
      if (T* instance = (*i)->find(*p, name))
	return instance;
    }
  }

  Log::writeWarning("Unable to find or read resource \'%s\' with any known codec", name.c_str());
  return NULL;
}

template <typename T>
inline T* Resource<T>::readInstance(const Path& path, const String& name)
{
  typedef typename ResourceCodec<T>::InstanceList CodecList;

  CodecList codecs;
  ResourceCodec<T>::getInstances(codecs);

  if (codecs.empty())
  {
    Log::writeWarning("No codecs found for reading");
    return NULL;
  }

  String suffix = path.getSuffix();

  for (typename CodecList::const_iterator i = codecs.begin();  i != codecs.end();  i++)
  {
    if ((*i)->supportsSuffix(suffix))
    {
      if (T* instance = (*i)->read(path, name))
	return instance;
    }
  }

  Log::writeWarning("Unable to read resource \'%s\' from \'%s\' with any known codec",
                    name.c_str(),
		    path.asString().c_str());
  return NULL;
}

template <typename T>
inline T* Resource<T>::readInstance(Stream& stream, const String& name)
{
  typedef typename ResourceCodec<T>::InstanceList CodecList;

  CodecList codecs;
  ResourceCodec<T>::getInstances(codecs);

  if (codecs.empty())
  {
    Log::writeWarning("No codecs found for reading");
    return NULL;
  }

  off64_t position = stream.getPosition();

  for (typename CodecList::const_iterator i = codecs.begin();  i != codecs.end();  i++)
  {
    if (T* instance = (*i)->read(stream, name))
      return instance;

    if (!stream.setPosition(position))
      return NULL;
  }

  Log::writeWarning("Unable to load resource with any known codec");
  return NULL;
}

template <typename T>
inline bool Resource<T>::writeInstance(const Path& path, const T& resource)
{
  typedef typename ResourceCodec<T>::InstanceList CodecList;

  CodecList codecs;
  ResourceCodec<T>::getInstances(codecs);

  if (codecs.empty())
  {
    Log::writeWarning("No codecs found for writing");
    return false;
  }

  String suffix = path.getSuffix();

  for (typename CodecList::const_iterator i = codecs.begin();  i != codecs.end();  i++)
  {
    if ((*i)->supportsSuffix(suffix))
    {
      if ((*i)->write(path, resource))
	return true;
      else
	Log::writeWarning("Failed to write resource \'%s\' with codec \'%s\' to path \'%s\'",
	                  resource.getName().c_str(),
			  (*i)->getName().c_str(),
			  path.asString().c_str());
    }
  }

  Log::writeWarning("Unable to write resource \'%s\' with any known codec", resource.getName().c_str());
  return false;
}

template <typename T>
inline bool Resource<T>::addSearchPath(const Path& path)
{
  if (!path.isDirectory())
  {
    Log::writeError("Invalid search path %s", path.asString().c_str());
    return false;
  }

  PathList::iterator i = std::find(paths.begin(), paths.end(), path);
  if (i == paths.end())
    paths.push_back(path);
  else
    Log::writeWarning("Duplicate search path %s not added", path.asString().c_str());

  return true;
}

template <typename T>
inline void Resource<T>::removeSearchPaths(void)
{
  paths.clear();
}

template <typename T>
typename Resource<T>::PathList Resource<T>::paths;

///////////////////////////////////////////////////////////////////////

template <typename T, typename S>
inline DerivedResource<T,S>::DerivedResource(const String& name):
  Resource<T>(name)
{
}

template <typename T, typename S>
inline T* DerivedResource<T,S>::readInstance(const String& name)
{
  if (T* object = T::findInstance(name))
    return object;

  if (S* data = S::findInstance(name))
    return T::createInstance(*data, name);

  Ptr<S> data(S::readInstance(name));
  if (!data)
    return NULL;

  return T::createInstance(*data, name);
}

template <typename T, typename S>
inline T* DerivedResource<T,S>::readInstance(const Path& path, const String& name)
{
  Ptr<S> data(S::readInstance(path));
  if (!data)
    return NULL;

  return T::createInstance(*data, name);
}

template <typename T, typename S>
inline T* DerivedResource<T,S>::readInstance(Stream& stream, const String& name)
{
  Ptr<S> data(S::readInstance(stream));
  if (!data)
    return NULL;

  return T::createInstance(*data, name);
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RESOURCE_H*/
///////////////////////////////////////////////////////////////////////
