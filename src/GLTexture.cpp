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

#include <wendy/Config.h>

#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLTexture.h>
#include <wendy/GLContext.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLHelper.h>

#include <glm/gtx/bit.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

namespace
{

GLenum convertToGL(AddressMode mode)
{
  switch (mode)
  {
    case ADDRESS_WRAP:
      return GL_REPEAT;
    case ADDRESS_CLAMP:
      return GL_CLAMP_TO_EDGE;
  }

  panic("Invalid texture address mode %u", mode);
}

GLint convertToGL(FilterMode mode, bool mipmapped)
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
  }

  panic("Invalid texture filter mode %u", mode);
}

GLenum convertToProxyGL(TextureType type)
{
  switch (type)
  {
    case TEXTURE_1D:
      return GL_PROXY_TEXTURE_1D;
    case TEXTURE_2D:
      return GL_PROXY_TEXTURE_2D;
    case TEXTURE_3D:
      return GL_PROXY_TEXTURE_3D;
    case TEXTURE_RECT:
      return GL_PROXY_TEXTURE_RECTANGLE_ARB;
    case TEXTURE_CUBE:
      return GL_PROXY_TEXTURE_CUBE_MAP;
  }

  panic("Invalid texture type %u", type);
}

GLenum convertToGL(CubeFace face)
{
  switch (face)
  {
    case CUBE_POSITIVE_X:
      return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    case CUBE_NEGATIVE_X:
      return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
    case CUBE_POSITIVE_Y:
      return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
    case CUBE_NEGATIVE_Y:
      return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
    case CUBE_POSITIVE_Z:
      return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
    case CUBE_NEGATIVE_Z:
      return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
  }

  panic("Invalid image cube face %u", face);
}

const char* asString(TextureType type)
{
  switch (type)
  {
    case TEXTURE_1D:
      return "texture1D";
    case TEXTURE_2D:
      return "texture2D";
    case TEXTURE_3D:
      return "texture3D";
    case TEXTURE_RECT:
      return "textureRECT";
    case TEXTURE_CUBE:
      return "textureCube";
  }

  panic("Invalid texture type %u", type);
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

TextureParams::TextureParams(TextureType initType):
  type(initType),
  mipmapped(true),
  sRGB(false)
{
  if (type == TEXTURE_RECT)
    mipmapped = false;
}

///////////////////////////////////////////////////////////////////////

bool TextureImage::copyFrom(const wendy::Image& source,
                            unsigned int x,
                            unsigned int y,
                            unsigned int z)
{
  if (source.getFormat() != texture.format)
  {
    // TODO: Convert to compatible pixel format

    logError("Cannot copy texture data from source image of different pixel format");
    return false;
  }

  if (texture.is1D())
  {
    if (source.getDimensionCount() > 1)
    {
      logError("Cannot blt to texture; source image has too many dimensions");
      return false;
    }

    texture.context.setCurrentTexture(&texture);

    glTexSubImage1D(convertToGL(texture.type),
                    level,
                    x,
                    source.getWidth(),
                    convertToGL(texture.format.getSemantic()),
                    convertToGL(texture.format.getType()),
                    source.getPixels());
  }
  else if (texture.is3D())
  {
    texture.context.setCurrentTexture(&texture);

    glTexSubImage3D(convertToGL(texture.type),
                    level,
                    x, y, z,
                    source.getWidth(),
                    source.getHeight(),
                    source.getDepth(),
                    convertToGL(texture.format.getSemantic()),
                    convertToGL(texture.format.getType()),
                    source.getPixels());
  }
  else
  {
    if (source.getDimensionCount() > 2)
    {
      logError("Cannot blt to texture; source image has too many dimensions");
      return false;
    }

    texture.context.setCurrentTexture(&texture);

    glTexSubImage2D(convertToGL(texture.type),
                    level,
                    x, y,
                    source.getWidth(), source.getHeight(),
                    convertToGL(texture.format.getSemantic()),
                    convertToGL(texture.format.getType()),
                    source.getPixels());
  }

#if WENDY_DEBUG
  if (!checkGL("Error during copy from image into level %u of texture \'%s\'",
               level,
               texture.getName().c_str()))
  {
    return false;
  }
#endif

  return true;
}

Ref<wendy::Image> TextureImage::getData() const
{
  Ref<wendy::Image> result = wendy::Image::create(texture.getCache(),
                                                  texture.format,
                                                  width,
                                                  height,
                                                  depth);

  texture.context.setCurrentTexture(&texture);

  glGetTexImage(convertToGL(texture.type),
                level,
                convertToGL(texture.format.getSemantic()),
                convertToGL(texture.format.getType()),
                result->getPixels());

#if WENDY_DEBUG
  if (!checkGL("Error during copy to image from level %u of texture \'%s\'",
               level,
               texture.getName().c_str()))
  {
    return NULL;
  }
#endif

  return result;
}

unsigned int TextureImage::getWidth() const
{
  return width;
}

unsigned int TextureImage::getHeight() const
{
  return height;
}

unsigned int TextureImage::getDepth() const
{
  return depth;
}

CubeFace TextureImage::getFace() const
{
  return face;
}

const PixelFormat& TextureImage::getFormat() const
{
  return texture.getFormat();
}

Texture& TextureImage::getTexture() const
{
  return texture;
}

TextureImage::TextureImage(Texture& initTexture,
                           unsigned int initLevel,
                           unsigned int initWidth,
                           unsigned int initHeight,
                           unsigned int initDepth,
                           CubeFace initFace):
  texture(initTexture),
  level(initLevel),
  width(initWidth),
  height(initHeight),
  depth(initDepth),
  face(initFace)
{
}

void TextureImage::attach(int attachment, unsigned int z)
{
  if (texture.is1D())
  {
    glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              convertToGL(texture.type),
                              texture.textureID,
                              level);
  }
  else if (texture.is3D())
  {
    glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              convertToGL(texture.type),
                              texture.textureID,
                              level,
                              z);
  }
  else
  {
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              convertToGL(texture.type),
                              texture.textureID,
                              level);
  }

