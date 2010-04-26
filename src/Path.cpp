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

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Path::Path(const String& initName)
{
  operator = (initName);
}

Path::Path(const char* format, ...)
{
  va_list vl;
  char* message;
  int result;

  va_start(vl, format);
  result = vasprintf(&message, format, vl);
  va_end(vl);

  if (result != -1)
  {
    operator = (message);
    std::free(message);
  }
}

bool Path::createDirectory(void) const
{
#ifdef _MSC_VER
  return _mkdir(name.c_str()) == 0;
#else
  return mkdir(name.c_str(), 0777) == 0;
#endif
}

bool Path::destroyDirectory(void) const
{
#ifdef _MSC_VER
  return _rmdir(name.c_str()) == 0;
#else
  return rmdir(name.c_str()) == 0;
#endif
}

bool Path::exists(void) const
{
#ifdef _MSC_VER
  return _access(name.c_str(), F_OK) == 0;
#else
  return access(name.c_str(), F_OK) == 0;
#endif
}

const String& Path::asString(void) const
{
  return name;
}

Path Path::operator + (const String& child) const
{
  return Path(name + '/' + child);
}

Path& Path::operator += (const String& child)
{
  String newName = name;
  newName.append(1, '/');
  newName.append(child);

  return operator = (newName);
}

bool Path::operator == (const Path& other) const
{
  // TODO: Implement sane code.

  return name == other.name;
}

bool Path::operator != (const Path& other) const
{
  // TODO: Implement sane code.

  return name != other.name;
}

Path& Path::operator = (const String& newName)
{
  name = newName;

  if (!name.empty())
  {
    // Remove extraneous trailing slashes

    while (String::size_type end = name.length() - 1)
    {
      if (name[end] != '/')
	break;

      name.erase(end);
    }

    // TODO: Compact repeated slashes.
  }

  return *this;
}

bool Path::isReadable(void) const
{
#ifdef _MSC_VER
  return _access(name.c_str(), R_OK) == 0;
#else
  return access(name.c_str(), R_OK) == 0;
#endif
}

bool Path::isWritable(void) const
{
#ifdef _MSC_VER
  return _access(name.c_str(), W_OK) == 0;
#else
  return access(name.c_str(), W_OK) == 0;
#endif
}

bool Path::isFile(void) const
{
#ifdef _MSC_VER
  struct _stati64 sb;

  if (_stati64(name.c_str(), &sb) != 0)
    return false;
#else
  struct stat64 sb;

  if (stat64(name.c_str(), &sb) != 0)
    return false;
#endif

  return S_ISREG(sb.st_mode) ? true : false;
}

bool Path::isDirectory(void) const
{
#ifdef _MSC_VER
  struct _stati64 sb;

  if (_stati64(name.c_str(), &sb) != 0)
    return false;
#else
  struct stat64 sb;

  if (stat64(name.c_str(), &sb) != 0)
    return false;
#endif

  return S_ISDIR(sb.st_mode) ? true : false;
}

Path Path::getParent(void) const
{
  // TODO: Fix this.

  String::size_type offset = name.find_last_of('/');
  if (offset == String::npos)
    return Path(".");

  return Path(name.substr(0, offset + 1));
}

bool Path::getChildren(List& children) const
{
#if _WIN32
  WIN32_FIND_DATA data;
  HANDLE search;

  search = FindFirstFile(name.c_str(), &data);
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

  stream = opendir(name.c_str());
  if (!stream)
    return false;

  while (entry = readdir(stream))
    children.push_back(operator + (entry->d_name));

  closedir(stream);
#endif
  return true;
}

bool Path::getChildren(List& children, const Pattern& pattern) const
{
#if _WIN32
  WIN32_FIND_DATA data;
  HANDLE search;

  search = FindFirstFile(name.c_str(), &data);
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

  stream = opendir(name.c_str());
  if (!stream)
    return false;

  while (entry = readdir(stream))
  {
    if (pattern.matches(entry->d_name))
      children.push_back(operator + (entry->d_name));
  }

  closedir(stream);
#endif
  return true;
}

String Path::getSuffix(void) const
{
  String last;

  String::size_type start = name.rfind('/');
  if (start == String::npos)
    last = name;
  else
    last = name.substr(start, String::npos);

  String::size_type offset = last.find_last_of('.');
  if (offset == String::npos)
    return "";

  return last.substr(offset + 1, String::npos);
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
