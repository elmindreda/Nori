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

namespace
{

String createRandomName(void)
{
  String name = "auto:";
  for (unsigned int i = 0;  i < 8;  i++)
    name.push_back('a' + std::rand() % ('z' - 'a' + 1));

  return name;
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

ResourceInfo::ResourceInfo(ResourceIndex& initIndex,
                           const Path& initPath):
  index(initIndex),
  path(initPath)
{
}

///////////////////////////////////////////////////////////////////////

Resource::Resource(const ResourceInfo& info,
                   const String& typeName):
  type(info.index.getType(typeName)),
  path(info.path)
{
  if (!path.asString().empty())
  {
    if (type.findResource(path))
      throw Exception("Duplicate path for resource");
  }

  type.resources.push_back(this);
}

Resource::Resource(const Resource& source):
  type(source.type)
{
  type.resources.push_back(this);
}

Resource::~Resource(void)
{
  type.resources.erase(std::find(type.resources.begin(), type.resources.end(), this));
}

const Path& Resource::getPath(void) const
{
  return path;
}

ResourceIndex& Resource::getIndex(void) const
{
  return type.getIndex();
}

///////////////////////////////////////////////////////////////////////

ResourceType::ResourceType(ResourceIndex& initIndex, const String& initName):
  index(&initIndex),
  name(initName)
{
}

ResourceRef ResourceType::findResource(const Path& path) const
{
  for (List::const_iterator i = resources.begin();  i != resources.end();  i++)
  {
    if ((*i)->getPath() == path)
      return *i;
  }

  return NULL;
}

bool ResourceType::isEmpty(void) const
{
  return resources.empty();
}

const String& ResourceType::getName(void) const
{
  return name;
}

ResourceIndex& ResourceType::getIndex(void) const
{
  return *index;
}

///////////////////////////////////////////////////////////////////////

ResourceIndex::~ResourceIndex(void)
{
  for (TypeList::iterator i = types.begin();  i != types.end();  i++)
  {
    if (!i->isEmpty())
      throw Exception("Resource index destroyed with attached resources");
  }
}

ResourceType& ResourceIndex::getType(const String& name)
{
  for (TypeList::iterator i = types.begin();  i != types.end();  i++)
  {
    if (i->getName() == name)
      return *i;
  }

  types.push_back(ResourceType(*this, name));
  return types.back();
}

const ResourceIndex::TypeList& ResourceIndex::getTypes(void)
{
  return types;
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
