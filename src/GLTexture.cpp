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

#include <wendy/Bimap.h>

#include <wendy/OpenGL.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLTexture.h>
#include <wendy/GLContext.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLConvert.h>

#include <glm/gtx/bit.hpp>

#include <pugixml.hpp>

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

Bimap<String, FilterMode> filterModeMap;
Bimap<String, AddressMode> addressModeMap;

const unsigned int TEXTURE_XML_VERSION = 1;

} /*namespace*/

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
               texture.getPath().asString().c_str()))
  {
    return false;
  }
#endif

  return true;
}

bool TextureImage::copyTo(wendy::Image& result) const
{
  result = wendy::Image(texture.getCache(), texture.format, width, height, depth);

  texture.context.setCurrentTexture(&texture);

  glGetTexImage(convertToGL(texture.type),
                level,
                convertToGL(texture.format.getSemantic()),
                convertToGL(texture.format.getType()),
                result.getPixels());

#if WENDY_DEBUG
  if (!checkGL("Error during copy to image from level %u of texture \'%s\'",
               level,
               texture.getPath().asString().c_str()))
  {
    return false;
  }
#endif

  return true;
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
          texture.getPath().asString().c_str());
#endif
}

void TextureImage::detach(int attachment)
{
  if (texture.is1D())
  {
    glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              convertToGL(texture.type),
                              0,
                              0);
  }
  else if (texture.is3D())
  {
    glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              convertToGL(texture.type),
                              0,
                              0,
                              0);
  }
  else
  {
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              convertToGL(texture.type),
                              0,
                              0);
  }

#if WENDY_DEBUG
  checkGL("Error when detaching level %u of texture \'%s\' from framebuffer",
          level,
          texture.getPath().asString().c_str());
#endif
}

///////////////////////////////////////////////////////////////////////

Texture::~Texture()
{
  if (textureID)
    glDeleteTextures(1, &textureID);
}

void Texture::generateMipmaps()
{
  glGenerateMipmapEXT(convertToGL(type));
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
  return isPowerOfTwo(width) && isPowerOfTwo(height) && isPowerOfTwo(depth);
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
  return width;
}

unsigned int Texture::getHeight(unsigned int level) const
{
  return height;
}

unsigned int Texture::getDepth(unsigned int level) const
{
  return depth;
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
          getPath().asString().c_str());
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

    if (type != TEXTURE_1D)
      glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_T, convertToGL(newMode));

    addressMode = newMode;
  }

#if WENDY_DEBUG
  checkGL("Error when changing address mode for texture \'%s\'",
          getPath().asString().c_str());
#endif
}

const PixelFormat& Texture::getFormat() const
{
  return format;
}

TextureImage* Texture::getImage(unsigned int level, CubeFace face)
{
  if (isCube())
    return images[face * levels + level];
  else
    return images[level];
}

Context& Texture::getContext() const
{
  return context;
}

Ref<Texture> Texture::create(const ResourceInfo& info,
                             Context& context,
                             const wendy::Image& source,
                             unsigned int flags)
{
  Ref<Texture> texture(new Texture(info, context));
  if (!texture->init(source, flags))
    return NULL;

  return texture;
}

Ref<Texture> Texture::create(const ResourceInfo& info,
                             Context& context,
                             const ImageCube& source,
                             unsigned int flags)
{
  Ref<Texture> texture(new Texture(info, context));
  if (!texture->init(source, flags))
    return NULL;

  return texture;
}

Ref<Texture> Texture::read(Context& context, const Path& path)
{
  TextureReader reader(context);
  return reader.read(path);
}

Texture::Texture(const ResourceInfo& info, Context& initContext):
  Resource(info),
  context(initContext),
  textureID(0),
  width(0),
  height(0),
  depth(0),
  levels(0),
  filterMode(FILTER_BILINEAR),
  addressMode(ADDRESS_WRAP)
{
}

Texture::Texture(const Texture& source):
  Resource(source),
  context(source.context)
{
}

