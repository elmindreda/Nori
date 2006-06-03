///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@home.se>
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

#include <moira/Config.h>
#include <moira/Core.h>
#include <moira/Log.h>
#include <moira/Point.h>
#include <moira/Color.h>
#include <moira/Image.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{

unsigned int getClosestPower(unsigned int value, unsigned int maximum)
{
  unsigned int result;

  if (value > maximum)
    result = maximum;
  else
    result = value;

  if (result & (result - 1))
  {
    // value is not power of two
    // find largest power lesser than maximum

    unsigned int i;

    for (i = 0;  result & ~1;  i++)
      result >>= 1;

    result = 1 << i;
  }

  if ((result << 1) > maximum)
  {
    // maximum is not power of two, so we give up here
    return result;
  }

  if (value > result)
  {
    // there is room to preserve all detail, so use it
    return result << 1;
  }

  return result;
}

ImageFormat::Type getConversionFormat(const ImageFormat& format)
{
  switch (format)
  {
    case ImageFormat::RGBX8888:
      return ImageFormat::RGB888;
    default:
      return format;
  }
}

unsigned int convertFormatToGL(const ImageFormat& format)
{
  switch (format)
  {
    case ImageFormat::ALPHA8:
      return GL_ALPHA8;
    case ImageFormat::GREY8:
      return GL_LUMINANCE8;
    case ImageFormat::GREYALPHA88:
      return GL_LUMINANCE8_ALPHA8;
    case ImageFormat::RGB888:
      return GL_RGB8;
    case ImageFormat::RGBX8888:
      return 0;
    case ImageFormat::RGBA8888:
      return GL_RGBA8;
    default:
      return 0;
  }
}

unsigned int convertFormatToGenericGL(const ImageFormat& format)
{
  switch (format)
  {
    case ImageFormat::ALPHA8:
      return GL_ALPHA;
    case ImageFormat::GREY8:
      return GL_LUMINANCE;
    case ImageFormat::GREYALPHA88:
      return GL_LUMINANCE_ALPHA;
    case ImageFormat::RGB888:
      return GL_RGB;
    case ImageFormat::RGBX8888:
      return 0;
    case ImageFormat::RGBA8888:
      return GL_RGBA;
    default:
      return 0;
  }
}

}

///////////////////////////////////////////////////////////////////////

Texture::~Texture(void)
{
  if (textureID != 0)
    glDeleteTextures(1, &textureID);
}

void* Texture::lock(unsigned int level)
{
  // TODO: Fail if any level already locked.

  if (flags & SHADOWED)
  {
    // TODO: Expose memory mipmap.
    return NULL;
  }
  else
  {
    // TODO: Read back and expose VRAM mipmap.
    return NULL;
  }
}

void Texture::unlock(void)
{
  // TODO: Update VRAM mipmap.
}

GLuint Texture::getGLID(void) const
{
  return textureID;
}

GLenum Texture::getTarget(void) const
{
  return textureTarget;
}

unsigned int Texture::getWidth(unsigned int level) const
{
  return width >> level;
}

unsigned int Texture::getHeight(unsigned int level) const
{
  return height >> level;
}

unsigned int Texture::getDepth(unsigned int level) const
{
  return depth >> level;
}

unsigned int Texture::getPhysicalWidth(unsigned int level) const
{
  return physicalWidth >> level;
}

unsigned int Texture::getPhysicalHeight(unsigned int level) const
{
  return physicalHeight >> level;
}

unsigned int Texture::getPhysicalDepth(unsigned int level) const
{
  return physicalDepth >> level;
}

unsigned int Texture::getLevelCount(void) const
{
  return levelCount;
}

unsigned int Texture::getFlags(void) const
{
  return flags;
}

const ImageFormat& Texture::getFormat(void) const
{
  return format;
}

void Texture::setFilters(GLenum minFilter, GLenum magFilter)
{
  glPushAttrib(GL_TEXTURE_BIT);
  glBindTexture(textureTarget, textureID);
  glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, magFilter);
  glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, minFilter);
  glPopAttrib();
}

