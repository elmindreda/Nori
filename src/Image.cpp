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

#include <png.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

bool getEncodeConversionFormatPNG(int& result, const PixelFormat& format)
{
  if (format.getType() != PixelFormat::UINT8)
    return false;

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

bool getDecodeConversionFormatPNG(PixelFormat& result, int format)
{
  switch (format)
  {
    case PNG_COLOR_TYPE_GRAY:
      result = PixelFormat::R8;
      return true;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      result = PixelFormat::RG8;
      return true;
    case PNG_COLOR_TYPE_RGB:
      result = PixelFormat::RGB8;
      return true;
    case PNG_COLOR_TYPE_RGB_ALPHA:
      result = PixelFormat::RGBA8;
      return true;
  }

  return false;
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

Image::Image(const ResourceInfo& info,
             const PixelFormat& initFormat,
             unsigned int initWidth,
             unsigned int initHeight,
             unsigned int initDepth,
             const void* initData,
             unsigned int pitch):
  Resource(info),
  width(initWidth),
  height(initHeight),
  depth(initDepth),
  format(initFormat)
{
  assert(format.getSemantic() != PixelFormat::NONE);
  assert(format.getType() != PixelFormat::DUMMY);
  assert(width > 0);
  assert(height > 0);
  assert(depth > 0);

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
      unsigned int size = format.getSize();
      data.resize(width * height * depth * size);

      uint8* target = data;
      const uint8* source = (const uint8*) initData;

      for (unsigned int z = 0;  z < depth;  z++)
      {
        for (unsigned int y = 0;  y < height;  y++)
        {
          std::memcpy(target, source, width * size);
          source += pitch;
          target += width * size;
        }
      }
    }
    else
    {
      data.copyFrom((const uint8*) initData,
                    width * height * depth * format.getSize());
    }
  }
  else
  {
    const unsigned int size = width * height * depth * format.getSize();
    data.resize(size);
    std::memset(data, 0, size);
  }
}

Image::Image(const Image& source):
  Resource(source)
{
  operator = (source);
}

bool Image::transformTo(const PixelFormat& targetFormat, PixelTransform& transform)
{
  if (format == targetFormat)
    return true;

  if (!transform.supports(targetFormat, format))
    return false;

  Block target(width * height * depth * targetFormat.getSize());
  transform.convert(target, targetFormat, data, format, width * height * depth);
  data.attach(target.detach(), target.getSize());

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

  if (area.position.x < 0 || area.position.y < 0 ||
      area.size.x < 0 || area.size.y < 0 ||
      area.position.x >= (int) width ||
      area.position.y >= (int) height)
  {
    logError("Invalid image area dimensions");
    return false;
  }

  Recti targetArea = area;

  if (area.position.x + area.size.x > (int) width)
    targetArea.size.x = (int) width - area.position.x;
  if (area.position.y + area.size.y > (int) height)
    targetArea.size.y = (int) height - area.position.y;

  const unsigned int pixelSize = format.getSize();

  Block scratch(targetArea.size.x * targetArea.size.y * pixelSize);

  for (int y = 0;  y < targetArea.size.y;  y++)
  {
    scratch.copyFrom(data + ((y + targetArea.position.y) * width + targetArea.position.x) * pixelSize,
                     targetArea.size.x * pixelSize,
                     y * targetArea.size.x * pixelSize);
  }

  width = targetArea.size.x;
  height = targetArea.size.y;

  data.attach(scratch.detach(), scratch.getSize());
  return true;
}

void Image::flipHorizontal()
{
  unsigned int pixelSize = format.getSize();

  Block scratch(data.getSize());

  for (unsigned int z = 0;  z < depth;  z++)
  {
    size_t offset = z * width * height * pixelSize;

    for (unsigned int y = 0;  y < height;  y++)
    {
      scratch.copyFrom(data + offset + y * width * pixelSize,
                       width * pixelSize,
                       offset + (height - y - 1) * width * pixelSize);
    }
  }

  data.attach(scratch.detach(), scratch.getSize());
}

void Image::flipVertical()
{
  unsigned int pixelSize = format.getSize();

  Block scratch(data.getSize());

  for (unsigned int z = 0;  z < depth;  z++)
  {
    for (unsigned int y = 0;  y < height;  y++)
    {
      const uint8* source = data + (z * height + y) * width * pixelSize;
      uint8* target = scratch + ((z * height + y + 1) * width - 1) * pixelSize;

      while (source < target)
      {
        for (unsigned int i = 0;  i < pixelSize;  i++)
          target[i] = source[i];

        source += pixelSize;
        target -= pixelSize;
      }
    }
  }

  data.attach(scratch.detach(), scratch.getSize());
}

