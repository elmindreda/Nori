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
// 1. The origin of this software must not be misrepresented; you
//    must not claim that you wrote the original software. If you use
//    this software in a product, an acknowledgment in the product
//    documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and
//    must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
///////////////////////////////////////////////////////////////////////

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Block.h>
#include <wendy/Rectangle.h>
#include <wendy/Path.h>
#include <wendy/Pixel.h>
#include <wendy/Resource.h>
#include <wendy/Image.h>

#include <cstring>

#include <pugixml.hpp>

#include <glm/gtx/bit.hpp>

#include <png.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

bool convertToBitDepth(int& result, const PixelFormat& format)
{
  switch (format.getType())
  {
    case PixelFormat::UINT8:
    case PixelFormat::UINT16:
      result = format.getSize() * 8;
      return false;
    default:
      return false;
  }
}

bool convertToColorType(int& result, const PixelFormat& format)
{
  switch (format.getSemantic())
  {
    case PixelFormat::R:
      result = PNG_COLOR_TYPE_GRAY;
      return true;
    case PixelFormat::RG:
      result = PNG_COLOR_TYPE_GRAY_ALPHA;
      return true;
    case PixelFormat::RGB:
      result = PNG_COLOR_TYPE_RGB;
      return true;
    case PixelFormat::RGBA:
      result = PNG_COLOR_TYPE_RGB_ALPHA;
      return true;
    default:
      return false;
  }
}

PixelFormat::Semantic convertToSemantic(int colorType)
{
  switch (colorType)
  {
    case PNG_COLOR_TYPE_GRAY:
      return PixelFormat::R;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      return PixelFormat::RG;
    case PNG_COLOR_TYPE_RGB:
      return PixelFormat::RGB;
    case PNG_COLOR_TYPE_RGB_ALPHA:
      return PixelFormat::RGBA;
  }

  return PixelFormat::NONE;
}

PixelFormat::Type convertToType(int bitDepth)
{
  switch (bitDepth)
  {
    case 8:
      return PixelFormat::UINT8;
    case 16:
      return PixelFormat::UINT16;
  }

  return PixelFormat::DUMMY;
}

PixelFormat convertToPixelFormat(int colorType, int bitDepth)
{
  return PixelFormat(convertToSemantic(colorType), convertToType(bitDepth));
}

void writeErrorPNG(png_structp context, png_const_charp error)
{
  logError("libpng error: %s", error);
}

void writeWarningPNG(png_structp context, png_const_charp warning)
{
  logWarning("libpng warning: %s", warning);
}

void readStreamPNG(png_structp context, png_bytep data, png_size_t length)
{
  std::ifstream* stream = reinterpret_cast<std::ifstream*>(png_get_io_ptr(context));
  stream->read((char*) data, length);
}

void writeStreamPNG(png_structp context, png_bytep data, png_size_t length)
{
  std::ofstream* stream = reinterpret_cast<std::ofstream*>(png_get_io_ptr(context));
  stream->write((char*) data, length);
}

void flushStreamPNG(png_structp context)
{
  std::ofstream* stream = reinterpret_cast<std::ofstream*>(png_get_io_ptr(context));
  stream->flush();
}

const unsigned int IMAGE_CUBE_XML_VERSION = 2;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

bool Image::transformTo(const PixelFormat& targetFormat, PixelTransform& transform)
{
  if (format == targetFormat)
    return true;

  if (!transform.supports(targetFormat, format))
    return false;

  Block temp(width * height * depth * targetFormat.getSize());
  transform.convert(temp, targetFormat, data, format, width * height * depth);
  data.attach(temp.detach(), temp.getSize());
  format = targetFormat;
  return true;
}

bool Image::crop(const Recti& area)
{
  if (getDimensionCount() > 2)
  {
    logError("Cannot 2D crop 3D image");
    return false;
  }

  if (!Recti(0, 0, width, height).contains(area))
  {
    logError("Cropping area must be entirely within image");
    return false;
  }

  const size_t pixelSize = format.getSize();
  Block temp(area.size.x * area.size.y * pixelSize);

  for (size_t y = 0;  y < area.size.y;  y++)
  {
    std::memcpy(temp + y * area.size.x * pixelSize,
                data + ((y + area.position.y) * width + area.position.x) * pixelSize,
                area.size.x * pixelSize);
  }

  width = area.size.x;
  height = area.size.y;

  data.attach(temp.detach(), temp.getSize());
  return true;
}

void Image::flipHorizontal()
{
  const size_t rowSize = width * format.getSize();
  Block temp(data.getSize());

  for (size_t z = 0;  z < depth;  z++)
  {
    const size_t sliceOffset = z * height * rowSize;

    for (size_t y = 0;  y < height;  y++)
    {
      std::memcpy(temp + sliceOffset + rowSize * (height - y - 1),
                  data + sliceOffset + rowSize * y,
                  rowSize);
    }
  }

  data.attach(temp.detach(), temp.getSize());
}

