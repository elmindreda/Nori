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
#ifndef WENDY_BLOCK_H
#define WENDY_BLOCK_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief Heap memory byte block container.
 */
class Block
{
public:
  /*! Default constructor.
   *  @param size The initial size, in bytes, of this data block.
   */
  explicit Block(size_t size = 0);
  /*! Constructor. Copies the specified number of bytes into this block
   *  @param source The data to copy into this block.
   *  @param sourceSize The size, in bytes, of the data.
   */
  Block(const Byte* source, size_t sourceSize);
  /*! Copy constructor.
   *  @remarks Performs a deep copy of data items.
   */
  Block(const Block& source);
  /*! Destructor.
   */
  ~Block(void);
  /*! Copies the specified number of bytes from this block, starting
   *  at the specified offset.
   *  @param target The target buffer for the data to be copied.
   *  @param targetSize The number of bytes to copy.
   *  @param offset The desired offset within this data block, in bytes,
   *  from which to start.
   */
  void copyTo(Byte* target, size_t targetSize, size_t offset = 0) const;
  /*! Copies the specified number of bytes into this block, starting
   *  at the specified offset.
   *  @param source The source buffer from which to copy.
   *  @param sourceSize The number of bytes to copy.
   *  @param offset The desired offset within this data block, in bytes,
   *  from which to start.
   */
  void copyFrom(const Byte* source, size_t sourceSize, size_t offset = 0);
  /*! Changes the number of bytes in this data block.
   *  @param newSize The new size, in bytes, of this data block.
   */
  void resize(size_t newSize);
  /*! Assures that the size of this block is at least the specified
   *  number of bytes.
   *  @param minSize The minimum desired size, in bytes.
   */
  void reserve(size_t minSize);
  void attach(Byte* newData, size_t newSize);
  Byte* detach(void);
  /*! Frees the currently allocated data block.
   */
  void destroy(void);
  /*! Cast operator.
   */
  operator Byte* (void);
  /*! Cast operator.
   */
  operator const Byte* (void) const;
  /*! Assignment operator.
   */
  Block& operator = (const Block& source);
  /*! @return The size, in bytes, of this data block.
   */
  size_t getSize(void) const;
  /*! @return The allocation granularity, or zero if granularity is disabled.
   */
  size_t getGrain(void) const;
  /*! Sets or disables the allocation granularity.
   *  @param newGrain The desired allocation granularity, or zero to disable.
   */
  void setGrain(size_t newGrain);
  /*! @return The data block.
   */
  Byte* getData(void);
  /*! @return The first item.
   */
  const Byte* getData(void) const;
private:
  size_t size;
  size_t grain;
  Byte* data;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_BLOCK_H*/
///////////////////////////////////////////////////////////////////////