Texture* Texture::createInstance(const std::string& name,
                                 const Path& path,
				 unsigned int flags)
{
  ImageReader reader;
  
  Ptr<Image> image = reader.read(path);
  if (!image)
    return NULL;

  return createInstance(name, *image, flags);
}

Texture* Texture::createInstance(const std::string& name,
                                 const Image& image,
				 unsigned int flags)
{
  Ptr<Texture> texture = new Texture(name);
  if (!texture->init(image, flags))
    return NULL;

  return texture.detachObject();
}

Texture::Texture(const std::string& name):
  Managed<Texture>(name),
  textureID(0),
  textureTarget(0),
  width(0),
  height(0),
  depth(0),
  physicalWidth(0),
  physicalHeight(0),
  physicalDepth(0),
  levelCount(0),
  flags(0)
{
}

bool Texture::init(const Image& image, unsigned int initFlags)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create texture without OpenGL context");
    return false;
  }

  if (image.getHeight() > 1)
    textureTarget = GL_TEXTURE_2D;
  else
    textureTarget = GL_TEXTURE_1D;

  Ptr<Image> result;
  const Image* current = &image;

  ImageFormat format = getConversionFormat(current->getFormat());
  if (format != current->getFormat())
    current = result = current->convertTo(format);

  if (initFlags & RECTANGULAR)
  {
    // TODO: Support ARB_texture_rectangle.

    if (!GLEW_ARB_texture_non_power_of_two)
    {
      Log::writeError("Rectangular textures unsupported");
      return false;
    }
  }
  else
  {
    unsigned int maxSize;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*) &maxSize);

    unsigned int physicalWidth = getClosestPower(current->getWidth(), maxSize);
    unsigned int physicalHeight = getClosestPower(current->getHeight(), maxSize);

    if (physicalWidth != current->getWidth() || physicalHeight != current->getHeight())
    {
      Image* scaled = new Image(current->getFormat(), physicalWidth, physicalHeight);

      // TODO: Update this if support for non byte-sized channels are added.

      gluScaleImage(convertFormatToGenericGL(current->getFormat()),
                    current->getWidth(),
		    current->getHeight(),
		    GL_UNSIGNED_BYTE,
		    current->getPixels(),
		    scaled->getWidth(),
		    scaled->getHeight(),
		    GL_UNSIGNED_BYTE,
		    scaled->getPixels());

      current = result = scaled;

      // TODO: Make this work instead.
      //source = source->resampleTo(physicalWidth, physicalHeight);
    }
  }

  glPushAttrib(GL_TEXTURE_BIT);
  glGenTextures(1, &textureID);
  glBindTexture(textureTarget, textureID);

  if (initFlags & MIPMAPPED)
  {
    if (textureTarget == GL_TEXTURE_1D)
    {
      gluBuild1DMipmaps(textureTarget,
                        format.getChannelCount(),
                        current->getWidth(),
                        convertFormatToGenericGL(current->getFormat()),
                        GL_UNSIGNED_BYTE,
                        current->getPixels());
    }
    else
    {
      gluBuild2DMipmaps(textureTarget,
                        format.getChannelCount(),
                        current->getWidth(),
                        current->getHeight(),
                        convertFormatToGenericGL(current->getFormat()),
                        GL_UNSIGNED_BYTE,
                        current->getPixels());
    }
  }
  else
  {
    if (textureTarget == GL_TEXTURE_1D)
    {
      glTexImage1D(textureTarget,
                   0,
                   convertFormatToGL(current->getFormat()),
                   current->getWidth(),
                   0,
                   convertFormatToGenericGL(current->getFormat()),
                   GL_UNSIGNED_BYTE,
                   current->getPixels());
    }
    else
    {
      glTexImage2D(textureTarget,
                   0,
                   convertFormatToGL(current->getFormat()),
                   current->getWidth(),
                   current->getHeight(),
                   0,
                   convertFormatToGenericGL(current->getFormat()),
                   GL_UNSIGNED_BYTE,
                   current->getPixels());
    }

    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }

  glPopAttrib();

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeWarning("Error during texture creation: %s", gluErrorString(error));
    return false;
  }
  
  // "Regular" dimensions of texture object are equal to source image dimensions
  width = image.getWidth();
  height = image.getHeight();

  flags = initFlags;

  return true;
}
  
///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