void Image::flipVertical()
{
  const size_t pixelSize = format.getSize();
  Block temp(data.getSize());

  for (size_t z = 0;  z < depth;  z++)
  {
    for (size_t y = 0;  y < height;  y++)
    {
      const uint8* source = data + (z * height + y) * width * pixelSize;
      uint8* target = temp + ((z * height + y + 1) * width - 1) * pixelSize;

      while (source < target)
      {
        std::memcpy(target, source, pixelSize);
        source += pixelSize;
        target -= pixelSize;
      }
    }
  }

  data.attach(temp.detach(), temp.getSize());
}

bool Image::isPOT() const
{
  return isPowerOfTwo(width) && isPowerOfTwo(height) && isPowerOfTwo(depth);
}

bool Image::isSquare() const
{
  return width == height;
}

unsigned int Image::getWidth() const
{
  return width;
}

unsigned int Image::getHeight() const
{
  return height;
}

unsigned int Image::getDepth() const
{
  return depth;
}

void* Image::getPixels()
{
  return data;
}

const void* Image::getPixels() const
{
  return data;
}

void* Image::getPixel(unsigned int x, unsigned int y, unsigned int z)
{
  if (x >= width || y >= height || z >= depth)
    return NULL;

  return data + ((z * height + y) * width + x) * format.getSize();
}

const void* Image::getPixel(unsigned int x, unsigned int y, unsigned int z) const
{
  if (x >= width || y >= height || z >= depth)
    return NULL;

  return data + ((z * height + y) * width + x) * format.getSize();
}

const PixelFormat& Image::getFormat() const
{
  return format;
}

unsigned int Image::getDimensionCount() const
{
  if (depth > 1)
    return 3;

  if (height > 1)
    return 2;

  return 1;
}

Ref<Image> Image::getArea(const Recti& area) const
{
  if (getDimensionCount() > 2)
  {
    logError("Cannot retrieve area of 3D image");
    return NULL;
  }

  if (!Recti(0, 0, width, height).contains(area))
  {
    logError("Cannot retrieve area outside of image");
    return NULL;
  }

  const size_t rowSize = area.size.x * format.getSize();
  Ref<Image> result = create(cache, format, area.size.x, area.size.y);

  for (size_t y = 0;  y < area.size.y;  y++)
  {
    std::memcpy(result->getPixel(0, y),
                getPixel(area.position.x, area.position.y + y),
                rowSize);
  }

  return result;
}

Ref<Image> Image::create(const ResourceInfo& info,
                         const PixelFormat& format,
                         unsigned int width,
                         unsigned int height,
                         unsigned int depth,
                         const void* data,
                         ptrdiff_t pitch)
{
  Ref<Image> image(new Image(info));
  if (!image->init(format, width, height, depth, data, pitch))
    return NULL;

  return image;
}

Ref<Image> Image::read(ResourceCache& cache, const String& name)
{
  ImageReader reader(cache);
  return reader.read(name);
}

Image::Image(const ResourceInfo& info):
  Resource(info)
{
}

bool Image::init(const PixelFormat& initFormat,
                 unsigned int initWidth,
                 unsigned int initHeight,
                 unsigned int initDepth,
                 const void* initData,
                 ptrdiff_t pitch)
{
  format = initFormat;
  width = initWidth;
  height = initHeight;
  depth = initDepth;

  if (!format.isValid())
  {
    logError("Cannot create image with invalid pixel format");
    return false;
  }

  if (!width || !height || !depth)
  {
    logError("Cannot create image with zero size in any dimension");
    return false;
  }

  if ((height > 1) && (width == 1))
  {
    width = height;
    height = 1;
  }

  if ((depth > 1) && (height == 1))
  {
    height = depth;
    depth = 1;
  }

  if (initData)
  {
    if (pitch)
    {
      const size_t pixelSize = format.getSize();
      data.resize(width * height * depth * pixelSize);

      uint8* target = data;
      const uint8* source = (const uint8*) initData;

      for (size_t z = 0;  z < depth;  z++)
      {
        for (size_t y = 0;  y < height;  y++)
        {
          std::memcpy(target, source, width * pixelSize);
          source += pitch;
          target += width * pixelSize;
        }
      }
    }
    else
      std::memcpy(data, initData, width * height * depth * format.getSize());
  }
  else
  {
    const size_t size = width * height * depth * format.getSize();
    data.resize(size);
    std::memset(data, 0, size);
  }

  return true;
}

Image::Image(const Image& source):
  Resource(source)
{
  panic("Image objects may not be copied");
}

Image& Image::operator = (const Image& source)
{
  panic("Image objects may not be assigned");
}

///////////////////////////////////////////////////////////////////////

ImageReader::ImageReader(ResourceCache& cache):
  ResourceReader<Image>(cache)
{
}