#if WENDY_DEBUG
  checkGL("Error when attaching level %u of texture \'%s\' to framebuffer",
          level,
          texture.getName().c_str());
#endif
}

void TextureImage::detach(int attachment)
{
  if (texture.is1D())
  {
    glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              convertToGL(texture.type),
                              0, 0);
  }
  else if (texture.is3D())
  {
    glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              convertToGL(texture.type),
                              0, 0, 0);
  }
  else
  {
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              convertToGL(texture.type),
                              0, 0);
  }

#if WENDY_DEBUG
  checkGL("Error when detaching level %u of texture \'%s\' from framebuffer",
          level,
          texture.getName().c_str());
#endif
}

///////////////////////////////////////////////////////////////////////

Texture::~Texture()
{
  if (textureID)
    glDeleteTextures(1, &textureID);

  if (Stats* stats = context.getStats())
    stats->removeTexture(getSize());
}

void Texture::generateMipmaps()
{
  glGenerateMipmapEXT(convertToGL(type));

  if (!hasMipmaps())
  {
    retrieveImages();

    glTexParameteri(convertToGL(type),
                    GL_TEXTURE_MIN_FILTER,
                    convertToGL(filterMode, true));
  }
}

bool Texture::is1D() const
{
  return type == TEXTURE_1D;
}

bool Texture::is2D() const
{
  return type == TEXTURE_2D || TEXTURE_RECT;
}

bool Texture::is3D() const
{
  return type == TEXTURE_3D;
}

bool Texture::isCube() const
{
  return type == TEXTURE_CUBE;
}

bool Texture::isPOT() const
{
  return isPowerOfTwo(getWidth()) &&
         isPowerOfTwo(getHeight()) &&
         isPowerOfTwo(getDepth());
}

bool Texture::hasMipmaps() const
{
  return levels > 1;
}

TextureType Texture::getType() const
{
  return type;
}

unsigned int Texture::getWidth(unsigned int level) const
{
  return getImage(level).getWidth();
}

unsigned int Texture::getHeight(unsigned int level) const
{
  return getImage(level).getHeight();
}

unsigned int Texture::getDepth(unsigned int level) const
{
  return getImage(level).getDepth();
}

unsigned int Texture::getLevelCount() const
{
  return levels;
}

FilterMode Texture::getFilterMode() const
{
  return filterMode;
}

void Texture::setFilterMode(FilterMode newMode)
{
  if (newMode != filterMode)
  {
    context.setCurrentTexture(this);

    glTexParameteri(convertToGL(type),
                    GL_TEXTURE_MIN_FILTER,
                    convertToGL(newMode, hasMipmaps()));
    glTexParameteri(convertToGL(type),
                    GL_TEXTURE_MAG_FILTER,
                    convertToGL(newMode, false));

    filterMode = newMode;
  }

#if WENDY_DEBUG
  checkGL("Error when changing filter mode for texture \'%s\'",
          getName().c_str());
#endif
}

AddressMode Texture::getAddressMode() const
{
  return addressMode;
}

void Texture::setAddressMode(AddressMode newMode)
{
  if (type == TEXTURE_RECT)
  {
    if (newMode != ADDRESS_CLAMP)
    {
      logError("Rectangular textures only support ADDRESS_CLAMP");
      return;
    }
  }

  if (newMode != addressMode)
  {
    context.setCurrentTexture(this);

    glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_S, convertToGL(newMode));
    glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_T, convertToGL(newMode));
    glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_R, convertToGL(newMode));

    addressMode = newMode;
  }

