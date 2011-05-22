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

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

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

GLenum convertToGL(AddressMode mode)
{
  switch (mode)
  {
    case ADDRESS_WRAP:
      return GL_REPEAT;
    case ADDRESS_CLAMP:
      return GL_CLAMP_TO_EDGE;
  }

  logError("Invalid texture address mode %u", mode);
  return 0;
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

  log("Invalid texture filter mode %u", mode);
  return 0;
}

GLenum convertToGL(ImageCube::Face face)
{
  switch (face)
  {
    case ImageCube::POSITIVE_X:
      return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    case ImageCube::NEGATIVE_X:
      return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
    case ImageCube::POSITIVE_Y:
      return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
    case ImageCube::NEGATIVE_Y:
      return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
    case ImageCube::POSITIVE_Z:
      return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
    case ImageCube::NEGATIVE_Z:
      return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
  }

  logError("Invalid image cube face %u", face);
  return 0;
}

Bimap<String, FilterMode> filterModeMap;
Bimap<String, AddressMode> addressModeMap;

const unsigned int TEXTURE_XML_VERSION = 1;

} /*namespace*/

///////////////////////////////////////////////////////////////////////

bool TextureImage::copyFrom(const wendy::Image& source, unsigned int x, unsigned int y, unsigned int z)
{
  if (source.getFormat() != texture.format)
  {
    // TODO: Convert to compatible pixel format

    logError("Cannot copy texture data from source image of different pixel format");
    return false;
  }

  if (texture.type == TEXTURE_1D)
  {
    if (source.getDimensionCount() > 1)
    {
      logError("Cannot blt to texture; source image has too many dimensions");
      return false;
    }

    texture.context.setCurrentTexture(&texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage1D(convertToGL(texture.type),
                    level,
		    x,
		    source.getWidth(),
                    convertToGL(texture.format.getSemantic()),
                    convertToGL(texture.format.getType()),
		    source.getPixels());
  }
  else if (texture.type == TEXTURE_3D)
  {
    texture.context.setCurrentTexture(&texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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
  result = wendy::Image(texture.getIndex(), texture.format, width, height, depth);

  texture.context.setCurrentTexture(&texture);

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
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

unsigned int TextureImage::getWidth(void) const
{
  return width;
}

unsigned int TextureImage::getHeight(void) const
{
  return height;
}

unsigned int TextureImage::getDepth(void) const
{
  return depth;
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
                           unsigned int initHeight,
                           unsigned int initDepth):
  texture(initTexture),
  level(initLevel),
  width(initWidth),
  height(initHeight),
  depth(initDepth)
{
}

void TextureImage::attach(int attachment)
{
  if (texture.type == TEXTURE_1D)
  {
    glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              convertToGL(texture.type),
                              texture.textureID,
                              level);
  }
  else if (texture.type == TEXTURE_3D)
  {
    glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              convertToGL(texture.type),
                              texture.textureID,
                              level,
                              0);
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
  checkGL("Error when attaching level %u of texture \'%s\' to image canvas",
          level,
          texture.getPath().asString().c_str());
#endif
}

void TextureImage::detach(int attachment)
{
  if (texture.type == TEXTURE_1D)
  {
    glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
                              attachment,
                              convertToGL(texture.type),
                              0,
                              0);
  }
  else if (texture.type == TEXTURE_3D)
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
  checkGL("Error when detaching level %u of texture \'%s\' from image canvas",
          level,
          texture.getPath().asString().c_str());
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

TextureType Texture::getType(void) const
{
  return type;
}

unsigned int Texture::getWidth(unsigned int level)
{
  return getImage(level).getWidth();
}

unsigned int Texture::getHeight(unsigned int level)
{
  return getImage(level).getHeight();
}

unsigned int Texture::getDepth(unsigned int level)
{
  return getImage(level).getDepth();
}

unsigned int Texture::getSourceWidth(void) const
{
  return sourceWidth;
}

unsigned int Texture::getSourceHeight(void) const
{
  return sourceHeight;
}

unsigned int Texture::getSourceDepth(void) const
{
  return sourceDepth;
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
    context.setCurrentTexture(this);

    glTexParameteri(convertToGL(type),
                    GL_TEXTURE_MIN_FILTER,
		    convertToGL(newMode, isMipmapped()));
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

AddressMode Texture::getAddressMode(void) const
{
  return addressMode;
}

void Texture::setAddressMode(AddressMode newMode)
{
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
  sourceWidth(0),
  sourceHeight(0),
  sourceDepth(0),
  flags(0),
  filterMode(FILTER_BILINEAR),
  addressMode(ADDRESS_WRAP)
{
}

Texture::Texture(const Texture& source):
  Resource(source),
  context(source.context)
{
}

bool Texture::init(const wendy::Image& source, unsigned int initFlags)
{
  wendy::Image final = source;

  format = final.getFormat();

  if (!convertToGL(format))
  {
    logError("Source image for texture \'%s\' has unsupported pixel format \'%s\'",
             getPath().asString().c_str(),
             format.asString().c_str());
    return false;
  }

  flags = initFlags;

  // Figure out which texture target to use

  if (flags & RECTANGULAR)
  {
    if (final.getDimensionCount() > 2)
    {
      logError("Rectangular textures cannot have more than two dimensions");
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
    if (final.getDimensionCount() == 1)
      type = TEXTURE_1D;
    else if (final.getDimensionCount() == 2)
      type = TEXTURE_2D;
    else
      type = TEXTURE_3D;
  }

  // Save source image dimensions
  sourceWidth = final.getWidth();
  sourceHeight = final.getHeight();
  sourceDepth = final.getDepth();

  unsigned int width, height, depth;

  // Adapt source image to OpenGL restrictions
  {
    // Figure out target dimensions for rescaling

    if (flags & RECTANGULAR)
    {
      width = sourceWidth;
      height = sourceHeight;
      depth = 1;

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
      depth = getClosestPower(sourceDepth, maxSize);
    }

    // Rescale source image (no-op if the sizes are equal)
    if (!final.resize(width, height, depth))
    {
      logError("Failed to rescale image for texture \'%s\'",
               getPath().asString().c_str());
      return false;
    }
  }

  glGenTextures(1, &textureID);

  context.setCurrentTexture(this);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  if (type == TEXTURE_1D)
  {
    glTexImage1D(convertToGL(type),
                 0,
                 convertToGL(format),
                 final.getWidth(),
                 0,
                 convertToGL(format.getSemantic()),
                 convertToGL(format.getType()),
                 final.getPixels());
  }
  else if (type == TEXTURE_3D)
  {
    glTexImage3D(convertToGL(type),
                 0,
                 convertToGL(format),
                 final.getWidth(),
                 final.getHeight(),
                 final.getDepth(),
                 0,
                 convertToGL(format.getSemantic()),
                 convertToGL(format.getType()),
                 final.getPixels());
  }
  else
  {
    glTexImage2D(convertToGL(type),
                 0,
                 convertToGL(format),
                 final.getWidth(),
                 final.getHeight(),
                 0,
                 convertToGL(format.getSemantic()),
                 convertToGL(format.getType()),
                 final.getPixels());
  }

  if (flags & MIPMAPPED)
  {
    glTexParameteri(convertToGL(type), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glGenerateMipmapEXT(convertToGL(type));

    unsigned int level = 0;

    for (;;)
    {
      glGetTexLevelParameteriv(convertToGL(type), level, GL_TEXTURE_WIDTH, (int*) &width);
      glGetTexLevelParameteriv(convertToGL(type), level, GL_TEXTURE_HEIGHT, (int*) &height);
      glGetTexLevelParameteriv(convertToGL(type), level, GL_TEXTURE_DEPTH, (int*) &depth);

      if (width == 0)
        break;

      TextureImageRef image = new TextureImage(*this, level, width, height, depth);
      images.push_back(image);

      level++;
    }
  }
  else
  {
    glTexParameteri(convertToGL(type), GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    TextureImageRef image = new TextureImage(*this, 0, final.getWidth(), final.getHeight(), final.getDepth());
    images.push_back(image);
  }

  glTexParameteri(convertToGL(type), GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  if (type == TEXTURE_RECT)
  {
    glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    addressMode = ADDRESS_CLAMP;
  }
  else
  {
    glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_S, GL_REPEAT);

    if (type != TEXTURE_1D)
      glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (type == TEXTURE_3D)
      glTexParameteri(convertToGL(type), GL_TEXTURE_WRAP_R, GL_REPEAT);

    addressMode = ADDRESS_WRAP;
  }

  if (!checkGL("OpenGL error during creation of texture \'%s\' of format \'%s\'",
               getPath().asString().c_str(),
               format.asString().c_str()))
  {
    return false;
  }

  return true;
}

bool Texture::init(const ImageCube& source, unsigned int initFlags)
{
  // Check image dimensions
  {
    if (!source.isPOT())
    {
      logError("Source images for texture \'%s\' do not have POT dimensions",
               getPath().asString().c_str());
      return false;
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

    sourceWidth = source.images[0]->getWidth();
    sourceHeight = source.images[0]->getHeight();

    const unsigned int maxSize = context.getLimits().getMaxTextureCubeSize();

    if (sourceWidth > maxSize)
    {
      logError("Source images for texture \'%s\' are too large; maximum size is %ux%u",
               getPath().asString().c_str(),
               maxSize, maxSize);
      return false;
    }
  }

  // Check image formats
  {
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

    if (!convertToGL(format.getType()))
    {
      logError("Source images for texture \'%s\' have unsupported component type",
               getPath().asString().c_str());
      return false;
    }
  }

  // Check creation flags
  {
    flags = initFlags;

    if (flags & RECTANGULAR)
    {
      logError("Invalid flags for texture \'%s\': cube maps cannot be rectangular",
               getPath().asString().c_str());
      return false;
    }
  }

  type = TEXTURE_CUBE;

  glGenTextures(1, &textureID);

  context.setCurrentTexture(this);

  for (unsigned int i = 0;  i < 6;  i++)
  {
    GLenum faceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;

    wendy::Image& image = *source.images[i];

    glTexImage2D(faceTarget,
                 0,
                 convertToGL(image.getFormat()),
                 image.getWidth(),
                 image.getHeight(),
                 0,
                 convertToGL(image.getFormat().getSemantic()),
                 convertToGL(image.getFormat().getType()),
                 image.getPixels());
  }

  glGenerateMipmapEXT(convertToGL(type));

  for (unsigned int i = 0;  i < 6;  i++)
  {
    GLenum faceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;

    wendy::Image& image = *source.images[i];

    if (flags & MIPMAPPED)
    {
      glTexParameteri(convertToGL(type), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

      unsigned int level = 0;

      for (;;)
      {
        unsigned int width, height;

        glGetTexLevelParameteriv(faceTarget, level, GL_TEXTURE_WIDTH, (int*) &width);
        glGetTexLevelParameteriv(faceTarget, level, GL_TEXTURE_HEIGHT, (int*) &height);

        if (width == 0)
          break;

        TextureImageRef image = new TextureImage(*this, level, width, height, 1);
        images.push_back(image);

        level++;
      }
    }
    else
    {
      glTexParameteri(convertToGL(type), GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      unsigned int width, height;

      glGetTexLevelParameteriv(faceTarget, 0, GL_TEXTURE_WIDTH, (int*) &width);
      glGetTexLevelParameteriv(faceTarget, 0, GL_TEXTURE_HEIGHT, (int*) &height);

      TextureImageRef image = new TextureImage(*this, 0, width, height, 1);
      images.push_back(image);
    }
  }

  glTexParameteri(convertToGL(type), GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  if (!checkGL("OpenGL error during creation of texture \'%s\' of format \'%s\'",
               getPath().asString().c_str(),
               format.asString().c_str()))
  {
    return false;
  }

  return true;
}

Texture& Texture::operator = (const Texture& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

TextureReader::TextureReader(Context& initContext):
  ResourceReader(initContext.getIndex()),
  context(initContext),
  info(initContext.getIndex())
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
  if (Resource* cache = getIndex().findResource(path))
    return dynamic_cast<Texture*>(cache);

  info.path = path;

  std::ifstream stream;
  if (!getIndex().openFile(stream, info.path))
    return NULL;

  if (!XML::Reader::read(stream))
  {
    texture = NULL;
    return NULL;
  }

  if (!texture)
  {
    logError("No texture specification found in file");
    return NULL;
  }

  return texture.detachObject();
}

bool TextureReader::onBeginElement(const String& name)
{
  if (name == "texture")
  {
    if (texture)
    {
      logError("Only one texture specification per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != TEXTURE_XML_VERSION)
    {
      logError("Texture specification XML format version mismatch");
      return false;
    }

    unsigned int flags = Texture::DEFAULT;

    // Parse flags
    {
      bool defaultValue;

      defaultValue = (Texture::DEFAULT | Texture::MIPMAPPED) ? true : false;
      if (readBoolean("mipmapped", defaultValue) != defaultValue)
	flags ^= Texture::MIPMAPPED;

      defaultValue = (Texture::DEFAULT | Texture::RECTANGULAR) ? true : false;
      if (readBoolean("rectangular", defaultValue) != defaultValue)
	flags ^= Texture::RECTANGULAR;
    }

    Path imagePath(readString("image"));
    if (imagePath.isEmpty())
    {
      imagePath = readString("imagecube");
      if (imagePath.isEmpty())
      {
        logError("No image specified for texture \'%s\'",
                 info.path.asString().c_str());
        return false;
      }

      Ref<ImageCube> source = ImageCube::read(getIndex(), imagePath);
      if (!source)
      {
        logError("Failed to load image cube \'%s\' for texture \'%s\'",
                 imagePath.asString().c_str(),
                 info.path.asString().c_str());
        return false;
      }

      texture = Texture::create(info, context, *source, flags);
      if (!texture)
        return false;
    }
    else
    {
      Ref<wendy::Image> source = wendy::Image::read(getIndex(), imagePath);
      if (!source)
      {
        logError("Failed to load image \'%s\' for texture \'%s\'",
                 imagePath.asString().c_str(),
                 info.path.asString().c_str());
        return false;
      }

      texture = Texture::create(info, context, *source, flags);
      if (!texture)
        return false;
    }

    String filterModeName = readString("filter");
    if (!filterModeName.empty())
    {
      if (filterModeMap.hasKey(filterModeName))
	texture->setFilterMode(filterModeMap[filterModeName]);
      else
      {
	logError("Invalid filter mode name \'%s\'",
                 filterModeName.c_str());
	return false;
      }
    }

    String addressModeName = readString("address");
    if (!addressModeName.empty())
    {
      if (addressModeMap.hasKey(addressModeName))
	texture->setAddressMode(addressModeMap[addressModeName]);
      else
      {
	logError("Invalid address mode name \'%s\'",
                 addressModeName.c_str());
	return false;
      }
    }

    return true;
  }

  return true;
}

bool TextureReader::onEndElement(const String& name)
{
  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