bool Texture::init(const wendy::Image& source, unsigned int flags)
{
  format = source.getFormat();

  if (!convertToGL(format))
  {
    logError("Source image for texture \'%s\' has unsupported pixel format \'%s\'",
             getPath().asString().c_str(),
             format.asString().c_str());
    return false;
  }

  // Figure out which texture target to use

  if (flags & RECTANGULAR)
  {
    if (source.getDimensionCount() > 2)
    {
      logError("Source image for rectangular texture \'%s\' texture has more than two dimensions",
               getPath().asString().c_str());
      return false;
    }

    if (flags & MIPMAPPED)
    {
      logError("Rectangular textures cannot be mipmapped");
      return false;
    }

    type = TEXTURE_RECT;
  }
  else
  {
    if (!source.isPOT())
    {
      logWarning("Texture \'%s\' does not have power-of-two dimensions; this may cause slowdown",
                 getPath().asString().c_str());
    }

    if (source.getDimensionCount() == 1)
      type = TEXTURE_1D;
    else if (source.getDimensionCount() == 2)
      type = TEXTURE_2D;
    else
      type = TEXTURE_3D;
  }

  // Save source image dimensions
  width = source.getWidth();
  height = source.getHeight();
  depth = source.getDepth();

  if (type == TEXTURE_1D)
  {
    glTexImage1D(convertToProxyGL(type),
                 0,
                 convertToGL(format),
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
                 convertToGL(format),
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
                 convertToGL(format),
                 width,
                 height,
                 0,
                 convertToGL(format.getSemantic()),
                 convertToGL(format.getType()),
                 NULL);
  }

  if (!validateProxy())
    return false;

  glGenTextures(1, &textureID);

  context.setCurrentTexture(this);

  if (type == TEXTURE_1D)
  {
    glTexImage1D(convertToGL(type),
                 0,
                 convertToGL(format),
                 width,
                 0,
                 convertToGL(format.getSemantic()),
                 convertToGL(format.getType()),
                 source.getPixels());
  }
  else if (type == TEXTURE_3D)
  {
    glTexImage3D(convertToGL(type),
                 0,
                 convertToGL(format),
                 width,
                 height,
                 depth,
                 0,
                 convertToGL(format.getSemantic()),
                 convertToGL(format.getType()),
                 source.getPixels());
  }
  else
  {
    glTexImage2D(convertToGL(type),
                 0,
                 convertToGL(format),
                 width,
                 height,
                 0,
                 convertToGL(format.getSemantic()),
                 convertToGL(format.getType()),
                 source.getPixels());
  }

  if (flags & MIPMAPPED)
    generateMipmaps();

  levels = retrieveImages(convertToGL(type), NO_CUBE_FACE);

  applyDefaults();

  if (!checkGL("OpenGL error during creation of texture \'%s\' format \'%s\'",
               getPath().asString().c_str(),
               format.asString().c_str()))
  {
    return false;
  }

  return true;
}

bool Texture::init(const ImageCube& source, unsigned int flags)
{
  if (flags & RECTANGULAR)
  {
    logError("Invalid flags for texture \'%s\': cube maps cannot use the rectangular flag",
              getPath().asString().c_str());
    return false;
  }

  // Verify image
  {
    if (!source.isPOT())
    {
      logWarning("Texture \'%s\' does not have power-of-two dimensions; this may cause slowdown",
               getPath().asString().c_str());
    }

    if (!source.isSquare())
    {
      logError("Source images for texture \'%s\' are not square",
               getPath().asString().c_str());
      return false;
    }

    if (!source.hasSameSize())
    {
      logError("Source images for texture \'%s\' do not have the same size",
               getPath().asString().c_str());
      return false;
    }

    width = source.images[0]->getWidth();
    height = source.images[0]->getHeight();
    depth = 1;

    if (!source.hasSameFormat())
    {
      logError("Source images for texture \'%s\' do not have same format",
               getPath().asString().c_str());
      return false;
    }

    format = source.images[0]->getFormat();

    if (!convertToGL(format))
    {
      logError("Source images for texture \'%s\' have unsupported pixel format \'%s\'",
               getPath().asString().c_str(),
               format.asString().c_str());
      return false;
    }
  }

  type = TEXTURE_CUBE;

  glTexImage2D(convertToProxyGL(type),
               0,
               convertToGL(format),
               width,
               height,
               0,
               convertToGL(format.getSemantic()),
               convertToGL(format.getType()),
               NULL);

  if (!validateProxy())
    return false;

  glGenTextures(1, &textureID);

  context.setCurrentTexture(this);

  for (unsigned int i = 0;  i < 6;  i++)
  {
    wendy::Image& image = *source.images[i];

    glTexImage2D(convertToGL(CubeFace(i)),
                 0,
                 convertToGL(image.getFormat()),
                 image.getWidth(),
                 image.getHeight(),
                 0,
                 convertToGL(image.getFormat().getSemantic()),
                 convertToGL(image.getFormat().getType()),
                 image.getPixels());
  }

  if (flags & MIPMAPPED)
    generateMipmaps();

  for (unsigned int i = 0;  i < 6;  i++)
    levels = retrieveImages(convertToGL(CubeFace(i)), CubeFace(i));

  applyDefaults();

  if (!checkGL("OpenGL error during creation of texture \'%s\' format \'%s\'",
               getPath().asString().c_str(),
               format.asString().c_str()))
  {
    return false;
  }

  return true;
}

