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
  Block(const uint8* source, size_t sourceSize);
  /*! Copy constructor.
   *  @remarks Performs a deep copy of data items.
   */
  Block(const Block& source);
  /*! Destructor.
   */
  ~Block();
  /*! Copies the specified number of bytes from this block, starting
   *  at the specified offset.
   *  @param target The target buffer for the data to be copied.
   *  @param targetSize The number of bytes to copy.
   *  @param offset The desired offset within this data block, in bytes,
   *  from which to start.
   */
  void copyTo(uint8* target, size_t targetSize, size_t offset = 0) const;
  /*! Copies the specified number of bytes into this block, starting
   *  at the specified offset.
   *  @param source The source buffer from which to copy.
   *  @param sourceSize The number of bytes to copy.
   *  @param offset The desired offset within this data block, in bytes,
   *  from which to start.
   */
  void copyFrom(const uint8* source, size_t sourceSize, size_t offset = 0);
  /*! Changes the number of bytes in this data block.
   *  @param newSize The new size, in bytes, of this data block.
   */
  void resize(size_t newSize);
  void attach(uint8* newData, size_t newSize);
  uint8* detach();
  /*! Frees the currently allocated data block.
   */
  void destroy();
  /*! Cast operator.
   */
  operator uint8* ();
  /*! Cast operator.
   */
  operator const uint8* () const;
  /*! Assignment operator.
   */
  Block& operator = (const Block& source);
  /*! @return The size, in bytes, of this data block.
   */
  size_t getSize() const;
  /*! @return The data block.
   */
  uint8* getData();
  /*! @return The first item.
   */
  const uint8* getData() const;
private:
  size_t size;
  uint8* data;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_BLOCK_H*/
///////////////////////////////////////////////////////////////////////
