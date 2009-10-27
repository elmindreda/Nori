///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

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

GLint unmipmapMinFilter(GLint minFilter)
{
  if (minFilter == GL_NEAREST_MIPMAP_NEAREST ||
      minFilter == GL_NEAREST_MIPMAP_LINEAR)
    return GL_NEAREST;
  else if (minFilter == GL_LINEAR_MIPMAP_NEAREST ||
	   minFilter == GL_LINEAR_MIPMAP_LINEAR)
    return GL_LINEAR;

  return minFilter;
}

GLint convertAddressMode(AddressMode mode)
{
  switch (mode)
  {
    case ADDRESS_WRAP:
      return GL_REPEAT;
    case ADDRESS_CLAMP:
      return GL_CLAMP_TO_EDGE;
    default:
      throw Exception("Invalid address mode");
  }
}

GLint convertFilterMode(FilterMode mode, bool mipmapped)
{
  switch (mode)
  {
    case FILTER_NEAREST:
    {
      if (mipmapped)
	return GL_NEAREST_MIPMAP_NEAREST;
      else
	return GL_NEAREST;
    }

    case FILTER_BILINEAR:
    {
      if (mipmapped)
	return GL_LINEAR_MIPMAP_NEAREST;
      else
	return GL_LINEAR;
    }

    case FILTER_TRILINEAR:
    {
      if (mipmapped)
	return GL_LINEAR_MIPMAP_LINEAR;
      else
	return GL_LINEAR;
    }

    default:
      throw Exception("Invalid filter mode");
  }
}

Bimap<PixelFormat, GLenum> formatMap;
Bimap<PixelFormat, GLenum> genericFormatMap;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

bool TextureImage::copyFrom(const moira::Image& source, unsigned int x, unsigned int y)
{
  /*
  moira::Image final = source;
  final.convertTo(texture.format);

  // Moira has y-axis down, OpenGL has y-axis up
  final.flipHorizontal();

  if (texture.textureTarget == GL_TEXTURE_1D)
  {
    if (final.getDimensionCount() > 1)
    {
      Log::writeError("Cannot blt to texture; source image has too many dimensions");
      return false;
    }

    // TODO: Stop using attribute stack.

    glPushAttrib(GL_TEXTURE_BIT | GL_PIXEL_MODE_BIT);
    glBindTexture(texture.textureTarget, texture.textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage1D(texture.textureTarget,
                    level,
		    x,
		    final.getWidth(),
                    genericFormatMap[texture.format],
                    GL_UNSIGNED_BYTE,
		    final.getPixels());

    glPopAttrib();
  }
  else
  {
    if (final.getDimensionCount() > 2)
    {
      Log::writeError("Cannot blt to texture; source image has too many dimensions");
      return false;
    }

    // TODO: Stop using attribute stack.

    glPushAttrib(GL_TEXTURE_BIT | GL_PIXEL_MODE_BIT);
    glBindTexture(texture.textureTarget, texture.textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage2D(texture.textureTarget,
                    level,
		    x, y,
		    final.getWidth(), final.getHeight(),
                    genericFormatMap[texture.format],
                    GL_UNSIGNED_BYTE,
		    final.getPixels());

    glPopAttrib();
  }

#if WENDY_DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Error during copy into level %u of texture \'%s\': %s",
                    level,
                    texture.getName().c_str(),
		    gluErrorString(error));
    return false;
  }
#endif
  */

  return true;
}

bool TextureImage::copyFromColorBuffer(unsigned int x, unsigned int y)
{
  cgGLSetManageTextureParameters((CGcontext) texture.context.cgContextID, CG_FALSE);

  glBindTexture(texture.textureTarget, texture.textureID);

  if (texture.textureTarget == GL_TEXTURE_1D)
  {
    glCopyTexSubImage1D(texture.textureTarget,
                        level,
			0,
                        x, y,
                        width);
  }
  else
  {
    glCopyTexSubImage2D(texture.textureTarget,
                        level,
			0, 0,
                        x, y,
                        width,
                        height);
  }

  cgGLSetManageTextureParameters((CGcontext) texture.context.cgContextID, CG_TRUE);

#if WENDY_DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Error during copy from color buffer to level %u of texture \'%s\': %s",
                    level,
                    texture.getName().c_str(),
		    gluErrorString(error));
    return false;
  }
#endif

  return true;
}

unsigned int TextureImage::getWidth(void) const
{
  return width;
}

unsigned int TextureImage::getHeight(void) const
{
  return height;
}

const PixelFormat& TextureImage::getFormat(void) const
{
  return texture.getFormat();
}

Texture& TextureImage::getTexture(void) const
{
  return texture;
}

TextureImage::TextureImage(Texture& initTexture,
                           unsigned int initLevel,
                           unsigned int initWidth,
                           unsigned int initHeight):
  texture(initTexture),
  level(initLevel),
  width(initWidth),
  height(initHeight)
{
}

