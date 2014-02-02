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

#include <wendy/Config.hpp>

#include <wendy/Core.hpp>
#include <wendy/Rect.hpp>
#include <wendy/Path.hpp>
#include <wendy/Pixel.hpp>
#include <wendy/Resource.hpp>
#include <wendy/Image.hpp>

#include <fstream>

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
  switch (format.type())
  {
    case PixelFormat::UINT8:
    case PixelFormat::UINT16:
      result = format.size() * 8;
      return false;
    default:
      return false;
  }
}

bool convertToColorType(int& result, const PixelFormat& format)
{
  switch (format.semantic())
  {
    case PixelFormat::L:
      result = PNG_COLOR_TYPE_GRAY;
      return true;
    case PixelFormat::LA:
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
      return PixelFormat::L;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      return PixelFormat::LA;
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

const uint IMAGE_CUBE_XML_VERSION = 2;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

bool Image::transformTo(const PixelFormat& format, PixelTransform& transform)
{
  if (m_format == format)
    return true;

  if (!transform.supports(format, m_format))
    return false;

  std::vector<char> temp(m_width * m_height * m_depth * format.size());
  transform.convert(&temp[0], format, &m_data[0], m_format, m_width * m_height * m_depth);
  std::swap(m_data, temp);

  m_format = format;
  return true;
}

bool Image::crop(const Recti& area)
{
  if (dimensionCount() > 2)
  {
    logError("Cannot 2D crop 3D image");
    return false;
  }

  if (!Recti(0, 0, m_width, m_height).contains(area))
  {
    logError("Cropping area must be entirely within image");
    return false;
  }

  const size_t pixelSize = m_format.size();
  std::vector<char> temp(area.size.x * area.size.y * pixelSize);

  for (size_t y = 0;  y < (size_t) area.size.y;  y++)
  {
    std::memcpy(&temp[0] + y * area.size.x * pixelSize,
                &m_data[0] + ((y + area.position.y) * m_width + area.position.x) * pixelSize,
                area.size.x * pixelSize);
  }

  m_width = area.size.x;
  m_height = area.size.y;

  std::swap(m_data, temp);
  return true;
}

void Image::flipHorizontal()
{
  const size_t rowSize = m_width * m_format.size();
  std::vector<char> temp(m_data.size());

  for (size_t z = 0;  z < m_depth;  z++)
  {
    const size_t sliceOffset = z * m_height * rowSize;

    for (size_t y = 0;  y < m_height;  y++)
    {
      std::memcpy(&temp[0] + sliceOffset + rowSize * (m_height - y - 1),
                  &m_data[0] + sliceOffset + rowSize * y,
                  rowSize);
    }
  }

  std::swap(m_data, temp);
}

void Image::flipVertical()
{
  const size_t pixelSize = m_format.size();
  std::vector<char> temp(m_data.size());

  for (size_t z = 0;  z < m_depth;  z++)
  {
    for (size_t y = 0;  y < m_height;  y++)
    {
      const char* source = &m_data[0] + (z * m_height + y) * m_width * pixelSize;
      char* target = &temp[0] + ((z * m_height + y + 1) * m_width - 1) * pixelSize;

      while (source < target)
      {
        std::memcpy(target, source, pixelSize);
        source += pixelSize;
        target -= pixelSize;
      }
    }
  }

  std::swap(m_data, temp);
}

bool Image::isPOT() const
{
  return isPowerOfTwo(m_width) && isPowerOfTwo(m_height) && isPowerOfTwo(m_depth);
}

void* Image::pixel(uint x, uint y, uint z)
{
  if (x >= m_width || y >= m_height || z >= m_depth)
    return nullptr;

  return &m_data[0] + ((z * m_height + y) * m_width + x) * m_format.size();
}

const void* Image::pixel(uint x, uint y, uint z) const
{
  if (x >= m_width || y >= m_height || z >= m_depth)
    return nullptr;

  return &m_data[0] + ((z * m_height + y) * m_width + x) * m_format.size();
}

uint Image::dimensionCount() const
{
  if (m_depth > 1)
    return 3;

  if (m_height > 1)
    return 2;

  return 1;
}

Ref<Image> Image::area(const Recti& area) const
{
  if (dimensionCount() > 2)
  {
    logError("Cannot retrieve area of 3D image");
    return nullptr;
  }

  if (!Recti(0, 0, m_width, m_height).contains(area))
  {
    logError("Cannot retrieve area outside of image");
    return nullptr;
  }

  const size_t rowSize = area.size.x * m_format.size();
  Ref<Image> result = create(cache(), m_format, area.size.x, area.size.y);

  for (size_t y = 0;  y < (size_t) area.size.y;  y++)
  {
    std::memcpy(result->pixel(0, y),
                pixel(area.position.x, area.position.y + y),
                rowSize);
  }

  return result;
}

Ref<Image> Image::create(const ResourceInfo& info,
                         const PixelFormat& format,
                         uint width,
                         uint height,
                         uint depth,
                         const void* pixels,
                         ptrdiff_t pitch)
{
  Ref<Image> image(new Image(info));
  if (!image->init(format, width, height, depth, (const char*) pixels, pitch))
    return nullptr;

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

bool Image::init(const PixelFormat& format,
                 uint width,
                 uint height,
                 uint depth,
                 const char* pixels,
                 ptrdiff_t pitch)
{
  m_format = format;
  m_width = width;
  m_height = height;
  m_depth = depth;

  if (!m_format.isValid())
  {
    logError("Cannot create image with invalid pixel format");
    return false;
  }

  if (!m_width || !m_height || !m_depth)
  {
    logError("Cannot create image with zero size in any dimension");
    return false;
  }

  if ((m_height > 1) && (m_width == 1))
  {
    m_width = m_height;
    m_height = 1;
  }

  if ((m_depth > 1) && (m_height == 1))
  {
    m_height = m_depth;
    m_depth = 1;
  }

  if (pixels)
  {
    if (pitch)
    {
      const size_t pixelSize = m_format.size();
      m_data.resize(m_width * m_height * m_depth * pixelSize);

      char* target = &m_data[0];
      const char* source = pixels;

      for (size_t z = 0;  z < m_depth;  z++)
      {
        for (size_t y = 0;  y < m_height;  y++)
        {
          std::memcpy(target, source, m_width * pixelSize);
          source += pitch;
          target += m_width * pixelSize;
        }
      }
    }
    else
      m_data.assign(pixels, pixels + m_width * m_height * m_depth * m_format.size());
  }
  else
    m_data.resize(m_width * m_height * m_depth * m_format.size(), 0);

  return true;
}

///////////////////////////////////////////////////////////////////////

ImageReader::ImageReader(ResourceCache& cache):
  ResourceReader<Image>(cache)
{
}

Ref<Image> ImageReader::read(const String& name, const Path& path)
{
  std::ifstream stream(path.name(), std::ios::in | std::ios::binary);
  if (stream.fail())
  {
    logError("Failed to open image file %s", path.name().c_str());
    return nullptr;
  }

  // Check if file is valid
  {
    unsigned char header[8];

    if (!stream.read((char*) header, sizeof(header)))
    {
      logError("Failed to read PNG header from image %s", name.c_str());
      return nullptr;
    }

    if (png_sig_cmp(header, 0, sizeof(header)))
    {
      logError("Invalid PNG signature in image %s", name.c_str());
      return nullptr;
    }
  }

  png_structp context;
  png_infop pngInfo;
  png_infop pngEndInfo;

  // Set up for image reading
  {
    context = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                     nullptr,
                                     writeErrorPNG,
                                     writeWarningPNG);
    if (!context)
    {
      logError("Failed to create PNG read struct for image %s",
               name.c_str());
      return nullptr;
    }

    png_set_read_fn(context, &stream, readStreamPNG);

    pngInfo = png_create_info_struct(context);
    if (!pngInfo)
    {
      png_destroy_read_struct(&context, nullptr, nullptr);

      logError("Failed to create PNG info struct for image %s",
               name.c_str());
      return nullptr;
    }

    pngEndInfo = png_create_info_struct(context);
    if (!pngEndInfo)
    {
      png_destroy_read_struct(&context, &pngInfo, nullptr);

      logError("Failed to create PNG end info struct for image %s",
               name.c_str());
      return nullptr;
    }

    png_set_sig_bytes(context, 8);

    png_read_png(context, pngInfo, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, nullptr);
  }

  const PixelFormat format = convertToPixelFormat(png_get_color_type(context, pngInfo),
                                                  png_get_bit_depth(context, pngInfo));

  if (!format.isValid())
  {
    png_destroy_read_struct(&context, &pngInfo, &pngEndInfo);

    logError("Image %s has unsupported pixel format", name.c_str());
    return nullptr;
  }

  const uint width  = png_get_image_width(context, pngInfo);
  const uint height = png_get_image_height(context, pngInfo);
  const ResourceInfo info(cache, name, path);

  Ref<Image> result = Image::create(info, format, width, height);

  // Read image data
  {
    const size_t rowSize = png_get_rowbytes(context, pngInfo);
    png_byte** rows = png_get_rows(context, pngInfo);

    for (size_t i = 0;  i < height;  i++)
      std::memcpy(result->pixel(0, height - i - 1), rows[i], rowSize);
  }

  // Clean up library structures
  png_destroy_read_struct(&context, &pngInfo, &pngEndInfo);

  return result;
}

///////////////////////////////////////////////////////////////////////

bool ImageWriter::write(const Path& path, const Image& image)
{
  if (image.dimensionCount() > 2)
  {
    logError("Cannot write 3D images to PNG file");
    return false;
  }

  std::ofstream stream(path.name());
  if (!stream.is_open())
  {
    logError("Failed to create image file %s", path.name().c_str());
    return false;
  }

  png_structp context = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                nullptr,
                                                writeErrorPNG,
                                                writeWarningPNG);
  if (!context)
  {
    logError("Failed to create PNG write struct for image file %s",
             path.name().c_str());
    return false;
  }

  png_set_write_fn(context, &stream, writeStreamPNG, flushStreamPNG);
  png_set_filter(context, 0, PNG_FILTER_NONE);

  png_infop info = png_create_info_struct(context);
  if (!info)
  {
    png_destroy_write_struct(&context, png_infopp(nullptr));
    logError("Failed to create PNG info struct for image file %s",
             path.name().c_str());
    return false;
  }

  const PixelFormat& format = image.format();

  int colorType, bitDepth;

  if (!convertToColorType(colorType, format) ||
      !convertToBitDepth(bitDepth, format))
  {
    png_destroy_write_struct(&context, &info);
    logError("Failed to write image %s: pixel format %s is not supported by the PNG format",
             image.name().c_str(),
             format.asString().c_str());
    return false;
  }

  png_set_IHDR(context,
               info,
               image.width(),
               image.height(),
               bitDepth,
               colorType,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  std::vector<const png_byte*> rows(image.height());

  for (size_t i = 0;  i < image.height();  i++)
    rows[i] = (const png_byte*) image.pixel(0, image.height() - i - 1);

  png_set_rows(context, info, const_cast<png_byte**>(&rows[0]));
  png_write_png(context, info, PNG_TRANSFORM_IDENTITY, nullptr);
  png_destroy_write_struct(&context, &info);

  return true;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
