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
#ifndef WENDY_IMAGE_H
#define WENDY_IMAGE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief Container for one- or two-dimensional pixel data.
 */
class Image : public Resource
{
public:
  /*! Transforms the contents of this image to the specified pixel format using
   *  the specified pixel transform.
   *  @param[in] targetFormat The desired pixel format.
   *  @param[in] transform The pixel transform to use.
   */
  bool transformTo(const PixelFormat& targetFormat, PixelTransform& transform);
  /*! Sets this image to the specified area of the current image data.
   *  @param[in] area The desired area.
   *  @return @c true if successful, otherwise @c false.
   *
   *  @remarks This method fails if the desired area is partially or completely
   *  outside the current image data.
   */
  bool crop(const Recti& area);
  /*! Flips this image along the x axis.
   */
  void flipHorizontal();
  /*! Flips this image along the y axis.
   */
  void flipVertical();
  /*! @return @c true if this image has power-of-two dimensions, otherwise @c false.
   */
  bool isPOT() const;
  /*! @return @c true if this image is square, otherwise @c false.
   */
  bool isSquare() const;
  /*! @return The width, in pixels, of this image.
   */
  unsigned int getWidth() const;
  /*! @return The height, in pixels, of this image.
   */
  unsigned int getHeight() const;
  /*! @return The depth, in pixels, of this image.
   */
  unsigned int getDepth() const;
  /*! @return The base address of the pixel data for this image.
   */
  void* getPixels();
  /*! @return The base address of the pixel data for this image.
   */
  const void* getPixels() const;
  /*! Helper method to calculate the address of the specified pixel.
   *  @param[in] x The x coordinate of the desired pixel.
   *  @param[in] y The y coordinate of the desired pixel.
   *  @param[in] z The z coordinate of the desired pixel.
   *
   *  @return The address of the desired pixel, or @c NULL if the specified
   *  coordinates are outside of the current image data.
   */
  void* getPixel(unsigned int x, unsigned int y, unsigned int z = 0);
  /*! Helper method to calculate the address of the specified pixel.
   *  @param[in] x The x coordinate of the desired pixel.
   *  @param[in] y The y coordinate of the desired pixel.
   *  @param[in] z The z coordinate of the desired pixel.
   *  @return The address of the desired pixel, or @c NULL if the specified
   *  coordinates are outside of the current image data.
   */
  const void* getPixel(unsigned int x, unsigned int y, unsigned int z = 0) const;
  /*! @return The pixel format of this image.
   */
  const PixelFormat& getFormat() const;
  /*! @return The number of dimensions (that differ from 1) in this image.
   */
  unsigned int getDimensionCount() const;
  /*! Returns an image containing the specified area of this image.
   *  @param area The desired area of this image.
   *
   *  @remarks This method fails if the desired area is partially or completely
   *  outside the current image data.
   */
  Ref<Image> getArea(const Recti& area) const;
  /*! Creates an image with the specified properties.
   *  @param[in] info The resource information for this image.
   *  @param[in] format The desired format of the image.
   *  @param[in] width The desired width of the image. This cannot be zero.
   *  @param[in] height The desired height of the image. This cannot be zero.
   *  @param[in] depth The desired depth of the image. This cannot be zero.
   *  @param[in] data The pixel data to initialize the image with, or @c NULL
   *  to initialize it with zeros.
   *  @param[in] pitch The pitch, in bytes, between consecutive scanlines, or
   *  zero if the scanlines are contiguous in memory. If @c data is @c NULL,
   *  then this parameter is ignored.
   *  @return The newly created image object.
   *
   *  @remarks No, you cannot create an empty image object.
   */
  static Ref<Image> create(const ResourceInfo& info,
                           const PixelFormat& format,
                           unsigned int width,
                           unsigned int height = 1,
                           unsigned int depth = 1,
                           const void* data = NULL,
                           ptrdiff_t pitch = 0);
  static Ref<Image> read(ResourceCache& cache, const String& name);
private:
  Image(const ResourceInfo& info);
  Image(const Image& source);
  bool init(const PixelFormat& format,
            unsigned int width,
            unsigned int height,
            unsigned int depth,
            const void* data,
            ptrdiff_t pitch);
  Image& operator = (const Image& source);
  unsigned int width;
  unsigned int height;
  unsigned int depth;
  PixelFormat format;
  Block data;
};

///////////////////////////////////////////////////////////////////////

class ImageReader : public ResourceReader<Image>
{
public:
  ImageReader(ResourceCache& cache);
  using ResourceReader::read;
  Ref<Image> read(const String& name, const Path& path);
};

///////////////////////////////////////////////////////////////////////

class ImageWriter
{
public:
  bool write(const Path& path, const Image& image);
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_IMAGE_H*/
///////////////////////////////////////////////////////////////////////