void TextureImage::attach(int attachment)
{
  if (texture.textureTarget == GL_TEXTURE_1D)
    glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              texture.textureTarget,
                              texture.textureID,
                              level);
  else
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              texture.textureTarget,
                              texture.textureID,
                              level);
}

void TextureImage::detach(void)
{
}

///////////////////////////////////////////////////////////////////////

Texture::~Texture(void)
{
  if (textureID)
    glDeleteTextures(1, &textureID);
}

bool Texture::isPOT(void) const
{
  return (flags & RECTANGULAR) ? false : true;
}

bool Texture::isMipmapped(void) const
{
  return (flags & MIPMAPPED) ? true : false;
}

unsigned int Texture::getWidth(unsigned int level)
{
  return getImage(level).getWidth();
}

unsigned int Texture::getHeight(unsigned int level)
{
  return getImage(level).getHeight();
}

unsigned int Texture::getSourceWidth(void) const
{
  return sourceWidth;
}

unsigned int Texture::getSourceHeight(void) const
{
  return sourceHeight;
}

unsigned int Texture::getImageCount(void) const
{
  return images.size();
}

FilterMode Texture::getFilterMode(void) const
{
  return filterMode;
}

void Texture::setFilterMode(FilterMode newMode)
{
  if (newMode != filterMode)
  {
    cgGLSetManageTextureParameters((CGcontext) context.cgContextID, CG_FALSE);

    glBindTexture(textureTarget, textureID);

    glTexParameteri(textureTarget,
                    GL_TEXTURE_MIN_FILTER,
		    convertFilterMode(newMode, isMipmapped()));
    glTexParameteri(textureTarget,
                    GL_TEXTURE_MAG_FILTER,
		    convertFilterMode(newMode, false));

    filterMode = newMode;

    cgGLSetManageTextureParameters((CGcontext) context.cgContextID, CG_TRUE);
  }
}

AddressMode Texture::getAddressMode(void) const
{
  return addressMode;
}

void Texture::setAddressMode(AddressMode newMode)
{
  if (newMode != addressMode)
  {
    glPushAttrib(GL_TEXTURE_BIT);
    glBindTexture(textureTarget, textureID);

    GLint mode = convertAddressMode(addressMode);

    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, mode);

    if (textureTarget != GL_TEXTURE_1D)
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, mode);

    glPopAttrib();

    addressMode = newMode;
  }
}

const PixelFormat& Texture::getFormat(void) const
{
  return format;
}

TextureImage& Texture::getImage(unsigned int level)
{
  if (level >= images.size())
    throw Exception("Invalid mipmap level");

  return *images[level];
  /*
  if (getPhysicalWidth(level) == 0 || getPhysicalHeight(level) == 0)
  {
    Log::writeError("Cannot retrieve image for non-existent level %u of texture \'%s\'",
                    level,
		    getName().c_str());
    return NULL;
  }

  Ptr<moira::Image> result;

  result = new moira::Image(format, getPhysicalWidth(level), getPhysicalHeight(level));

  glPushAttrib(GL_TEXTURE_BIT | GL_PIXEL_MODE_BIT);
  glBindTexture(textureTarget, textureID);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glGetTexImage(textureTarget,
                level,
		genericFormatMap[format],
		GL_UNSIGNED_BYTE,
		result->getPixels());

  glPopAttrib();

#if WENDY_DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Error during texture image retrieval: %s", gluErrorString(error));
    return NULL;
  }
#endif

  result->flipHorizontal();

  return result.detachObject();
  */
}

Context& Texture::getContext(void) const
{
  return context;
}

Texture* Texture::createInstance(Context& context,
                                 const moira::Image& image,
				 unsigned int flags,
				 const String& name)
{
  Ptr<Texture> texture(new Texture(context, name));
  if (!texture->init(image, flags))
    return NULL;

  return texture.detachObject();
}

Texture::Texture(Context& initContext, const String& name):
  Resource<Texture>(name),
  context(initContext),
  textureTarget(0),
  textureID(0),
  sourceWidth(0),
  sourceHeight(0),
  flags(0),
  filterMode(FILTER_BILINEAR),
  addressMode(ADDRESS_WRAP)
{
}

Texture::Texture(const Texture& source):
  Resource<Texture>(""),
  context(source.context)
{
}

