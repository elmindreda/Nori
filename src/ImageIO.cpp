///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/Portability.h>
#include <wendy/Core.h>
#include <wendy/Block.h>
#include <wendy/Color.h>
#include <wendy/Vector.h>
#include <wendy/Rectangle.h>
#include <wendy/Path.h>
#include <wendy/Pixel.h>
#include <wendy/Stream.h>
#include <wendy/XML.h>
#include <wendy/Managed.h>
#include <wendy/Resource.h>
#include <wendy/Image.h>

#include <internal/ImageIO.h>

#include <png.h>

#include <cstring>

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
  Log::writeError("libpng error: %s", error);
}

void writeWarningPNG(png_structp context, png_const_charp warning)
{
  Log::writeWarning("libpng warning: %s", warning);
}

void readStreamPNG(png_structp context, png_bytep data, png_size_t length)
{
  Stream* stream = reinterpret_cast<Stream*>(png_get_io_ptr(context));
  stream->readItems(data, length);
}

void writeStreamPNG(png_structp context, png_bytep data, png_size_t length)
{
  Stream* stream = reinterpret_cast<Stream*>(png_get_io_ptr(context));
  stream->writeItems(data, length);
}

void flushStreamPNG(png_structp context)
{
  Stream* stream = reinterpret_cast<Stream*>(png_get_io_ptr(context));
  stream->flush();
}

const unsigned int IMAGE_CUBE_XML_VERSION = 1;

}

///////////////////////////////////////////////////////////////////////

ImageCodecPNG::ImageCodecPNG(void):
  ImageCodec("PNG image codec")
{
  addSuffix("png");
}

Image* ImageCodecPNG::read(const Path& path, const String& name)
{
  return ImageCodec::read(path, name);
}

Image* ImageCodecPNG::read(Stream& stream, const String& name)
{
  // Check if file is valid
  {
    unsigned char header[8];

    if (!stream.read(header, sizeof(header)))
    {
      Log::writeError("Unable to read PNG file header");
      return NULL;
    }

    if (png_sig_cmp(header, 0, sizeof(header)))
    {
      Log::writeError("File is not a valid PNG file");
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

      Log::writeError("Unsupported bit depth in PNG file");
      return NULL;
    }

    if (!getDecodeConversionFormatPNG(format, png_get_color_type(context, pngInfo)))
    {
      png_destroy_read_struct(&context, &pngInfo, &pngEndInfo);

      Log::writeError("Unsupported color type in PNG file");
      return NULL;
    }

    width  = png_get_image_width(context, pngInfo);
    height = png_get_image_height(context, pngInfo);
  }

  Ptr<Image> result(new Image(format, width, height, NULL, 0, name));

  // Read image data
  {
    const size_t size = png_get_rowbytes(context, pngInfo);

    png_bytepp rows = png_get_rows(context, pngInfo);

    Byte* data = (Byte*) result->getPixels();

    for (unsigned int i = 0;  i < height;  i++)
      std::memcpy(data + (height - i - 1) * size, rows[i], size);
  }

  // Clean up library structures
  png_destroy_read_struct(&context, &pngInfo, &pngEndInfo);

  return result.detachObject();
}

bool ImageCodecPNG::write(const Path& path, const Image& image)
{
  return ImageCodec::write(path, image);
}

bool ImageCodecPNG::write(Stream& stream, const Image& image)
{
  png_structp context = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                NULL,
						writeErrorPNG,
						writeWarningPNG);
  if (!context)
  {
    Log::writeError("Unable to create write struct");
    return false;
  }

  png_set_write_fn(context, &stream, writeStreamPNG, flushStreamPNG);
  png_set_filter(context, 0, PNG_FILTER_NONE);

  png_infop info = png_create_info_struct(context);
  if (!info)
  {
    png_destroy_write_struct(&context, png_infopp_NULL);
    Log::writeError("Unable to create info struct");
    return false;
  }

  int format;

  if (!getEncodeConversionFormatPNG(format, image.getFormat()))
  {
    png_destroy_write_struct(&context, &info);
    Log::writeError("Unable to encode image format");
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

  const Byte* data = (const Byte*) image.getPixels();

  const size_t pixelSize = image.getFormat().getSize();

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

ImageCubeCodecXML::ImageCubeCodecXML(void):
  ImageCubeCodec("XML image cube codec")
{
  addSuffix("cube");
}

ImageCube* ImageCubeCodecXML::read(const Path& path, const String& name)
{
  return ImageCubeCodec::read(path, name);
}

ImageCube* ImageCubeCodecXML::read(Stream& stream, const String& name)
{
  cube = new ImageCube(name);

  if (!XML::Codec::read(stream))
  {
    cube = NULL;
    return NULL;
  }

  return cube.detachObject();
}

bool ImageCubeCodecXML::write(const Path& path, const ImageCube& cube)
{
  return ImageCubeCodec::write(path, cube);
}

bool ImageCubeCodecXML::write(Stream& stream, const ImageCube& cube)
{
  try
  {
    setStream(&stream);

    beginElement("image-cube");
    addAttribute("version", IMAGE_CUBE_XML_VERSION);

    // TODO: Figure out how to handle writing image data.

    endElement();

    setStream(NULL);
  }
  catch (Exception& exception)
  {
    Log::writeError("Failed to write image cube specification %s: %s",
                    cube.getName().c_str(),
		    exception.what());
    setStream(NULL);
    return false;
  }

  return true;
}

bool ImageCubeCodecXML::onBeginElement(const String& name)
{
  if (name == "image-cube")
  {
    const unsigned int version = readInteger("version");
    if (version != IMAGE_CUBE_XML_VERSION)
    {
      Log::writeError("Image cube specification XML format version mismatch");
      return false;
    }

    return true;
  }

  if (name == "positive-x")
  {
    ImageRef image = Image::readInstance(readString("name"));
    if (!image)
      return false;

    cube->images[ImageCube::POSITIVE_X] = image;
    return true;
  }

  if (name == "negative-x")
  {
    ImageRef image = Image::readInstance(readString("name"));
    if (!image)
      return false;

    cube->images[ImageCube::NEGATIVE_X] = image;
    return true;
  }

  if (name == "positive-y")
  {
    ImageRef image = Image::readInstance(readString("name"));
    if (!image)
      return false;

    cube->images[ImageCube::POSITIVE_Y] = image;
    return true;
  }

  if (name == "negative-y")
  {
    ImageRef image = Image::readInstance(readString("name"));
    if (!image)
      return false;

    cube->images[ImageCube::NEGATIVE_Y] = image;
    return true;
  }

  if (name == "positive-z")
  {
    ImageRef image = Image::readInstance(readString("name"));
    if (!image)
      return false;

    cube->images[ImageCube::POSITIVE_Z] = image;
    return true;
  }

  if (name == "negative-z")
  {
    ImageRef image = Image::readInstance(readString("name"));
    if (!image)
      return false;

    cube->images[ImageCube::NEGATIVE_Z] = image;
    return true;
  }

  return true;
}

bool ImageCubeCodecXML::onEndElement(const String& name)
{
  return true;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
