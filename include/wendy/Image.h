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

/*! @brief %Image data object.
 *
 *  This is the standard container for all forms of 1D and 2D image data.
 */
class Image : public Resource<Image>, public RefObject
{
public:
  enum Method
  {
    /*! Use nearest-neighbour sampling.
     */
    SAMPLE_NEAREST,
    /*! Use bilinear sampling.
     */
    SAMPLE_LINEAR,
    /*! Default sampling method.
     */
    SAMPLE_DEFAULT = SAMPLE_LINEAR,
  };
  /*! Constructor.
   *  @param format The desired format of the image.
   *  @param width The desired width of the image. This cannot be zero.
   *  @param height The desired height of the image. This cannot be zero.
   *  @param data The pixel data to initialize the image with, or @c NULL to
   *  initialize it with zeros.
   *  @param pitch The pitch, in bytes, between consecutive scanlines, or zero
   *  if the scanlines are contiguous in memory. If @c data is @c NULL, then
   *  this parameter is ignored.
   *  @param name The name of the image, or the empty string to automatically
   *  generate a name.
   *
   *  @remarks No, you cannot create an empty image object.
   */
  Image(const PixelFormat& format,
        unsigned int width,
        unsigned int height,
        const void* data = NULL,
        size_t pitch = 0,
        const String& name = "");
  /*! Copy constructor.
   */
  Image(const Image& source);
  /*! Changes the size of this image, resampling its pixel data using the
   * specified filter method.
   *  @return @c true if successful, otherwise @c false.
   */
  bool resize(unsigned int newWidth,
              unsigned int newHeight,
              Method method = SAMPLE_DEFAULT);
  /*! Transforms the contents of this image to the specified pixel format using
   *  the specified pixel transform.
   *  @param[in] targetFormat The desired pixel format.
   *  @param[in] transform The pixel transform to use.
   */
  void transformTo(const PixelFormat& targetFormat, PixelTransform& transform);
  /*! Converts this image to the specified channel data type.
   *  @param targetType The data type to convert to.
   */
  void convertTo(PixelFormat::Type targetType);
  /*! Sets this image to the specified area of the current image data.
   *  @param area The desired area.
   *  @return @c true if successful, otherwise @c false.
   *  @remarks This method fails if the desired area is completely outside the
   *  current image data.
   *  @remarks If the desired area is partially outside of the current image
   *  data, the area (and the resulting image) is cropped to fit the current
   *  (existing) image data.
   */
  bool crop(const Recti& area);
  /*! Flips this image along the x axis.
   */
  void flipHorizontal(void);
  /*! Flips this image along the y axis.
   */
  void flipVertical(void);
  /*! Assignment operator.
   */
  Image& operator = (const Image& source);
  /*! @return @c true if this image has power-of-two dimensions, otherwise @c false.
   */
  bool isPOT(void) const;
  /*! @return @c true if this image is square, otherwise @c false.
   */
  bool isSquare(void) const;
  /*! @return The width, in pixels, of this image.
   */
  unsigned int getWidth(void) const;
  /*! @return The height, in pixels, of this image.
   */
  unsigned int getHeight(void) const;
  /*! @return The base address of the pixel data for this image.
   */
  void* getPixels(void);
  /*! @return The base address of the pixel data for this image.
   */
  const void* getPixels(void) const;
  /*! Helper method to calculate the address of the specified pixel.
   *  @param x The x coordinate of the desired pixel.
   *  @param y The y coordinate of the desired pixel.
   *  @return The address of the desired pixel, or @c NULL if the specified
   *  coordinates are outside of the current image data.
   */
  void* getPixel(unsigned int x, unsigned int y);
  /*! Helper method to calculate the address of the specified pixel.
   *  @param x The x coordinate of the desired pixel.
   *  @param y The y coordinate of the desired pixel.
   *  @return The address of the desired pixel, or @c NULL if the specified
   *  coordinates are outside of the current image data.
   */
  const void* getPixel(unsigned int x, unsigned int y) const;
  /*! @return The pixel format of this image.
   */
  const PixelFormat& getFormat(void) const;
  /*! @return The number of dimensions (that differ from 1) in this image.
   */
  unsigned int getDimensionCount(void) const;
  /*! Returns an image containing the specified area of this image.
   *  @param area The desired area of this image.
   */
  Image* getArea(const Recti& area);
private:
  unsigned int width;
  unsigned int height;
  PixelFormat format;
  Block data;
};

///////////////////////////////////////////////////////////////////////

typedef Ref<Image> ImageRef;

///////////////////////////////////////////////////////////////////////

/*! @brief %Image cube object.
 */
class ImageCube : public Resource<ImageCube>
{
public:
  enum Face
  {
    POSITIVE_X,
    NEGATIVE_X,
    POSITIVE_Y,
    NEGATIVE_Y,
    POSITIVE_Z,
    NEGATIVE_Z,
  };
  ImageCube(const String& name = "");
  ImageCube* clone(void) const;
  /*! @return @c true if all images have power-of-two dimensions, otherwise @c false.
   */
  bool isPOT(void) const;
  /*! @return @c true if all images are square, otherwise @c false.
   */
  bool isSquare(void) const;
  /*! @return @c true if all images are present, otherwise @c false.
   */
  bool isComplete(void) const;
  /*! @return @c true if all images have the same format, otherwise @c false.
   */
  bool hasSameFormat(void) const;
  /*! @return @c true if all images have the same size, otherwise @c false.
   */
  bool hasSameSize(void) const;
  Ref<Image> images[6];
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_IMAGE_H*/
///////////////////////////////////////////////////////////////////////
