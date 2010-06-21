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
#include <wendy/Vector.h>
#include <wendy/Rectangle.h>
#include <wendy/Path.h>
#include <wendy/Pixel.h>
#include <wendy/Stream.h>
#include <wendy/Managed.h>
#include <wendy/Resource.h>
#include <wendy/Image.h>

#include <stdint.h>

#include <cstring>
#include <cmath>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

void samplePixelsNearest1D(Byte* target,
                           unsigned int targetWidth,
                           const Byte* source,
                           unsigned int sourceWidth,
                           const PixelFormat& format)
{
  const size_t pixelSize = format.getSize();

  const float sx = (sourceWidth - 1) / (float) (targetWidth - 1);

  Byte* targetPixel = target;

  for (unsigned int x = 0;  x < targetWidth;  x++)
  {
    const Byte* sourcePixel = source + (size_t) (x * sx) * pixelSize;
    for (unsigned int i = 0;  i < pixelSize;  i++)
      targetPixel[i] = sourcePixel[i];

    targetPixel += pixelSize;
  }
}

template <typename T>
void samplePixelsLinear1D(void* target,
                          size_t targetWidth,
                          const void* source,
                          size_t sourceWidth,
                          size_t channelCount)
{
  const float stepU = (sourceWidth - 1) / (float) (targetWidth - 1);

  T* targetPixel = (T*) target;
  const T* sourcePixel = (const T*) source;

  for (size_t x = 0;  x < targetWidth;  x++)
  {
    const float u = x * stepU;

    const size_t minU = (size_t) floorf(u);
    const size_t maxU = (size_t) ceilf(u);

    const float fracU = u - (float) minU;

    for (size_t i = 0;  i < channelCount;  i++)
    {
      *targetPixel++ = (T) (sourcePixel[minU * channelCount + i] * (1.f - fracU) +
                            sourcePixel[maxU * channelCount + i] * fracU);
    }
  }
}

void samplePixelsLinear1D_UINT24(void* target,
                                 size_t targetWidth,
                                 const void* source,
                                 size_t sourceWidth,
                                 size_t channelCount)
{
  // TODO: The code
}

void samplePixelsLinear1D(Byte* target,
                          unsigned int targetWidth,
                          const Byte* source,
                          unsigned int sourceWidth,
                          const PixelFormat& format)
{
  switch (format.getType())
  {
    case PixelFormat::UINT8:
      samplePixelsLinear1D<uint8_t>(target, targetWidth,
                                    source, sourceWidth,
                                    format.getChannelCount());
      break;
    case PixelFormat::UINT16:
      samplePixelsLinear1D<uint16_t>(target, targetWidth,
                                     source, sourceWidth,
                                     format.getChannelCount());
      break;
    case PixelFormat::UINT24:
      samplePixelsLinear1D_UINT24(target, targetWidth,
                                  source, sourceWidth,
                                  format.getChannelCount());
      break;
    case PixelFormat::UINT32:
      samplePixelsLinear1D<uint32_t>(target, targetWidth,
                                     source, sourceWidth,
                                     format.getChannelCount());
      break;
    case PixelFormat::FLOAT32:
      samplePixelsLinear1D<float>(target, targetWidth,
                                  source, sourceWidth,
                                  format.getChannelCount());
      break;
  }
}

void samplePixelsNearest2D(Byte* target,
                           unsigned int targetWidth,
                           unsigned int targetHeight,
                           const Byte* source,
                           unsigned int sourceWidth,
                           unsigned int sourceHeight,
                           const PixelFormat& format)
{
  const size_t pixelSize = format.getSize();

  const float sx = (sourceWidth - 1) / (float) (targetWidth - 1);
  const float sy = (sourceHeight - 1) / (float) (targetHeight - 1);

  Byte* targetPixel = target;

  for (unsigned int y = 0;  y < targetHeight;  y++)
  {
    for (unsigned int x = 0;  x < targetWidth;  x++)
    {
      const Byte* sourcePixel = source + ((size_t) (x * sx) +
                                          (size_t) (y * sy) * sourceWidth) * pixelSize;
      for (unsigned int i = 0;  i < pixelSize;  i++)
        targetPixel[i] = sourcePixel[i];

      targetPixel += pixelSize;
    }
  }
}

