///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_STREAM_H
#define WENDY_STREAM_H
///////////////////////////////////////////////////////////////////////

#if WENDY_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <cstdio>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief Generic byte stream.
 */
class Stream
{
public:
  enum
  {
    /*! The stream will be readable.
     */
    READABLE = 1,
    /*! The stream will be writable.
     */
    WRITABLE = 2,
    /*! The (file) stream will overwrite any existing file.
     */
    OVERWRITE = 4,
  };
  /*! Destructor.
   */
  virtual ~Stream(void);
  /*! Reads data from the stream.
   *  @param[out] data The target buffer for the read data.
   *  @param[in] size The number of bytes to be read.
   *  @return The number of bytes actually read.
   */
  virtual size_t read(void* data, size_t size) = 0;
  /*! Writes data to the stream.
   */
  virtual size_t write(const void* data, size_t size) = 0;
  /*! Flushes the stream's buffers.
   */
  virtual void flush(void) = 0;
  /*! Reads one data item from the stream.
   */
  template <typename T>
  inline bool readItem(T& item)
    { return read(&item, sizeof(T)) == sizeof(T); }
  /*! Writes one data item to the stream.
   */
  template <typename T>
  inline bool writeItem(T& item)
    { return write(&item, sizeof(T)) == sizeof(T); }
  /*! Reads a number of data items from the stream.
   */
  template <typename T>
  inline bool readItems(T* items, size_t count)
    { return read(items, sizeof(T) * count) == sizeof(T) * count; }
  /*! Writes a number of data items to the stream.
    */
  template <typename T>
  inline bool writeItems(T* items, size_t count)
    { return write(items, sizeof(T) * count) == sizeof(T) * count; }
  /*! @return @c true if the end of the stream has been reached, otherwise @c false.
   */
  virtual bool isEOF(void) const = 0;
  /*! @return @c true if the stream is readable, otherwise @c false.
   */
  virtual bool isReadable(void) const = 0;
  /*! @return @c true if the stream is writable, otherwise @c false.
    */
  virtual bool isWritable(void) const = 0;
  /*! @return @c true if the stream is seekable, otherwise @c false.
   */
  virtual bool isSeekable(void) const = 0;
  /*! @return The size, in bytes, of the stream.
   */
  virtual off64_t getSize(void) const = 0;
  /*! @return The current position, in bytes, within the stream.
   */
  virtual off64_t getPosition(void) const = 0;
  /*! Sets the current position within the stream.
   *  @param position The desired position, in bytes, from the beginning of the stream.
   *  @return @c true if successful, otherwise @c false.
   *  @remarks If seeking fails on a seekable stream, the current position is undefined.
   */
  virtual bool setPosition(off64_t position) = 0;
};

///////////////////////////////////////////////////////////////////////

/*! @brief POSIX stream.
 *
 *  Wraps a POSIX file stream object.
 */
class PosixStream : public Stream
{
public:
  /*! Destructor.
   */
  ~PosixStream(void);
  size_t read(void* data, size_t size);
  size_t write(const void* data, size_t size);
  void flush(void);
  bool isEOF(void) const;
  bool isReadable(void) const;
  bool isWritable(void) const;
  off64_t getSize(void) const;
  off64_t getPosition(void) const;
  bool setPosition(off64_t position);
protected:
  PosixStream(void);
  PosixStream(const PosixStream& source);
  bool convertFlags(unsigned int flags, String& mode);
  PosixStream& operator = (const PosixStream& source);
  std::FILE* file;
  unsigned int flags;
};

///////////////////////////////////////////////////////////////////////

/*! @brief File stream.
 *
 *  Wraps a POSIX file stream opened from a file.
 */
