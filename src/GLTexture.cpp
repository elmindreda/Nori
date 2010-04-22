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
Bimap<PixelFormat::Type, GLenum> typeMap;
Bimap<ImageCube::Face, GLenum> faceMap;
Bimap<AddressMode, GLenum> addressMap;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

bool TextureImage::copyFrom(const moira::Image& source, unsigned int x, unsigned int y)
{
  moira::Image final = source;

  if (final.getFormat() != texture.format)
  {
    // TODO: Convert to compatible pixel format

    Log::writeError("Cannot copy texture data from source image of different pixel format");
    return false;
  }

  if (texture.textureTarget == GL_TEXTURE_1D)
  {
    if (final.getDimensionCount() > 1)
    {
      Log::writeError("Cannot blt to texture; source image has too many dimensions");
      return false;
    }

    cgGLSetManageTextureParameters((CGcontext) texture.context.cgContextID, CG_FALSE);

    glBindTexture(texture.textureTarget, texture.textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage1D(texture.textureTarget,
                    level,
		    x,
		    final.getWidth(),
                    genericFormatMap[texture.format],
                    GL_UNSIGNED_BYTE,
		    final.getPixels());

    cgGLSetManageTextureParameters((CGcontext) texture.context.cgContextID, CG_TRUE);
  }
  else
  {
    if (final.getDimensionCount() > 2)
    {
      Log::writeError("Cannot blt to texture; source image has too many dimensions");
      return false;
    }

    cgGLSetManageTextureParameters((CGcontext) texture.context.cgContextID, CG_FALSE);

    glBindTexture(texture.textureTarget, texture.textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage2D(texture.textureTarget,
                    level,
		    x, y,
		    final.getWidth(), final.getHeight(),
                    genericFormatMap[texture.format],
                    GL_UNSIGNED_BYTE,
		    final.getPixels());

    cgGLSetManageTextureParameters((CGcontext) texture.context.cgContextID, CG_TRUE);
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

bool TextureImage::copyTo(moira::Image& result) const
{
  result = moira::Image(texture.format, width, height);

  cgGLSetManageTextureParameters((CGcontext) texture.context.cgContextID, CG_FALSE);

  glBindTexture(texture.textureTarget, texture.textureID);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glGetTexImage(texture.textureTarget,
                level,
		genericFormatMap[texture.format],
		GL_UNSIGNED_BYTE,
		result.getPixels());

  cgGLSetManageTextureParameters((CGcontext) texture.context.cgContextID, CG_TRUE);

#if WENDY_DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Error during texture image retrieval: %s", gluErrorString(error));
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

#if WENDY_DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeError("Error when applying texture image: %s", gluErrorString(error));
#endif
}

void TextureImage::detach(int attachment)
{
  if (texture.textureTarget == GL_TEXTURE_1D)
    glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              texture.textureTarget,
                              0,
                              0);
  else
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              texture.textureTarget,
                              0,
                              0);

#if WENDY_DEBUG
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeError("Error when applying texture image: %s", gluErrorString(error));
#endif
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

bool Texture::isCubeMap(void) const
{
  return textureTarget == GL_TEXTURE_CUBE_MAP;
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
    if (!addressMap.hasKey(newMode))
      return;

    cgGLSetManageTextureParameters((CGcontext) context.cgContextID, CG_FALSE);

    glBindTexture(textureTarget, textureID);
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, addressMap[newMode]);

    if (textureTarget != GL_TEXTURE_1D)
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, addressMap[newMode]);

    cgGLSetManageTextureParameters((CGcontext) context.cgContextID, CG_TRUE);

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
}

Context& Texture::getContext(void) const
{
  return context;
}

Texture* Texture::createInstance(Context& context,
                                 const moira::Image& source,
				 unsigned int flags,
				 const String& name)
{
  Ptr<Texture> texture(new Texture(context, name));
  if (!texture->init(source, flags))
    return NULL;

  return texture.detachObject();
}

Texture* Texture::createInstance(Context& context,
                                 const ImageCube& source,
				 unsigned int flags,
				 const String& name)
{
  Ptr<Texture> texture(new Texture(context, name));
  if (!texture->init(source, flags))
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

bool Texture::init(void)
{
  if (formatMap.isEmpty())
  {
    formatMap[PixelFormat::R8] = GL_ALPHA8;
    formatMap[PixelFormat::RG8] = GL_LUMINANCE8_ALPHA8;
    formatMap[PixelFormat::RGB8] = GL_RGB8;
    formatMap[PixelFormat::RGBA8] = GL_RGBA8;
    formatMap[PixelFormat::DEPTH16] = GL_DEPTH_COMPONENT16;
    formatMap[PixelFormat::DEPTH24] = GL_DEPTH_COMPONENT24;
    formatMap[PixelFormat::DEPTH32] = GL_DEPTH_COMPONENT32;
  }

  if (genericFormatMap.isEmpty())
  {
    genericFormatMap[PixelFormat::R8] = GL_ALPHA;
    genericFormatMap[PixelFormat::RG8] = GL_LUMINANCE_ALPHA;
    genericFormatMap[PixelFormat::RGB8] = GL_RGB;
    genericFormatMap[PixelFormat::RGBA8] = GL_RGBA;
    genericFormatMap[PixelFormat::DEPTH16] = GL_DEPTH_COMPONENT;
    genericFormatMap[PixelFormat::DEPTH24] = GL_DEPTH_COMPONENT;
    genericFormatMap[PixelFormat::DEPTH32] = GL_DEPTH_COMPONENT;
  }

  if (typeMap.isEmpty())
  {
    typeMap[PixelFormat::UINT8] = GL_UNSIGNED_BYTE;
    typeMap[PixelFormat::UINT16] = GL_UNSIGNED_SHORT;
    typeMap[PixelFormat::UINT32] = GL_UNSIGNED_INT;
  }

  if (faceMap.isEmpty())
  {
    faceMap[ImageCube::POSITIVE_X] = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    faceMap[ImageCube::NEGATIVE_X] = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
    faceMap[ImageCube::POSITIVE_Y] = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
    faceMap[ImageCube::NEGATIVE_Y] = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
    faceMap[ImageCube::POSITIVE_Z] = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
    faceMap[ImageCube::NEGATIVE_Z] = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
  }

  if (addressMap.isEmpty())
  {
    addressMap[ADDRESS_WRAP] = GL_REPEAT;
    addressMap[ADDRESS_CLAMP] = GL_CLAMP_TO_EDGE;
  }

  return true;
}

bool Texture::init(const moira::Image& source, unsigned int initFlags)
{
  if (!init())
    return false;

  if (!formatMap.hasKey(source.getFormat()))
  {
    Log::writeError("Source image for texture \'%s\' has unsupported pixel format \'%s\'",
                    getName().c_str(),
                    source.getFormat().asString().c_str());
    return false;
  }

  if (!typeMap.hasKey(source.getFormat().getType()))
  {
    Log::writeError("Source image for texture \'%s\' has unsupported component type",
                    getName().c_str());
    return false;
  }

  flags = initFlags;

  // Figure out which texture target to use

  if (flags & RECTANGULAR)
  {
    if (source.getDimensionCount() > 2)
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
    if (source.getDimensionCount() == 1)
      textureTarget = GL_TEXTURE_1D;
    else if (source.getDimensionCount() == 2)
      textureTarget = GL_TEXTURE_2D;
    else
    {
      // TODO: Support 3D textures

      Log::writeError("3D POT textures not supported by Wendy yet");
      return false;
    }
  }

  // Save source image dimensions
  sourceWidth = source.getWidth();
  sourceHeight = source.getHeight();

  unsigned int width, height;

  moira::Image final = source;

  // Adapt source image to OpenGL restrictions
  {
    format = final.getFormat();

    // Figure out target dimensions for rescaling

    if (flags & RECTANGULAR)
    {
      width = sourceWidth;
      height = sourceHeight;

      const unsigned int maxSize = context.getLimits().getMaxTextureRectangleSize();

      if (width > maxSize)
	width = maxSize;

      if (height > maxSize)
	height = maxSize;
    }
    else
    {
      const unsigned int maxSize = context.getLimits().getMaxTextureSize();

      width = getClosestPower(sourceWidth, maxSize);
      height = getClosestPower(sourceHeight, maxSize);
    }

    // Rescale source image (no-op if the sizes are equal)
    if (!final.resize(width, height))
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

  if (textureTarget == GL_TEXTURE_1D)
  {
    glTexImage1D(textureTarget,
                  0,
                  formatMap[final.getFormat()],
                  final.getWidth(),
                  0,
                  genericFormatMap[final.getFormat()],
                  typeMap[final.getFormat().getType()],
                  final.getPixels());
  }
  else
  {
    glTexImage2D(textureTarget,
                  0,
                  formatMap[final.getFormat()],
                  final.getWidth(),
                  final.getHeight(),
                  0,
                  genericFormatMap[final.getFormat()],
                  typeMap[final.getFormat().getType()],
                  final.getPixels());
  }

  if (flags & MIPMAPPED)
  {
    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glGenerateMipmapEXT(textureTarget);

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
    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    TextureImageRef image = new TextureImage(*this, 0, final.getWidth(), final.getHeight());
    images.push_back(image);
  }

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

bool Texture::init(const ImageCube& source, unsigned int initFlags)
{
  if (!init())
    return false;

  // Check image dimensions
  {
    if (!source.isPOT())
    {
      Log::writeError("Source images for texture \'%s\' do not have POT dimensions",
                      getName().c_str());
      return false;
    }

    if (!source.isSquare())
    {
      Log::writeError("Source images for texture \'%s\' are not square",
                      getName().c_str());
      return false;
    }

    if (!source.hasSameSize())
    {
      Log::writeError("Source images for texture \'%s\' do not have the same size",
                      getName().c_str());
      return false;
    }

    sourceWidth = source.images[0]->getWidth();
    sourceHeight = source.images[0]->getHeight();

    const unsigned int maxSize = context.getLimits().getMaxTextureCubeSize();

    if (sourceWidth > maxSize)
    {
      Log::writeError("Source images for texture \'%s\' are too large; maximum size is %ux%u",
                      getName().c_str(),
                      maxSize, maxSize);
      return false;
    }
  }

  // Check image formats
  {
    if (!source.hasSameFormat())
    {
      Log::writeError("Source images for texture \'%s\' do not have same format",
                      getName().c_str());
      return false;
    }

    format = source.images[0]->getFormat();

    if (!formatMap.hasKey(format))
    {
      Log::writeError("Source images for texture \'%s\' have unsupported pixel format \'%s\'",
                      getName().c_str(),
                      format.asString().c_str());
      return false;
    }

    if (!typeMap.hasKey(format.getType()))
    {
      Log::writeError("Source images for texture \'%s\' have unsupported component type",
                      getName().c_str());
      return false;
    }
  }

  // Check creation flags
  {
    flags = initFlags;

    if (flags & RECTANGULAR)
    {
      Log::writeError("Invalid flags for texture \'%s\': cube maps cannot be rectangular",
                      getName().c_str());
      return false;
    }
  }

  textureTarget = GL_TEXTURE_CUBE_MAP;

  // Clear any errors
  glGetError();

  cgGLSetManageTextureParameters((CGcontext) context.cgContextID, CG_FALSE);

  // Reimburse hamster wheel
  glGenTextures(1, &textureID);
  glBindTexture(textureTarget, textureID);

  for (unsigned int i = 0;  i < 6;  i++)
  {
    GLenum faceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;

    moira::Image& image = *source.images[i];

    glTexImage2D(faceTarget,
                 0,
                 formatMap[image.getFormat()],
                 image.getWidth(),
                 image.getHeight(),
                 0,
                 genericFormatMap[image.getFormat()],
                 typeMap[image.getFormat().getType()],
                 image.getPixels());
  }

  if (flags & MIPMAPPED)
  {
    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glGenerateMipmapEXT(textureTarget);

    unsigned int level = 0;

    for (;;)
    {
      unsigned int width, height;

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
    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    unsigned int width, height;

    glGetTexLevelParameteriv(textureTarget, 0, GL_TEXTURE_WIDTH, (int*) &width);
    glGetTexLevelParameteriv(textureTarget, 0, GL_TEXTURE_HEIGHT, (int*) &height);

    TextureImageRef image = new TextureImage(*this, 0, width, height);
    images.push_back(image);
  }

  glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
