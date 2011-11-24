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
#ifndef WENDY_PATH_H
#define WENDY_PATH_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class Pattern;
class Path;

///////////////////////////////////////////////////////////////////////

typedef std::vector<Path> PathList;

///////////////////////////////////////////////////////////////////////

/*! @brief File system path descriptor.
 *
 *  Represents the path to a single file or directory.
 */
class Path
{
public:
  /*! Constructor. Creates a path object with the specified name.
   */
  explicit Path(const String& name = "");
  /*! Creates a directory with this path.
   *  @return @c true if successful, otherwise @c false.
   */
  bool createDirectory() const;
  /*! Destroys the directory with this path.
   *  @return @c true if successful, otherwise @c false.
   */
  bool destroyDirectory() const;
  /*! @return @c true if a file or directory with this path exists, otherwise
   *  @c false.
   */
  bool exists() const;
  /*! @return This path represented as a string.
   */
  const String& asString() const;
  /*! Append operator. Creates a path with this path as the directory part and
   *  the specified name as the leaf (file or directory) name.
   *  @param[in] child The desired leaf name.
   *  @return The resulting path.
   */
  Path operator + (const String& child) const;
  /*! Appends a path separator and then the specified string (relative path) to
   *  the name of this path.
   */
  Path& operator += (const String& child);
  bool operator == (const Path& other) const;
  bool operator != (const Path& other) const;
  /*! Assignment operator.
   */
  Path& operator = (const String& newName);
  /*! @return @c true if this path is empty, otherwise @c false.
   */
  bool isEmpty() const;
  /*! @return @c true if the file or directory is readable, otherwise
   *  @c false.
   */
  bool isReadable() const;
  /*! @return @c true if the file or directory is writable, otherwise
   *  @c false.
   */
  bool isWritable() const;
  /*! @return @c true if the path represents a regular file, otherwise
   *  @c false.
   */
  bool isFile() const;
  /*! @return @c true if the path represents a directory, otherwise
   *  @c false.
   */
  bool isDirectory() const;
  /*! @return A path object representing the parent directory of this
   *  path object.
   *  @remarks The root directory is its own parent.
   */
  Path getParent() const;
  /*! Returns the paths of all files and directories in the directory with this
   *  path.
   *  @param[in,out] children The resulting list of paths.
   *  @return @c true if successful, otherwise @c false.
   */
  bool getChildren(PathList& children) const;
  /*! Returns the paths of all files and directories in the directory with this
   *  path whose names match the specified pattern.
   *  @param[in,out] children The resulting list of paths.
   *  @param[in] pattern The pattern to use.
   *  @return @c true if successful, otherwise @c false.
   */
  bool getChildren(PathList& children, const Pattern& pattern) const;
  /*! @return The suffix of the name of the represented path, or the empty
   *  string if no suffix is present.
   */
  String getSuffix() const;
  /*! @return The name part of the filename, without the directory or suffix parts.
   */
  String getName() const;
private:
  String path;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_PATH_H*/
///////////////////////////////////////////////////////////////////////