Image& Image::operator = (const Image& source)
{
  width = source.width;
  height = source.height;
  depth = source.depth;
  format = source.format;
  data = source.data;

  return *this;
}

bool Image::isPOT() const
{
  if (width & (width - 1))
    return false;
  if (height & (height - 1))
    return false;
  if (depth & (depth - 1))
    return false;

  return true;
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

Ref<Image> Image::getArea(const Recti& area)
{
  if (area.position.x >= (int) width || area.position.y >= (int) height)
    return NULL;

  Recti targetArea = area;

  if (area.position.x + area.size.x > (int) width)
    targetArea.size.x = (int) width - area.position.x;
  if (area.position.y + area.size.y > (int) height)
    targetArea.size.y = (int) height - area.position.y;

  const unsigned int pixelSize = format.getSize();

  ImageRef result = new Image(ResourceInfo(getCache()),
                              format,
                              targetArea.size.x, targetArea.size.y);

  for (int y = 0;  y < targetArea.size.y;  y++)
  {
    const uint8* source = data + ((y + targetArea.position.y) * width + targetArea.position.x) * pixelSize;
    uint8* target = result->data + y * result->width * pixelSize;
    memcpy(target, source, result->width * pixelSize);
  }

  return result;
}

Ref<Image> Image::read(ResourceCache& cache, const Path& path)
{
  ImageReader reader(cache);
  return reader.read(path);
}

///////////////////////////////////////////////////////////////////////

ImageCube::ImageCube(const ResourceInfo& info):
  Resource(info)
{
}

ImageCube::ImageCube(const ImageCube& source):
  Resource(source)
{
}

bool ImageCube::isPOT() const
{
  if (!isComplete())
    return false;

  for (unsigned int i = 0;  i < 6;  i++)
  {
    if (!images[i]->isPOT())
      return false;
  }

  return true;
}

bool ImageCube::isSquare() const
{
  if (!isComplete())
    return false;

  for (unsigned int i = 0;  i < 6;  i++)
  {
    if (!images[i]->isSquare())
      return false;
  }

  return true;
}

bool ImageCube::isComplete() const
{
  for (unsigned int i = 0;  i < 6;  i++)
  {
    if (!images[i])
      return false;
  }

  return true;
}

bool ImageCube::hasSameFormat() const
{
  if (!isComplete())
    return false;

  PixelFormat format = images[0]->getFormat();

  for (unsigned int i = 1;  i < 6;  i++)
  {
    if (images[i]->getFormat() != format)
      return false;
  }

  return true;
}

bool ImageCube::hasSameSize() const
{
  if (!isComplete())
    return false;

  unsigned int width = images[0]->getWidth();
  unsigned int height = images[0]->getHeight();

  for (unsigned int i = 1;  i < 6;  i++)
  {
    if (images[i]->getWidth() != width || images[i]->getHeight() != height)
      return false;
  }

  return true;
}

Ref<ImageCube> ImageCube::read(ResourceCache& cache, const Path& path)
{
  ImageCubeReader reader(cache);
  return reader.read(path);
}

///////////////////////////////////////////////////////////////////////

ImageReader::ImageReader(ResourceCache& cache):
  ResourceReader(cache)
{
}

Ref<Image> ImageReader::read(const Path& path)
{
  if (Resource* cached = getCache().findResource(path))
    return dynamic_cast<Image*>(cached);

  ResourceInfo info(getCache(), path);

  std::ifstream stream;
  if (!getCache().openFile(stream, info.path))
    return NULL;

  // Check if file is valid
  {
    unsigned char header[8];

    if (!stream.read((char*) header, sizeof(header)))
    {
      logError("Failed to read PNG file header");
      return NULL;
    }

    if (png_sig_cmp(header, 0, sizeof(header)))
    {
      logError("File is not a valid PNG file");
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
      return NULL;

    png_set_read_fn(context, &stream, readStreamPNG);

    pngInfo = png_create_info_struct(context);
    if (!pngInfo)
    {
      png_destroy_read_struct(&context, NULL, NULL);
      return NULL;
    }

    pngEndInfo = png_create_info_struct(context);
    if (!pngEndInfo)
    {
      png_destroy_read_struct(&context, &pngInfo, NULL);
      return NULL;
    }

    png_set_sig_bytes(context, 8);
  }

  PixelFormat format;
  unsigned int width;
  unsigned int height;

  // Read image information
  {
    png_read_png(context, pngInfo, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);

    if (png_get_bit_depth(context, pngInfo) != 8)
    {
      png_destroy_read_struct(&context, &pngInfo, &pngEndInfo);

      logError("Unsupported bit depth in PNG file");
      return NULL;
    }

    if (!getDecodeConversionFormatPNG(format, png_get_color_type(context, pngInfo)))
    {
      png_destroy_read_struct(&context, &pngInfo, &pngEndInfo);

      logError("Unsupported color type in PNG file");
      return NULL;
    }

    width  = png_get_image_width(context, pngInfo);
    height = png_get_image_height(context, pngInfo);
  }

  Ref<Image> result(new Image(info, format, width, height));

  // Read image data
  {
    const unsigned int size = png_get_rowbytes(context, pngInfo);

    png_bytepp rows = png_get_rows(context, pngInfo);

    uint8* data = (uint8*) result->getPixels();

    for (unsigned int i = 0;  i < height;  i++)
      std::memcpy(data + (height - i - 1) * size, rows[i], size);
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
    logError("Failed to open \'%s\' for writing",
             path.asString().c_str());
    return false;
  }

  png_structp context = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                NULL,
                                                writeErrorPNG,
                                                writeWarningPNG);
  if (!context)
  {
    logError("Failed to create write struct");
    return false;
  }

  png_set_write_fn(context, &stream, writeStreamPNG, flushStreamPNG);
  png_set_filter(context, 0, PNG_FILTER_NONE);

  png_infop info = png_create_info_struct(context);
  if (!info)
  {
    png_destroy_write_struct(&context, png_infopp(NULL));
    logError("Failed to create info struct");
    return false;
  }

  int format;

  if (!getEncodeConversionFormatPNG(format, image.getFormat()))
  {
    png_destroy_write_struct(&context, &info);
    logError("Failed to encode image format");
    return false;
  }

  png_set_IHDR(context,
               info,
               image.getWidth(),
               image.getHeight(),
               8,
               format,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  const uint8* data = (const uint8*) image.getPixels();

  const unsigned int pixelSize = image.getFormat().getSize();

  const png_byte** rows = new const png_byte* [image.getHeight()];

  for (unsigned int y = 0;  y < image.getHeight();  y++)
    rows[y] = data + y * image.getWidth() * pixelSize;

  png_set_rows(context, info, const_cast<png_byte**>(rows));

  png_write_png(context, info, PNG_TRANSFORM_IDENTITY, NULL);

  png_destroy_write_struct(&context, &info);

  delete [] rows;
  rows = NULL;

  return true;
}

///////////////////////////////////////////////////////////////////////

ImageCubeReader::ImageCubeReader(ResourceCache& cache):
  ResourceReader(cache)
{
}

Ref<ImageCube> ImageCubeReader::read(const Path& path)
{
  if (Resource* cached = getCache().findResource(path))
    return dynamic_cast<ImageCube*>(cached);

  std::ifstream stream;
  if (!getCache().openFile(stream, path))
    return NULL;

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load image cube \'%s\': %s",
             path.asString().c_str(),
             result.description());
    return NULL;
  }

  pugi::xml_node root = document.child("image-cube");
  if (!root || root.attribute("version").as_uint() != IMAGE_CUBE_XML_VERSION)
  {
    logError("Image cube file format mismatch in \'%s\'",
             path.asString().c_str());
    return NULL;
  }

  // NOTE: Keep these arrays in the same order, for added happiness

  const char* names[6] =
  {
    "positive-x",
    "negative-x",
    "positive-y",
    "negative-y",
    "positive-z",
    "negative-z",
  };

  const CubeFace sides[6] =
  {
    CUBE_POSITIVE_X,
    CUBE_NEGATIVE_X,
    CUBE_POSITIVE_Y,
    CUBE_NEGATIVE_Y,
    CUBE_POSITIVE_Z,
    CUBE_NEGATIVE_Z,
  };

  Ref<ImageCube> cube = new ImageCube(ResourceInfo(getCache(), path));

  for (size_t i = 0;  i < 6;  i++)
  {
    const Path imagePath(root.child(names[i]).attribute("path").value());
    if (imagePath.isEmpty())
    {
      logError("No path specified for %s side in image cube \'%s\'",
               names[i],
               path.asString().c_str());
      return NULL;
    }

    Ref<Image> image = Image::read(getCache(), imagePath);
    if (!image)
    {
      logError("Failed to load side %s of image cube \'%s\'",
               names[i],
               path.asString().c_str());
      return NULL;
    }

    cube->images[sides[i]] = image;
  }

  return cube;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