bool Texture::init(const moira::Image& image, unsigned int initFlags)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create texture without OpenGL context");
    return false;
  }

  if (formatMap.isEmpty())
  {
    formatMap[PixelFormat::R8] = GL_ALPHA8;
    formatMap[PixelFormat::RG8] = GL_LUMINANCE8_ALPHA8;
    formatMap[PixelFormat::RGB8] = GL_RGB8;
    formatMap[PixelFormat::RGBA8] = GL_RGBA8;
  }

  if (genericFormatMap.isEmpty())
  {
    genericFormatMap[PixelFormat::R8] = GL_ALPHA;
    genericFormatMap[PixelFormat::RG8] = GL_LUMINANCE_ALPHA;
    genericFormatMap[PixelFormat::RGB8] = GL_RGB;
    genericFormatMap[PixelFormat::RGBA8] = GL_RGBA;
  }

  if (!formatMap.hasKey(image.getFormat()))
  {
    Log::writeError("Source image for texture \'%s\' has unsupported format \'%s\'",
                    getName().c_str(),
                    image.getFormat().asString.c_str());
    return false;
  }

  flags = initFlags;

  // Figure out which texture target to use

  if (flags & RECTANGULAR)
  {
    if (image.getDimensionCount() > 2)
    {
      Log::writeError("Rectangular textures cannot have more than two dimensions");
      return false;
    }

    if (flags & MIPMAPPED)
    {
      Log::writeError("Rectangular textures cannot be mipmapped");
      return false;
    }

    textureTarget = GL_TEXTURE_RECTANGLE_ARB;
  }
  else
  {
    if (image.getDimensionCount() == 1)
      textureTarget = GL_TEXTURE_1D;
    else if (image.getDimensionCount() == 2)
      textureTarget = GL_TEXTURE_2D;
    else
    {
      // TODO: Support 3D textures

      Log::writeError("3D POT textures not supported by Wendy yet");
      return false;
    }
  }

  // Save source image dimensions
  sourceWidth = image.getWidth();
  sourceHeight = image.getHeight();

  unsigned int width, height;

  moira::Image source = image;

  // Adapt source image to OpenGL restrictions
  {
    format = source.getFormat();

    // Moira has y-axis down, OpenGL has y-axis up
    source.flipHorizontal();

    // Figure out target dimensions for rescaling

    if (flags & RECTANGULAR)
    {
      width = sourceWidth;
      height = sourceHeight;

      const unsigned int maxSize = GL::Context::get()->getLimits().getMaxTextureRectangleSize();

      if (width > maxSize)
	width = maxSize;

      if (height > maxSize)
	height = maxSize;
    }
    else
    {
      const unsigned int maxSize = GL::Context::get()->getLimits().getMaxTextureSize();

      width = getClosestPower(sourceWidth, maxSize);
      height = getClosestPower(sourceHeight, maxSize);
    }

    // Rescale source image (no-op if the sizes are equal)
    if (!source.resize(width, height))
    {
      Log::writeError("Failed to rescale image for texture \'%s\'", getName().c_str());
      return false;
    }
  }

  // Clear any errors
  glGetError();

  cgGLSetManageTextureParameters((CGcontext) context.cgContextID, CG_FALSE);

  // Contact space station
  glGenTextures(1, &textureID);
  glBindTexture(textureTarget, textureID);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  if (flags & MIPMAPPED)
  {
    if (textureTarget == GL_TEXTURE_1D)
    {
      gluBuild1DMipmaps(textureTarget,
                        format.getChannelCount(),
                        source.getWidth(),
                        genericFormatMap[source.getFormat()],
                        GL_UNSIGNED_BYTE,
                        source.getPixels());
    }
    else
    {
      gluBuild2DMipmaps(textureTarget,
                        format.getChannelCount(),
                        source.getWidth(),
                        source.getHeight(),
                        genericFormatMap[source.getFormat()],
                        GL_UNSIGNED_BYTE,
                        source.getPixels());
    }

    unsigned int level = 0;

    for (;;)
    {
      glGetTexLevelParameteriv(textureTarget, level, GL_TEXTURE_WIDTH, (int*) &width);
      glGetTexLevelParameteriv(textureTarget, level, GL_TEXTURE_HEIGHT, (int*) &height);

      if (width == 0)
        break;

      TextureImageRef image = new TextureImage(*this, level, width, height);
      images.push_back(image);

      level++;
    }
  }
  else
  {
    if (textureTarget == GL_TEXTURE_1D)
    {
      glTexImage1D(textureTarget,
                   0,
                   formatMap[source.getFormat()],
                   source.getWidth(),
                   0,
                   genericFormatMap[source.getFormat()],
                   GL_UNSIGNED_BYTE,
                   source.getPixels());
    }
    else
    {
      glTexImage2D(textureTarget,
                   0,
                   formatMap[source.getFormat()],
                   source.getWidth(),
                   source.getHeight(),
                   0,
                   genericFormatMap[source.getFormat()],
                   GL_UNSIGNED_BYTE,
                   source.getPixels());
    }

    TextureImageRef image = new TextureImage(*this, 0, source.getWidth(), source.getHeight());
    images.push_back(image);
  }

  if (flags & MIPMAPPED)
    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
  else
    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);

  if (textureTarget != GL_TEXTURE_1D)
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);

  cgGLSetManageTextureParameters((CGcontext) context.cgContextID, CG_TRUE);

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Error during creation of texture \'%s\': %s",
                    getName().c_str(),
		    gluErrorString(error));
    return false;
  }

  return true;
}

Texture& Texture::operator = (const Texture& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
