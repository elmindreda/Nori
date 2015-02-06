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

#include <fstream>
#include <cstring>

#include <glm/gtc/round.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace wendy
{

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

  for (uint y = 0;  y < (size_t) area.size.y;  y++)
  {
    std::memcpy(temp.data() + y * area.size.x * pixelSize,
                m_data.data() + ((y + area.position.y) * m_width + area.position.x) * pixelSize,
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

  for (uint z = 0;  z < m_depth;  z++)
  {
    const size_t sliceOffset = z * m_height * rowSize;

    for (uint y = 0;  y < m_height;  y++)
    {
      std::memcpy(temp.data() + sliceOffset + rowSize * (m_height - y - 1),
                  m_data.data() + sliceOffset + rowSize * y,
                  rowSize);
    }
  }

  std::swap(m_data, temp);
}

void Image::flipVertical()
{
  const size_t pixelSize = m_format.size();
  std::vector<char> temp(m_data.size());

  for (uint z = 0;  z < m_depth;  z++)
  {
    for (uint y = 0;  y < m_height;  y++)
    {
      const char* source = m_data.data() + (z * m_height + y) * m_width * pixelSize;
      char* target = temp.data() + ((z * m_height + y + 1) * m_width - 1) * pixelSize;

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

  return m_data.data() + ((z * m_height + y) * m_width + x) * m_format.size();
}

const void* Image::pixel(uint x, uint y, uint z) const
{
  if (x >= m_width || y >= m_height || z >= m_depth)
    return nullptr;

  return m_data.data() + ((z * m_height + y) * m_width + x) * m_format.size();
}

uint Image::dimensionCount() const
{
  if (m_depth > 1)
    return 3;

  if (m_height > 1)
    return 2;

  return 1;
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

Ref<Image> Image::read(ResourceCache& cache, const std::string& name)
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

  assert(m_format.isValid());
  assert(m_width);
  assert(m_height);
  assert(m_depth);

  if (pixels)
  {
    if (pitch)
    {
      const size_t pixelSize = m_format.size();
      m_data.resize(m_width * m_height * m_depth * pixelSize);

      char* target = m_data.data();
      const char* source = pixels;

      for (uint z = 0;  z < m_depth;  z++)
      {
        for (uint y = 0;  y < m_height;  y++)
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

ImageReader::ImageReader(ResourceCache& cache):
  ResourceReader<Image>(cache)
{
}

Ref<Image> ImageReader::read(const std::string& name, const Path& path)
{
  int width, height, format;
  stbi_uc* pixels = stbi_load(path.name().c_str(),
                              &width, &height,
                              &format, STBI_default);
  if (!pixels)
  {
    logError("Failed to read image %s", path.name().c_str());
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

} /*namespace wendy*/