bool Texture::validateProxy() const
{
  int width;
  glGetTexLevelParameteriv(convertToProxyGL(type), 0, GL_TEXTURE_WIDTH, &width);

  if (width == 0)
  {
    logError("Cannot create texture \'%s\' type \'%s\' size %ux%ux%u format \'%s\'",
              getPath().asString().c_str(),
              asString(type),
              width, height, depth,
              format.asString().c_str());

    return false;
  }

  return true;
}

unsigned int Texture::retrieveImages(unsigned int target, CubeFace face)
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
  // Set up filter modes
  {
    if (hasMipmaps())
      glTexParameteri(convertToGL(type), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    else
      glTexParameteri(convertToGL(type), GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(convertToGL(type), GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    filterMode = FILTER_BILINEAR;
  }

  // Set up address modes
  {
    glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    addressMode = ADDRESS_CLAMP;
  }
}

Texture& Texture::operator = (const Texture& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

TextureReader::TextureReader(Context& initContext):
  ResourceReader(initContext.getCache()),
  context(initContext)
{
  if (addressModeMap.isEmpty())
  {
    addressModeMap["wrap"] = ADDRESS_WRAP;
    addressModeMap["clamp"] = ADDRESS_CLAMP;
  }

  if (filterModeMap.isEmpty())
  {
    filterModeMap["nearest"] = FILTER_NEAREST;
    filterModeMap["bilinear"] = FILTER_BILINEAR;
    filterModeMap["trilinear"] = FILTER_TRILINEAR;
  }
}

Ref<Texture> TextureReader::read(const Path& path)
{
  if (Resource* cached = getCache().findResource(path))
    return dynamic_cast<Texture*>(cached);

  std::ifstream stream;
  if (!getCache().openFile(stream, path))
    return NULL;

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load(stream);
  if (!result)
  {
    logError("Failed to load texture \'%s\': %s",
             path.asString().c_str(),
             result.description());
    return NULL;
  }

  pugi::xml_node root = document.child("texture");
  if (!root || root.attribute("version").as_uint() != TEXTURE_XML_VERSION)
  {
    logError("Texture file format mismatch in \'%s\'",
             path.asString().c_str());
    return NULL;
  }

  unsigned int flags = Texture::DEFAULT;

  // Parse flags
  {
    bool defaultValue;

    defaultValue = (Texture::DEFAULT & Texture::MIPMAPPED) ? true : false;
    if (pugi::xml_attribute a = root.attribute("mipmapped"))
    {
      if (a.as_bool() != defaultValue)
        flags ^= Texture::MIPMAPPED;
    }

    defaultValue = (Texture::DEFAULT & Texture::RECTANGULAR) ? true : false;
    if (pugi::xml_attribute a = root.attribute("rectangular"))
    {
      if (a.as_bool() != defaultValue)
        flags ^= Texture::RECTANGULAR;
    }
  }

  Ref<Texture> texture;

  if (pugi::xml_attribute a = root.attribute("image"))
  {
    Ref<wendy::Image> image = wendy::Image::read(getCache(), Path(a.value()));
    if (!image)
    {
      logError("Failed to load source image for texture \'%s\'",
               path.asString().c_str());
      return NULL;
    }

    texture = Texture::create(ResourceInfo(getCache(), path), context, *image, flags);
  }
  else if (pugi::xml_attribute a = root.attribute("imagecube"))
  {
    Ref<ImageCube> cube = ImageCube::read(getCache(), Path(a.value()));
    if (!cube)
    {
      logError("Failed to load source image cube for texture \'%s\'",
               path.asString().c_str());
      return NULL;
    }

    texture = Texture::create(ResourceInfo(getCache(), path), context, *cube, flags);
  }
  else
  {
    logError("No image specified for texture \'%s\'",
              path.asString().c_str());
    return NULL;
  }

  if (pugi::xml_attribute a = root.attribute("filter"))
  {
    if (filterModeMap.hasKey(a.value()))
      texture->setFilterMode(filterModeMap[a.value()]);
    else
    {
      logError("Invalid filter mode name \'%s\'", a.value());
      return NULL;
    }
  }

  if (pugi::xml_attribute a = root.attribute("address"))
  {
    if (addressModeMap.hasKey(a.value()))
      texture->setAddressMode(addressModeMap[a.value()]);
    else
    {
      logError("Invalid address mode name \'%s\'", a.value());
      return NULL;
    }
  }

  return texture;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