#if WENDY_DEBUG
  checkGL("Error when changing address mode for texture \'%s\'",
          getName().c_str());
#endif
}

float Texture::getMaxAnisotropy() const
{
  return maxAnisotropy;
}

void Texture::setMaxAnisotropy(float newMax)
{
  if (newMax != maxAnisotropy)
  {
    if (!GLEW_EXT_texture_filter_anisotropic)
    {
      logError("Cannot set max anisotropy: "
               "GL_EXT_texture_filter_anisotropic is missing");
      return;
    }

    context.setCurrentTexture(this);

    glTexParameteri(convertToGL(type), GL_TEXTURE_MAX_ANISOTROPY_EXT, int(newMax));

    maxAnisotropy = newMax;
  }

#if WENDY_DEBUG
  checkGL("Error when changing max anisotropy for texture \'%s\'",
          getName().c_str());
#endif
}

const PixelFormat& Texture::getFormat() const
{
  return format;
}

size_t Texture::getSize() const
{
  size_t size = 0;

  for (ImageList::const_iterator i = images.begin();  i != images.end();  i++)
    size += (*i)->getSize();

  return size;
}

TextureImage& Texture::getImage(unsigned int level, CubeFace face)
{
  if (isCube())
    return *images[face * levels + level];
  else
    return *images[level];
}

const TextureImage& Texture::getImage(unsigned int level, CubeFace face) const
{
  if (isCube())
    return *images[face * levels + level];
  else
    return *images[level];
}

Context& Texture::getContext() const
{
  return context;
}

Ref<Texture> Texture::create(const ResourceInfo& info,
                             Context& context,
                             const TextureParams& params,
                             const wendy::Image& data)
{
  Ref<Texture> texture(new Texture(info, context));
  if (!texture->init(params, data))
    return NULL;

  return texture;
}

Ref<Texture> Texture::read(Context& context,
                           const TextureParams& params,
                           const String& imageName)
{
  ResourceCache& cache = context.getCache();

  String name;
  name += "source:";
  name += imageName;
  name += " mipmapped:";
  name += params.mipmapped ? "true" : "false";
  name += " sRGB:";
  name += params.sRGB ? "true" : "false";

  if (Ref<Texture> texture = cache.find<Texture>(name))
    return texture;

  Ref<wendy::Image> data = wendy::Image::read(cache, imageName);
  if (!data)
  {
    logError("Failed to read image for texture \'%s\'", name.c_str());
    return NULL;
  }

  return create(ResourceInfo(cache, name), context, params, *data);
}

Texture::Texture(const ResourceInfo& info, Context& initContext):
  Resource(info),
  context(initContext),
  textureID(0),
  levels(0),
  filterMode(FILTER_BILINEAR),
  addressMode(ADDRESS_WRAP),
  maxAnisotropy(1.f)
{
}

Texture::Texture(const Texture& source):
  Resource(source),
  context(source.context)
{
}

