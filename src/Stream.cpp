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

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Block.h>
#include <wendy/Path.h>
#include <wendy/Stream.h>

#if WENDY_HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if WENDY_HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32

#if WENDY_HAVE_DIRECT_H
#include <direct.h>
#endif

#endif /*_WIN32*/

#include <zlib.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Stream::~Stream(void)
{
}

///////////////////////////////////////////////////////////////////////

PosixStream::~PosixStream(void)
{
  if (file)
    std::fclose(file);
}

size_t PosixStream::read(void* data, size_t size)
{
  if (isSeekable())
    std::fseek(file, 0, SEEK_CUR);

  const size_t bytesRead = std::fread(data, 1, size, file);

  if (std::ferror(file))
  {
    // TODO: report error!
  }

  return bytesRead;
}

size_t PosixStream::write(const void* data, size_t size)
{
  if (isSeekable())
    std::fseek(file, 0, SEEK_CUR);

  const size_t bytesWritten = std::fwrite(data, 1, size, file);

  if (std::ferror(file))
  {
    // TODO: report error!
  }

  return bytesWritten;
}

void PosixStream::flush(void)
{
  std::fflush(file);
}

bool PosixStream::isEOF(void) const
{
  return std::feof(file) != 0;
}

bool PosixStream::isReadable(void) const
{
  return (flags & READABLE) ? true : false;
}

bool PosixStream::isWritable(void) const
{
  return (flags & WRITABLE) ? true : false;
}

off64_t PosixStream::getSize(void) const
{
#ifdef _MSC_VER
  struct _stati64 sb;

  if (_fstati64(fileno(file), &sb) != 0)
    return 0;
#else
  struct stat64 sb;

  if (fstat64(fileno(file), &sb) != 0)
    return 0;
#endif

  return sb.st_size;
}

off64_t PosixStream::getPosition(void) const
{
  const off64_t position = std::ftell(file);

  if (position == (off64_t) -1)
  {
    Log::writeWarning("Failed to retrieve file position: %s", strerror(errno));
    return 0;
  }

  return position;
}

bool PosixStream::setPosition(off64_t position)
{
  if (!isSeekable())
    return false;

  if (std::fseek(file, (size_t) position, SEEK_SET) != 0)
  {
    Log::writeWarning("Failed to set file position: %s", strerror(errno));
    return false;
  }

  return true;
}

PosixStream::PosixStream(void):
  file(NULL),
  flags(0)
{
}

PosixStream::PosixStream(const PosixStream& source)
{
  // NOTE: Not implemented.
}

bool PosixStream::convertFlags(unsigned int flags, String& mode)
{
  if ((flags & (READABLE | WRITABLE)) == 0)
    return false;

  if (flags & WRITABLE)
  {
    if (flags & OVERWRITE)
    {
      mode.append("wb");
      if (flags & READABLE)
        mode.append("+");
    }
    else
      mode.append("rb+");
  }
  else
    mode.append("rb");

  return true;
}

PosixStream& PosixStream::operator = (const PosixStream& source)
{
  // NOTE: Not implemented.

  return *this;
}

///////////////////////////////////////////////////////////////////////

bool FileStream::isSeekable(void) const
{
  return true;
}

const Path& FileStream::getPath(void) const
{
  return path;
}

FileStream* FileStream::createInstance(const Path& path,
                                       unsigned int flags)
{
  Ptr<FileStream> stream(new FileStream());
  if (!stream->init(path, flags))
    return NULL;

  return stream.detachObject();
}

FileStream::FileStream(void)
{
}