template <typename T>
void samplePixelsLinear2D(void* target,
                          size_t targetWidth,
                          size_t targetHeight,
                          const void* source,
                          size_t sourceWidth,
                          size_t sourceHeight,
                          size_t channelCount)
{
  const float stepU = (sourceWidth - 1) / (float) (targetWidth - 1);
  const float stepV = (sourceHeight - 1) / (float) (targetHeight - 1);

  T* targetPixel = (T*) target;
  const T* sourcePixel = (const T*) source;

  for (size_t y = 0;  y < targetHeight;  y++)
  {
    for (size_t x = 0;  x < targetWidth;  x++)
    {
      const float u = x * stepU;
      const float v = y * stepV;

      const size_t minU = (size_t) floorf(u);
      const size_t minV = (size_t) floorf(v);
      const size_t maxU = (size_t) ceilf(u);
      const size_t maxV = (size_t) ceilf(v);

      const float fracU = u - (float) minU;
      const float fracV = v - (float) minV;

      for (size_t i = 0;  i < channelCount;  i++)
      {
        T value = 0;

        value += (T) (sourcePixel[(minU + minV * sourceWidth) * channelCount + i] *
                      (1.f - fracU) * (1.f - fracV));
        value += (T) (sourcePixel[(maxU + minV * sourceWidth) * channelCount + i] *
                      (fracU) * (1.f - fracV));
        value += (T) (sourcePixel[(minU + maxV * sourceWidth) * channelCount + i] *
                      (1.f - fracU) * (fracV));
        value += (T) (sourcePixel[(maxU + maxV * sourceWidth) * channelCount + i] *
                      (fracU) * (fracV));

        *targetPixel++ = value;
      }
    }
  }
}

void samplePixelsLinear2D_UINT24(void* target,
                                 size_t targetWidth,
                                 size_t targetHeight,
                                 const void* source,
                                 size_t sourceWidth,
                                 size_t sourceHeight,
                                 size_t channelCount)
{
  // TODO: The code
}

