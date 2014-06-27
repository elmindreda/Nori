///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#include <algorithm>
#include <fstream>
#include <cstring>

#include <glm/gtx/bit.hpp>

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

PixelFormat convertToPixelFormat(int format)
{
  switch (format)
  {
    case STBI_grey:
      return PixelFormat::L8;
    case STBI_grey_alpha:
      return PixelFormat::LA8;
    case STBI_rgb:
      return PixelFormat::RGB8;
    case STBI_rgb_alpha:
      return PixelFormat::RGBA8;
    default:
      return PixelFormat();
  }
}

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

  for (uint y = 0;  y < uint(area.size.y);  y++)
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

  std::vector<char> data;

  stream.seekg(0, std::ios::end);
  data.resize((size_t) stream.tellg());
  stream.seekg(0, std::ios::beg);
  stream.read(&data[0], data.size());

  int width, height, format;
  stbi_uc* pixels = stbi_load_from_memory((stbi_uc*) data.data(), data.size(),
                                          &width, &height,
                                          &format, STBI_default);
  if (!pixels)
  {
    logError("Failed to load image %s", path.name().c_str());
    return nullptr;
  }

  for (int i = 0;  i < height / 2;  i++)
  {
    std::swap_ranges(pixels + width * format * i,
                     pixels + width * format * (i + 1),
                     pixels + width * format * (height - i - 1));
  }

  Ref<Image> result = Image::create(ResourceInfo(cache, name, path),
                                    convertToPixelFormat(format),
                                    width, height, 1, pixels);

  stbi_image_free(pixels);
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

  if (image.format().type() != PixelFormat::UINT8)
  {
    logError("Only 8-bit images may be written");
    return false;
  }

  const int stride = int(image.width() * image.format().size());
  const void* start = (char*) image.pixels() + stride * (image.height() - 1);

  return stbi_write_png(path.name().c_str(),
                        image.width(), image.height(),
                        image.format().channelCount(),
                        start, -stride) != 0;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