bool FileStream::init(const Path& initPath, unsigned int initFlags)
{
  path = initPath;
  flags = initFlags;

  String mode;
  if (!convertFlags(flags, mode))
    return false;

  file = std::fopen(path.asString().c_str(), mode.c_str());
  if (!file)
  {
    Log::writeError("Failed to open file %s: %s",
                    path.asString().c_str(),
		    strerror(errno));
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

TextStream::TextStream(Stream& initStream, bool initOwner):
  stream(initStream),
  owner(initOwner)
{
}

TextStream::~TextStream(void)
{
  if (owner)
    delete &stream;
}

size_t TextStream::writeText(const String& text)
{
  return write(text.c_str(), text.size());
}

size_t TextStream::writeText(const char* format, ...)
{
  va_list vl;
  char* text;
  size_t size;

  va_start(vl, format);
  size = vasprintf(&text, format, vl);
  va_end(vl);

  if (size > 0)
    size = write(text, size);

  std::free(text);
  return size;
}

size_t TextStream::readText(String& string, size_t count)
{
  Block buffer(count);

  count = read(buffer.getData(), count);
  string.assign((const char*) buffer.getData(), count);
  return count;
}

bool TextStream::writeLine(const char* format, ...)
{
  char* text;
  size_t size;

  va_list vl;
  va_start(vl, format);
  size = vasprintf(&text, format, vl);
  va_end(vl);

  if (size > 0)
  {
    if (write(text, size) < size)
    {
      std::free(text);
      return false;
    }
  }

  std::free(text);

  const char newline = '\n';
  if (!writeItem(newline))
    return false;

  return true;
}

bool TextStream::readLine(String& line)
{
  char c;
  String buffer;

  while (readItem(c))
  {
    if (c == '\n')
    {
      line = buffer;
      return true;
    }

    buffer.append(1, c);
  }

  if (!buffer.empty())
  {
    line = buffer;
    return true;
  }

  return false;
}

size_t TextStream::read(void* data, size_t size)
{
  return stream.read(data, size);
}

size_t TextStream::write(const void* data, size_t size)
{
  return stream.write(data, size);
}

void TextStream::flush(void)
{
  stream.flush();
}

bool TextStream::isEOF(void) const
{
  return stream.isEOF();
}

bool TextStream::isReadable(void) const
{
  return stream.isReadable();
}

bool TextStream::isWritable(void) const
{
  return stream.isWritable();
}

bool TextStream::isSeekable(void) const
{
  return stream.isSeekable();
}

off64_t TextStream::getSize(void) const
{
  return stream.getSize();
}

off64_t TextStream::getPosition(void) const
{
  return stream.getPosition();
}

bool TextStream::setPosition(off64_t position)
{
  return stream.setPosition(position);
}

TextStream* TextStream::createInstance(Stream* stream, bool owner)
{
  if (!stream)
    return NULL;

  return new TextStream(*stream, owner);
}

TextStream::TextStream(const TextStream& source):
  stream(source.stream),
  owner(false)
{
  // NOTE: Not implemented.
}

TextStream& TextStream::operator = (const TextStream& source)
{
  // NOTE: Not implemented.

  return *this;
}

///////////////////////////////////////////////////////////////////////

BlockStream::BlockStream(void):
  m_size(0),
  m_position(0),
  m_locks(0)
{
  m_data.setGrain(1024);
  m_data.reserve(1);
}

BlockStream::BlockStream(const void* data, size_t size):
  m_size(size),
  m_position(0),
  m_locks(0)
{
  m_data.setGrain(1024);
  m_data.copyFrom((const Byte*) data, size);
}

void* BlockStream::lock(void)
{
  m_locks++;

  return m_data;
}

void BlockStream::unlock(void)
{
  if (m_locks)
    m_locks--;
}

size_t BlockStream::read(void* data, size_t size)
{
  if (m_locks || isEOF())
    return 0;

  if (m_position + size > m_size)
    size = m_size - m_position;

  m_data.copyTo((Byte*) data, size, m_position);

  m_position += (unsigned int) size;
  return size;
}

size_t BlockStream::write(const void* data, size_t size)
{
  if (m_position + size > m_data.getSize())
  {
    m_data.resize(m_position + size);
    m_size = m_position + size;
  }

  m_data.copyFrom((const Byte*) data, size, m_position);

  m_position += size;
  return size;
}

void BlockStream::flush(void)
{
  // NOTE: This space is intentionally left blank.
}

bool BlockStream::isEOF(void) const
{
  if (m_position < m_size)
    return false;

  return true;
}

bool BlockStream::isReadable(void) const
{
  return true;
}

bool BlockStream::isWritable(void) const
{
  return true;
}

bool BlockStream::isSeekable(void) const
{
  return true;
}

off64_t BlockStream::getSize(void) const
{
  return (off64_t) m_size;
}

off64_t BlockStream::getPosition(void) const
{
  return (off64_t) m_position;
}

bool BlockStream::setPosition(off64_t position)
{
  m_position = (size_t) position;
  return true;
}

BlockStream::BlockStream(const BlockStream& source)
{
  // NOTE: Not implemented.
}

BlockStream& BlockStream::operator = (const BlockStream& source)
{
  // NOTE: Not implemented.

  return *this;
}

///////////////////////////////////////////////////////////////////////

GzipStream::~GzipStream(void)
{
  if (file)
    gzclose(file);
}

size_t GzipStream::read(void* data, size_t size)
{
  if (!isReadable())
    return 0;

  gzseek(file, 0, SEEK_CUR);
  return gzread(file, data, (unsigned int) size);
}

size_t GzipStream::write(const void* data, size_t size)
{
  if (!isWritable())
    return 0;

  gzseek(file, 0, SEEK_CUR);
  return gzwrite(file, data, (unsigned int) size);
}

void GzipStream::flush(void)
{
  gzflush(file, Z_SYNC_FLUSH);
}

bool GzipStream::isEOF(void) const
{
  return gzeof(file) ? true : false;
}

bool GzipStream::isReadable(void) const
{
  return (flags & READABLE) ? true : false;
}

bool GzipStream::isWritable(void) const
{
  return (flags & WRITABLE) ? true : false;
}

bool GzipStream::isSeekable(void) const
{
  return true;
}

off64_t GzipStream::getSize(void) const
{
  // NOTE: This is very unsafe.

  off64_t position = gztell(file);

  gzseek(file, 0, SEEK_END);
  off64_t size = gztell(file);
  gzseek(file, (size_t) position, SEEK_SET);

  return size;
}

off64_t GzipStream::getPosition(void) const
{
  const off64_t position = gztell(file);

  if (position == (off64_t) -1)
  {
    int error;
    Log::writeWarning("Failed to retrieve file position: %s",
                      gzerror(file, &error));
    return 0;
  }

  return position;
}

bool GzipStream::setPosition(off64_t position)
{
  if (gzseek(file, (size_t) position, SEEK_SET) == (off64_t) -1)
  {
    int error;
    Log::writeWarning("Failed to set file position: %s",
                      gzerror(file, &error));
    return false;
  }

  return true;
}

GzipStream* GzipStream::createInstance(const Path& path, unsigned int flags)
{
  Ptr<GzipStream> stream(new GzipStream());
  if (!stream->init(path, flags))
    return NULL;

  return stream.detachObject();
}

GzipStream::GzipStream(void):
  file(NULL),
  flags(0)
{
}

GzipStream::GzipStream(const GzipStream& source)
{
  // NOTE: Not implemented.
}

GzipStream& GzipStream::operator = (const GzipStream& source)
{
  // NOTE: Not implemented.

  return *this;
}

bool GzipStream::init(const Path& path, unsigned int initFlags)
{
  flags = initFlags;

  String mode;
  if (!convertFlags(flags, mode))
    return false;

  file = gzopen(path.asString().c_str(), mode.c_str());
  if (!file)
    return false;

  return true;
}

bool GzipStream::convertFlags(unsigned int flags, String& mode)
{
  if ((flags & (READABLE | WRITABLE)) == 0)
    return false;

  if (flags & WRITABLE)
    mode.append("wb");
  else
    mode.append("rb");

  return true;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