Ref<Image> ImageReader::read(const String& name, const Path& path)
{
  std::ifstream stream(path.asString().c_str(), std::ios::in | std::ios::binary);
  if (stream.fail())
  {
    logError("Failed to open image file \'%s\'", path.asString().c_str());
    return NULL;
  }

  // Check if file is valid
  {
    unsigned char header[8];

    if (!stream.read((char*) header, sizeof(header)))
    {
      logError("Failed to read PNG header from image \'%s\'", name.c_str());
      return NULL;
    }

    if (png_sig_cmp(header, 0, sizeof(header)))
    {
      logError("Invalid PNG signature in image \'%s\'", name.c_str());
      return NULL;
    }
  }

  png_structp context;
  png_infop pngInfo;
  png_infop pngEndInfo;

  // Set up for image reading
  {
    context = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                     NULL,
                                     writeErrorPNG,
                                     writeWarningPNG);
    if (!context)
    {
      logError("Failed to create PNG read struct for image \'%s\'",
               name.c_str());
      return NULL;
    }

    png_set_read_fn(context, &stream, readStreamPNG);

    pngInfo = png_create_info_struct(context);
    if (!pngInfo)
    {
      png_destroy_read_struct(&context, NULL, NULL);

      logError("Failed to create PNG info struct for image \'%s\'",
               name.c_str());
      return NULL;
    }

    pngEndInfo = png_create_info_struct(context);
    if (!pngEndInfo)
    {
      png_destroy_read_struct(&context, &pngInfo, NULL);

      logError("Failed to create PNG end info struct for image \'%s\'",
               name.c_str());
      return NULL;
    }

    png_set_sig_bytes(context, 8);

    png_read_png(context, pngInfo, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);
  }

  const PixelFormat format = convertToPixelFormat(png_get_color_type(context, pngInfo),
                                                  png_get_bit_depth(context, pngInfo));

  if (!format.isValid())
  {
    png_destroy_read_struct(&context, &pngInfo, &pngEndInfo);

    logError("Image \'%s\' has unsupported pixel format", name.c_str());
    return NULL;
  }

  const unsigned int width  = png_get_image_width(context, pngInfo);
  const unsigned int height = png_get_image_height(context, pngInfo);
  const ResourceInfo info(cache, name, path);

  Ref<Image> result = Image::create(info, format, width, height);

  // Read image data
  {
    const size_t rowSize = png_get_rowbytes(context, pngInfo);
    png_byte** rows = png_get_rows(context, pngInfo);

    for (size_t i = 0;  i < height;  i++)
      std::memcpy(result->getPixel(0, height - i - 1), rows[i], rowSize);
  }

  // Clean up library structures
  png_destroy_read_struct(&context, &pngInfo, &pngEndInfo);

  return result;
}

///////////////////////////////////////////////////////////////////////

bool ImageWriter::write(const Path& path, const Image& image)
{
  if (image.getDimensionCount() > 2)
  {
    logError("Cannot write 3D images to PNG file");
    return false;
  }

  std::ofstream stream(path.asString().c_str());
  if (!stream.is_open())
  {
    logError("Failed to create image file \'%s\'", path.asString().c_str());
    return false;
  }

  png_structp context = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                NULL,
                                                writeErrorPNG,
                                                writeWarningPNG);
  if (!context)
  {
    logError("Failed to create PNG write struct for image file \'%s\'",
             path.asString().c_str());
    return false;
  }

  png_set_write_fn(context, &stream, writeStreamPNG, flushStreamPNG);
  png_set_filter(context, 0, PNG_FILTER_NONE);

  png_infop info = png_create_info_struct(context);
  if (!info)
  {
    png_destroy_write_struct(&context, png_infopp(NULL));
    logError("Failed to create PNG info struct for image file \'%s\'",
             path.asString().c_str());
    return false;
  }

  const PixelFormat& format = image.getFormat();

  int colorType, bitDepth;

  if (!convertToColorType(colorType, format) ||
      !convertToBitDepth(bitDepth, format))
  {
    png_destroy_write_struct(&context, &info);
    logError("Failed to write image \'%s\': pixel format \'%s\' is not supported by the PNG format",
             image.getName().c_str(),
             format.asString().c_str());
    return false;
  }

  png_set_IHDR(context,
               info,
               image.getWidth(),
               image.getHeight(),
               bitDepth,
               colorType,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  std::vector<const png_byte*> rows(image.getHeight());

  for (size_t i = 0;  i < image.getHeight();  i++)
    rows[i] = (const png_byte*) image.getPixel(0, image.getHeight() - i - 1);

  png_set_rows(context, info, const_cast<png_byte**>(&rows[0]));
  png_write_png(context, info, PNG_TRANSFORM_IDENTITY, NULL);
  png_destroy_write_struct(&context, &info);

  return true;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