class FileStream : public PosixStream
{
public:
  /*! @return The path of the source file.
   */
  bool isSeekable(void) const;
  /*! @return The path of the file underlying this stream.
   */
  const Path& getPath(void) const;
  /*! Creates a file stream on the specified file.
   *  @param[in] path The path name of the source file.
   *  @param[in] flags The mode flags to use.
   *  @return The newly created stream, or @c NULL if an error occurs.
   */
  static FileStream* createInstance(const Path& path, unsigned int flags);
protected:
  FileStream(void);
  bool init(const Path& path, unsigned int flags);
  Path path;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Text I/O stream wrapper.
 */
class TextStream : public Stream
{
public:
  /*! Constructor. Creates a text stream around the specified stream.
   *  @param[in] stream The regular stream to wrap.
   *  @param[in] owner @c true if you wish to transfer ownership of the
   *  specified stream, otherwise @c false.
   */
  TextStream(Stream& stream, bool owner = true);
  /*! Destructor. Destroys the wrapped stream, if this stream has
   *  been designated its owner.
   */
  ~TextStream(void);
  /*! Prints formatted text to this stream.
   *  @param[in] text The formatting string.
   *  @return The number of characters actually written.
   */
  virtual size_t writeText(const String& text);
  /*! Prints the specified text to this stream.
   *  @param[in] format The formatting string.
   *  @return The number of characters actually written.
   */
  virtual size_t writeText(const char* format, ...);
  /*! Reads a specified number of characters from the stream.
   *  @param string [out] The read string.
   *  @param[in] count The number of characters to read.
   *  @return The number of characters actually read.
   */
  virtual size_t readText(String& string, size_t count);
  /*! Prints a line of text to the stream.
   *  @param[in] format The formatting string.
   *  @return @c true if successful, otherwise @c false.
   */
  virtual bool writeLine(const char* format, ...);
  /*! Reads a line of text from the stream.
   *  @param[out] line The read line of text.
   *  @return @c true if successful, otherwise @c false.
   */
  virtual bool readLine(String& line);
  size_t read(void* data, size_t size);
  size_t write(const void* data, size_t size);
  void flush(void);
  bool isEOF(void) const;
  bool isReadable(void) const;
  bool isWritable(void) const;
  bool isSeekable(void) const;
  off64_t getSize(void) const;
  off64_t getPosition(void) const;
  bool setPosition(off64_t position);
  /*! Helper method. Creates a text stream on the specified stream, or returns @c NULL if given @c NULL.
   *  @param[in] stream The regular stream to wrap, or @c NULL to return @c NULL.
   *  @param[in] owner @c true to take ownership of the specified stream.
   *  @return The newly created text stream, or @c NULL.
   */
  static TextStream* createInstance(Stream* stream, bool owner = true);
private:
  TextStream(const TextStream& source);
  TextStream& operator = (const TextStream& source);
  Stream& stream;
  bool owner;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Memory block stream.
 */
class BlockStream : public Stream
{
public:
  /*! Default constructor.
   */
  BlockStream(void);
  /*! Constructor. Creates a block stream with the specified initial data.
   *  @param[in] data The base address of the data to import.
   *  @param[in] size The size, in bytes, of the data.
   */
  BlockStream(const void* data, size_t size);
  void* lock(void);
  void unlock(void);
  size_t read(void* data, size_t size);
  size_t write(const void* data, size_t size);
  void flush(void);
  bool isEOF(void) const;
  bool isReadable(void) const;
  bool isWritable(void) const;
  bool isSeekable(void) const;
  off64_t getSize(void) const;
  off64_t getPosition(void) const;
  bool setPosition(off64_t position);
private:
  BlockStream(const BlockStream& source);
  BlockStream& operator = (const BlockStream& source);
  Block m_data;
  size_t m_size;
  size_t m_position;
  unsigned int m_locks;
};

///////////////////////////////////////////////////////////////////////

/*! @brief GNU zip byte stream.
 */
class GzipStream : public Stream
{
public:
  ~GzipStream(void);
  size_t read(void* data, size_t size);
  size_t write(const void* data, size_t size);
  void flush(void);
  bool isEOF(void) const;
  bool isReadable(void) const;
  bool isWritable(void) const;
  bool isSeekable(void) const;
  off64_t getSize(void) const;
  off64_t getPosition(void) const;
  bool setPosition(off64_t position);
  static GzipStream* createInstance(const Path& path, unsigned int flags);
private:
  GzipStream(void);
  GzipStream(const GzipStream& source);
  GzipStream& operator = (const GzipStream& source);
  bool init(const Path& path, unsigned int flags);
  bool convertFlags(unsigned int flags, String& mode);
  void* file;
  unsigned int flags;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_STREAM_H*/
///////////////////////////////////////////////////////////////////////