void samplePixelsLinear2D(Byte* target,
                          unsigned int targetWidth,
                          unsigned int targetHeight,
                          const Byte* source,
                          unsigned int sourceWidth,
                          unsigned int sourceHeight,
                          const PixelFormat& format)
{
  switch (format.getType())
  {
    case PixelFormat::UINT8:
      samplePixelsLinear2D<uint8_t>(target, targetWidth, targetHeight,
                                    source, sourceWidth, sourceHeight,
                                    format.getChannelCount());
      break;
    case PixelFormat::UINT16:
      samplePixelsLinear2D<uint16_t>(target, targetWidth, targetHeight,
                                     source, sourceWidth, sourceHeight,
                                     format.getChannelCount());
      break;
    case PixelFormat::UINT24:
      samplePixelsLinear2D_UINT24(target, targetWidth, targetHeight,
                                  source, sourceWidth, sourceHeight,
                                  format.getChannelCount());
      break;
    case PixelFormat::UINT32:
      samplePixelsLinear2D<uint32_t>(target, targetWidth, targetHeight,
                                     source, sourceWidth, sourceHeight,
                                     format.getChannelCount());
      break;
    case PixelFormat::FLOAT32:
      samplePixelsLinear2D<float>(target, targetWidth, targetHeight,
                                  source, sourceWidth, sourceHeight,
                                  format.getChannelCount());
      break;
  }
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

Image::Image(const PixelFormat& initFormat,
             unsigned int initWidth,
             unsigned int initHeight,
             const void* initData,
             size_t pitch,
             const String& name):
  Resource<Image>(name),
  width(initWidth),
  height(initHeight),
  format(initFormat)
{
  if (format.getSemantic() == PixelFormat::NONE ||
      format.getType() == PixelFormat::DUMMY)
    throw Exception("Invalid image format");

  if (width == 0 || height == 0)
    throw Exception("Invalid image size");

  if ((height > 1) && (width == 1))
  {
    width = height;
    height = 1;
  }

  if (initData)
  {
    if (pitch)
    {
      size_t size = format.getSize();
      data.resize(width * height * size);

      Byte* target = data;
      const Byte* source = (const Byte*) initData;

      for (unsigned int y = 0;  y < height;  y++)
      {
        std::memcpy(target, source, width * size);
        source += pitch;
        target += width * size;
      }
    }
    else
      data.copyFrom((const Byte*) initData, width * height * format.getSize());
  }
  else
  {
    const size_t size = width * height * format.getSize();
    data.reserve(size);
    std::memset(data, 0, size);
  }
}

Image::Image(const Image& source):
  Resource<Image>(source)
{
  operator = (source);
}

bool Image::resize(unsigned int targetWidth,
                   unsigned int targetHeight,
                   Method method)
{
  if (targetWidth == 0 || targetHeight == 0)
    throw Exception("Invalid image target size");

  if (targetWidth == width && targetHeight == height)
    return true;

  const size_t pixelSize = format.getSize();

  Block scratch(targetWidth * targetHeight * pixelSize);

  switch (method)
  {
    case SAMPLE_NEAREST:
    {
      if (getDimensionCount() == 1)
        samplePixelsNearest1D(scratch, targetWidth, data, width, format);
      else
        samplePixelsNearest2D(scratch, targetWidth, targetHeight, data, width, height, format);

      break;
    }

    case SAMPLE_LINEAR:
    {
      if (getDimensionCount() == 1)
        samplePixelsLinear1D(scratch, targetWidth, data, width, format);
      else
        samplePixelsLinear2D(scratch, targetWidth, targetHeight, data, width, height, format);

      break;
    }

    default:
      Log::writeError("Invalid image resampling filter");
      return false;
  }

  width = targetWidth;
  height = targetHeight;

  data.attach(scratch.detach(), width * height * pixelSize);
  return true;
}

bool Image::transformTo(const PixelFormat& targetFormat, PixelTransform& transform)
{
  if (format == targetFormat)
    return true;

  if (!transform.supports(targetFormat, format))
    return false;

  Block target(width * height * targetFormat.getSize());
  transform.convert(target, targetFormat, data, format, width * height);
  data.attach(target.detach(), target.getSize());

  format = targetFormat;
  return true;
}

bool Image::crop(const Recti& area)
{
  if (area.position.x < 0 || area.position.y < 0 ||
      area.size.x < 0 || area.size.y < 0 ||
      area.position.x >= (int) width || area.position.y >= (int) height)
  {
    Log::writeError("Invalid image area dimensions");
    return false;
  }

  Recti targetArea = area;

  if (area.position.x + area.size.x > (int) width)
    targetArea.size.x = (int) width - area.position.x;
  if (area.position.y + area.size.y > (int) height)
    targetArea.size.y = (int) height - area.position.y;

  const size_t pixelSize = format.getSize();

  Block scratch(targetArea.size.x * targetArea.size.y * pixelSize);

  for (unsigned int y = 0;  y < targetArea.size.y;  y++)
  {
    scratch.copyFrom(data + ((y + targetArea.position.y) * width + targetArea.position.x) * pixelSize,
                     targetArea.size.x * pixelSize,
                     y * targetArea.size.x * pixelSize);
  }

  width = targetArea.size.x;
  height = targetArea.size.y;

  data.attach(scratch.detach(), width * height * pixelSize);
  return true;
}

void Image::flipHorizontal(void)
{
  size_t pixelSize = format.getSize();

  Block scratch(width * height * pixelSize);

  for (unsigned int y = 0;  y < height;  y++)
  {
    scratch.copyFrom(data + y * width * pixelSize,
                     width * pixelSize,
                     (height - y - 1) * width * pixelSize);
  }

  data.attach(scratch.detach(), width * height * pixelSize);
}

void Image::flipVertical(void)
{
  size_t pixelSize = format.getSize();

  Block scratch(width * height * pixelSize);

  for (unsigned int y = 0;  y < height;  y++)
  {
    const Byte* source = data + y * width * pixelSize;
    Byte* target = scratch + ((y + 1) * width - 1) * pixelSize;

    for (unsigned int x = 0;  x < width;  x++)
    {
      for (unsigned int i = 0;  i < pixelSize;  i++)
        target[i] = source[i];

      source += pixelSize;
      target -= pixelSize;
    }
  }

  data.attach(scratch.detach(), width * height * pixelSize);
}

Image& Image::operator = (const Image& source)
{
  width = source.width;
  height = source.height;
  format = source.format;

  data = source.data;
  return *this;
}

bool Image::isPOT(void) const
{
  if (width & (width - 1))
    return false;
  if (height & (height - 1))
    return false;

  return true;
}

bool Image::isSquare(void) const
{
  return width == height;
}

unsigned int Image::getWidth(void) const
{
  return width;
}

unsigned int Image::getHeight(void) const
{
  return height;
}

void* Image::getPixels(void)
{
  return data;
}

const void* Image::getPixels(void) const
{
  return data;
}

void* Image::getPixel(unsigned int x, unsigned int y)
{
  if (x >= width || y >= height)
    return NULL;

  return data + (y * width + x) * format.getSize();
}

const void* Image::getPixel(unsigned int x, unsigned int y) const
{
  if (x >= width || y >= height)
    return NULL;

  return data + (y * width + x) * format.getSize();
}

const PixelFormat& Image::getFormat(void) const
{
  return format;
}

unsigned int Image::getDimensionCount(void) const
{
  if (height > 1)
    return 2;
  else
    return 1;
}

Image* Image::getArea(const Recti& area)
{
  if (area.position.x >= (int) width || area.position.y >= (int) height)
    return NULL;

  Recti targetArea = area;

  if (area.position.x + area.size.x > (int) width)
    targetArea.size.x = (int) width - area.position.x;
  if (area.position.y + area.size.y > (int) height)
    targetArea.size.y = (int) height - area.position.y;

  const size_t pixelSize = format.getSize();

  Image* result = new Image(format, targetArea.size.x, targetArea.size.y);

  for (unsigned int y = 0;  y < targetArea.size.y;  y++)
  {
    const Byte* source = data + ((y + targetArea.position.y) * width + targetArea.position.x) * pixelSize;
    Byte* target = result->data + y * result->width * pixelSize;
    memcpy(target, source, result->width * pixelSize);
  }

  return result;
}

///////////////////////////////////////////////////////////////////////

ImageCube::ImageCube(const String& name):
  Resource<ImageCube>(name)
{
}

ImageCube* ImageCube::clone(void) const
{
  ImageCube* result = new ImageCube();

  for (size_t i = 0;  i < 6;  i++)
    result->images[i] = new Image(*images[i]);

  return result;
}

bool ImageCube::isPOT(void) const
{
  if (!isComplete())
    return false;

  for (size_t i = 0;  i < 6;  i++)
  {
    if (!images[i]->isPOT())
      return false;
  }

  return true;
}

bool ImageCube::isSquare(void) const
{
  if (!isComplete())
    return false;

  for (size_t i = 0;  i < 6;  i++)
  {
    if (!images[i]->isSquare())
      return false;
  }

  return true;
}

bool ImageCube::isComplete(void) const
{
  for (size_t i = 0;  i < 6;  i++)
  {
    if (!images[i])
      return false;
  }

  return true;
}

bool ImageCube::hasSameFormat(void) const
{
  if (!isComplete())
    return false;

  PixelFormat format = images[0]->getFormat();

  for (size_t i = 1;  i < 6;  i++)
  {
    if (images[i]->getFormat() != format)
      return false;
  }

  return true;
}

bool ImageCube::hasSameSize(void) const
{
  if (!isComplete())
    return false;

  unsigned int width = images[0]->getWidth();
  unsigned int height = images[0]->getHeight();

  for (size_t i = 1;  i < 6;  i++)
  {
    if (images[i]->getWidth() != width || images[i]->getHeight() != height)
      return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
