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
#include <wendy/Pattern.h>
#include <wendy/Path.h>

#if WENDY_HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if WENDY_HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if WENDY_HAVE_DIRECT_H
#include <direct.h>
#endif

#if WENDY_HAVE_UNISTD_H
#include <unistd.h>
#endif

#if WENDY_HAVE_DIRENT_H
#include <dirent.h>
#endif

#if WENDY_HAVE_WINDOWS_H
#include <windows.h>
#endif

#if WENDY_HAVE_IO_H
#include <io.h>
#endif

#include <cstdio>
#include <cstdlib>

///////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER

#define F_OK 0
#define W_OK 2
#define R_OK 4

#define S_ISDIR(x) ((x) & _S_IFDIR)
#define S_ISREG(x) ((x) & _S_IFREG)

#endif /*_MSC_VER*/

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Path::Path(const String& initName)
{
  operator = (initName);
}

bool Path::createDirectory() const
{
#ifdef _WIN32
  return _mkdir(path.c_str()) == 0;
#else
  return mkdir(path.c_str(), 0777) == 0;
#endif
}

bool Path::destroyDirectory() const
{
#ifdef _WIN32
  return _rmdir(path.c_str()) == 0;
#else
  return rmdir(path.c_str()) == 0;
#endif
}

bool Path::exists() const
{
#ifdef _WIN32
  return _access(path.c_str(), F_OK) == 0;
#else
  return access(path.c_str(), F_OK) == 0;
#endif
}

const String& Path::asString() const
{
  return path;
}

Path Path::operator + (const String& child) const
{
  return Path(path + '/' + child);
}

Path& Path::operator += (const String& child)
{
  return operator = (path + '/' + child);
}

bool Path::operator == (const Path& other) const
{
  // TODO: Implement sane code.

  return path == other.path;
}

bool Path::operator != (const Path& other) const
{
  // TODO: Implement sane code.

  return path != other.path;
}

Path& Path::operator = (const String& newPath)
{
  path = newPath;

  if (!path.empty())
  {
    // Remove extraneous trailing slashes

    while (String::size_type end = path.length() - 1)
    {
      if (path[end] != '/')
        break;

      path.erase(end);
    }

    // TODO: Compact repeated slashes.
  }

  return *this;
}

bool Path::isEmpty() const
{
  return path.empty();
}

bool Path::isReadable() const
{
#ifdef _WIN32
  return _access(path.c_str(), R_OK) == 0;
#else
  return access(path.c_str(), R_OK) == 0;
#endif
}

bool Path::isWritable() const
{
#ifdef _WIN32
  return _access(path.c_str(), W_OK) == 0;
#else
  return access(path.c_str(), W_OK) == 0;
#endif
}

bool Path::isFile() const
{
#ifdef _WIN32
  struct _stati64 sb;

  if (_stati64(path.c_str(), &sb) != 0)
    return false;
#else
  struct stat64 sb;

  if (stat64(path.c_str(), &sb) != 0)
    return false;
#endif

  return S_ISREG(sb.st_mode) ? true : false;
}

bool Path::isDirectory() const
{
#ifdef _WIN32
  struct _stati64 sb;

  if (_stati64(path.c_str(), &sb) != 0)
    return false;
#else
  struct stat64 sb;

  if (stat64(path.c_str(), &sb) != 0)
    return false;
#endif

  return S_ISDIR(sb.st_mode) ? true : false;
}

Path Path::getParent() const
{
  // TODO: Fix this.

  String::size_type offset = path.find_last_of('/');
  if (offset == String::npos)
    return Path(".");

  return Path(path.substr(0, offset + 1));
}

bool Path::getChildren(PathList& children) const
{
#if _WIN32
  WIN32_FIND_DATA data;
  HANDLE search;

  search = FindFirstFile(path.c_str(), &data);
  if (search == INVALID_HANDLE_VALUE)
    return false;

  do
  {
    children.push_back(operator + (data.cFileName));
  }
  while (FindNextFile(search, &data));

  FindClose(search);
#else
  DIR* stream;
  dirent* entry;

  stream = opendir(path.c_str());
  if (!stream)
    return false;

  while ((entry = readdir(stream)))
    children.push_back(operator + (entry->d_name));

  closedir(stream);
#endif
  return true;
}

bool Path::getChildren(PathList& children, const Pattern& pattern) const
{
#if _WIN32
  WIN32_FIND_DATA data;
  HANDLE search;

  search = FindFirstFile(path.c_str(), &data);
  if (search == INVALID_HANDLE_VALUE)
    return false;

  do
  {
    if (pattern.matches(data.cFileName))
      children.push_back(operator + (data.cFileName));
  }
  while (FindNextFile(search, &data));

  FindClose(search);
#else
  DIR* stream;
  dirent* entry;

  stream = opendir(path.c_str());
  if (!stream)
    return false;

  while ((entry = readdir(stream)))
  {
    if (pattern.matches(entry->d_name))
      children.push_back(operator + (entry->d_name));
  }

  closedir(stream);
#endif
  return true;
}

String Path::getSuffix() const
{
  String last;

  String::size_type start = path.find_last_of('/');
  if (start == String::npos)
    last = path;
  else
    last = path.substr(start, String::npos);

  String::size_type offset = last.find_last_of('.');
  if (offset == String::npos)
    return "";

  return last.substr(offset + 1, String::npos);
}

String Path::getName() const
{
  String::size_type start = path.find_last_of('/');
  String::size_type end = path.find_last_of('.');

  if (start == String::npos)
    start = 0;
  else
    start++;

  if (end != String::npos && end < start)
    end = String::npos;

  return path.substr(start, end - start);
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