bool Texture::init(const TextureParams& params, const wendy::Image& data)
{
  format = data.getFormat();

  if (!convertToGL(format, params.sRGB))
  {
    logError("Source image for texture \'%s\' has unsupported pixel format \'%s\'",
             getName().c_str(),
             format.asString().c_str());
    return false;
  }

  // Figure out which texture target to use

  if (params.type == TEXTURE_RECT)
  {
    if (data.getDimensionCount() > 2)
    {
      logError("Source image for rectangular texture \'%s\' has more than two dimensions",
               getName().c_str());
      return false;
    }

    if (params.mipmapped)
    {
      logError("Texture \'%s\' specified as both rectangular and mipmapped",
               getName().c_str());
      return false;
    }
  }
  else if (params.type == TEXTURE_CUBE)
  {
    if (data.getDimensionCount() > 2)
    {
      logError("Source image for cubemap texture \'%s\' has more than two dimensions",
               getName().c_str());
      return false;
    }

    const unsigned int width = data.getWidth();

    if (data.getWidth() % 6 != 0 ||
        data.getWidth() / 6 != data.getHeight() ||
        !isPowerOfTwo(data.getHeight()))
    {
      logError("Source image for cubemap texture \'%s\' has invalid dimensions",
               getName().c_str());
      return false;
    }
  }
  else
  {
    if (!data.isPOT())
    {
      logWarning("Texture \'%s\' does not have power-of-two dimensions; this may cause slowdown",
                 getName().c_str());
    }
  }

  type = params.type;

  unsigned int width, height, depth;

  if (type == TEXTURE_CUBE)
  {
    width = data.getWidth() / 6;
    height = data.getHeight();
    depth = 1;
  }
  else
  {
    width = data.getWidth();
    height = data.getHeight();
    depth = data.getDepth();
  }

  if (type == TEXTURE_1D)
  {
    glTexImage1D(convertToProxyGL(type),
                 0,
                 convertToGL(format, params.sRGB),
                 width,
                 0,
                 convertToGL(format.getSemantic()),
                 convertToGL(format.getType()),
                 NULL);
  }
  else if (type == TEXTURE_3D)
  {
    glTexImage3D(convertToProxyGL(type),
                 0,
                 convertToGL(format, params.sRGB),
                 width,
                 height,
                 depth,
                 0,
                 convertToGL(format.getSemantic()),
                 convertToGL(format.getType()),
                 NULL);
  }
  else
  {
    glTexImage2D(convertToProxyGL(type),
                 0,
                 convertToGL(format, params.sRGB),
                 width,
                 height,
                 0,
                 convertToGL(format.getSemantic()),
                 convertToGL(format.getType()),
                 NULL);
  }

  GLint proxyWidth;
  glGetTexLevelParameteriv(convertToProxyGL(type),
                           0,
                           GL_TEXTURE_WIDTH,
                           &proxyWidth);

  if (proxyWidth == 0)
  {
    logError("Cannot create texture \'%s\' type \'%s\' size %ux%ux%u format \'%s\'",
              getName().c_str(),
              asString(type),
              width, height, depth,
              format.asString().c_str());

    return false;
  }

  glGenTextures(1, &textureID);

  context.setCurrentTexture(this);

  if (type == TEXTURE_1D)
  {
    glTexImage1D(convertToGL(type),
                 0,
                 convertToGL(format, params.sRGB),
                 width,
                 0,
                 convertToGL(format.getSemantic()),
                 convertToGL(format.getType()),
                 data.getPixels());
  }
  else if (type == TEXTURE_3D)
  {
    glTexImage3D(convertToGL(type),
                 0,
                 convertToGL(format, params.sRGB),
                 width, height, depth,
                 0,
                 convertToGL(format.getSemantic()),
                 convertToGL(format.getType()),
                 data.getPixels());
  }
  else if (type == TEXTURE_CUBE)
  {
    const CubeFace faces[] =
    {
      CUBE_NEGATIVE_Z,
      CUBE_NEGATIVE_X,
      CUBE_POSITIVE_Z,
      CUBE_POSITIVE_X,
      CUBE_POSITIVE_Y,
      CUBE_NEGATIVE_Y
    };

    glPixelStorei(GL_UNPACK_ROW_LENGTH, data.getWidth());

    for (size_t i = 0;  i < 6;  i++)
    {
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, i * width);

      glTexImage2D(convertToGL(faces[i]),
                   0,
                   convertToGL(data.getFormat(), params.sRGB),
                   width, height,
                   0,
                   convertToGL(data.getFormat().getSemantic()),
                   convertToGL(data.getFormat().getType()),
                   data.getPixels());
    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  }
  else
  {
    glTexImage2D(convertToGL(type),
                 0,
                 convertToGL(format, params.sRGB),
                 width, height,
                 0,
                 convertToGL(format.getSemantic()),
                 convertToGL(format.getType()),
                 data.getPixels());
  }

  if (params.mipmapped)
    generateMipmaps();
  else
    retrieveImages();

  applyDefaults();

  if (!checkGL("OpenGL error during creation of texture \'%s\' format \'%s\'",
               getName().c_str(),
               format.asString().c_str()))
  {
    return false;
  }

  if (Stats* stats = context.getStats())
    stats->addTexture(getSize());

  return true;
}

void Texture::retrieveImages()
{
  images.clear();

  if (type == TEXTURE_CUBE)
  {
    for (size_t i = 0;  i < 6;  i++)
      levels = retrieveTargetImages(convertToGL(CubeFace(i)), CubeFace(i));
  }
  else
    levels = retrieveTargetImages(convertToGL(type), NO_CUBE_FACE);
}

unsigned int Texture::retrieveTargetImages(unsigned int target, CubeFace face)
{
  unsigned int level = 0;

  for (;;)
  {
    unsigned int width, height, depth;

    glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, (int*) &width);
    glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, (int*) &height);
    glGetTexLevelParameteriv(target, level, GL_TEXTURE_DEPTH, (int*) &depth);

    if (width == 0)
      break;

    images.push_back(new TextureImage(*this, level, width, height, depth, face));

    level++;
  }

  return level;
}

void Texture::applyDefaults()
{
  filterMode = FILTER_BILINEAR;

  glTexParameteri(convertToGL(type),
                  GL_TEXTURE_MIN_FILTER,
                  convertToGL(filterMode, hasMipmaps()));
  glTexParameteri(convertToGL(type),
                  GL_TEXTURE_MAG_FILTER,
                  convertToGL(filterMode, false));

  addressMode = ADDRESS_CLAMP;

  glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_S, convertToGL(addressMode));
  glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_T, convertToGL(addressMode));
  glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_R, convertToGL(addressMode));
}

Texture& Texture::operator = (const Texture& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
