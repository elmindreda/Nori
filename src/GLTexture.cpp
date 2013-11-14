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

#include <wendy/Config.hpp>

#include <wendy/GLTexture.hpp>
#include <wendy/GLBuffer.hpp>
#include <wendy/GLProgram.hpp>
#include <wendy/GLContext.hpp>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLHelper.hpp>

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
      return GL_PROXY_TEXTURE_RECTANGLE;
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
    default:
      break;
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

TextureData::TextureData(const Image& image):
  format(image.format()),
  width(image.width()),
  height(image.height()),
  depth(image.depth()),
  texels(image.pixels())
{
}

TextureData::TextureData(PixelFormat initFormat,
                         uint initWidth,
                         uint initHeight,
                         uint initDepth,
                         const void* initTexels):
  format(initFormat),
  width(initWidth),
  height(initHeight),
  depth(initDepth),
  texels(initTexels)
{
}

bool TextureData::isPOT() const
{
  return isPowerOfTwo(width) && isPowerOfTwo(height) && isPowerOfTwo(depth);
}

uint TextureData::dimensionCount() const
{
  if (depth > 1)
    return 3;

  if (height > 1)
    return 2;

  return 1;
}

///////////////////////////////////////////////////////////////////////

TextureParams::TextureParams(TextureType initType, uint initFlags):
  type(initType),
  flags(initFlags)
{
  if (type == TEXTURE_RECT)
    flags &= ~TF_MIPMAPPED;
}

///////////////////////////////////////////////////////////////////////

bool TextureImage::copyFrom(const Image& source, uint x, uint y, uint z)
{
  if (source.format() != m_texture.format())
  {
    // TODO: Convert to compatible pixel format

    logError("Cannot copy texture data from source image of different pixel format");
    return false;
  }

  if (m_texture.is1D())
  {
    if (source.dimensionCount() > 1)
    {
      logError("Cannot blt to texture; source image has too many dimensions");
      return false;
    }

    m_texture.context().setCurrentTexture(&m_texture);

    glTexSubImage1D(convertToGL(m_texture.type()),
                    m_level,
                    x,
                    source.width(),
                    convertToGL(m_texture.format().semantic()),
                    convertToGL(m_texture.format().type()),
                    source.pixels());
  }
  else if (m_texture.is3D())
  {
    m_texture.context().setCurrentTexture(&m_texture);

    glTexSubImage3D(convertToGL(m_texture.type()),
                    m_level,
                    x, y, z,
                    source.width(),
                    source.height(),
                    source.depth(),
                    convertToGL(m_texture.format().semantic()),
                    convertToGL(m_texture.format().type()),
                    source.pixels());
  }
  else
  {
    if (source.dimensionCount() > 2)
    {
      logError("Cannot blt to texture; source image has too many dimensions");
      return false;
    }

    m_texture.context().setCurrentTexture(&m_texture);

    glTexSubImage2D(convertToGL(m_texture.type()),
                    m_level,
                    x, y,
                    source.width(), source.height(),
                    convertToGL(m_texture.format().semantic()),
                    convertToGL(m_texture.format().type()),
                    source.pixels());
  }

#if WENDY_DEBUG
  if (!checkGL("Error during copy from image into level %u of texture %s",
               m_level,
               m_texture.name().c_str()))
  {
    return false;
  }
#endif

  return true;
}

Ref<Image> TextureImage::data() const
{
  Ref<Image> result = Image::create(m_texture.cache(),
                                    m_texture.format(),
                                    m_width,
                                    m_height,
                                    m_depth);

  m_texture.context().setCurrentTexture(&m_texture);

  glGetTexImage(convertToGL(m_texture.type()),
                m_level,
                convertToGL(m_texture.format().semantic()),
                convertToGL(m_texture.format().type()),
                result->pixels());

#if WENDY_DEBUG
  if (!checkGL("Error during copy to image from level %u of texture %s",
               m_level,
               m_texture.name().c_str()))
  {
    return nullptr;
  }
#endif

  return result;
}

size_t TextureImage::size() const
{
  return m_width * m_height * m_depth * m_texture.format().size();
}

TextureImage::TextureImage(Texture& texture,
                           uint level,
                           uint width,
                           uint height,
                           uint depth,
                           CubeFace face):
  m_texture(texture),
  m_level(level),
  m_width(width),
  m_height(height),
  m_depth(depth),
  m_face(face)
{
}

void TextureImage::attach(int attachment, uint z)
{
  if (m_texture.is1D())
  {
    glFramebufferTexture1D(GL_FRAMEBUFFER,
                           attachment,
                           convertToGL(m_texture.type()),
                           m_texture.m_textureID,
                           m_level);
  }
  else if (m_texture.is3D())
  {
    glFramebufferTexture3D(GL_FRAMEBUFFER,
                           attachment,
                           convertToGL(m_texture.type()),
                           m_texture.m_textureID,
                           m_level,
                           z);
  }
  else
  {
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           attachment,
                           convertToGL(m_texture.type()),
                           m_texture.m_textureID,
                           m_level);
  }

#if WENDY_DEBUG
  checkGL("Error when attaching level %u of texture %s to framebuffer",
          m_level,
          m_texture.name().c_str());
#endif
}

void TextureImage::detach(int attachment)
{
  if (m_texture.is1D())
  {
    glFramebufferTexture1D(GL_FRAMEBUFFER,
                           attachment,
                           convertToGL(m_texture.type()),
                           0, 0);
  }
  else if (m_texture.is3D())
  {
    glFramebufferTexture3D(GL_FRAMEBUFFER,
                           attachment,
                           convertToGL(m_texture.type()),
                           0, 0, 0);
  }
  else
  {
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           attachment,
                           convertToGL(m_texture.type()),
                           0, 0);
  }

#if WENDY_DEBUG
  checkGL("Error when detaching level %u of texture %s from framebuffer",
          m_level,
          m_texture.name().c_str());
#endif
}

///////////////////////////////////////////////////////////////////////

Texture::~Texture()
{
  if (m_textureID)
    glDeleteTextures(1, &m_textureID);

  if (Stats* stats = m_context.stats())
    stats->removeTexture(size());
}

void Texture::generateMipmaps()
{
  glGenerateMipmap(convertToGL(m_type));

  if (!hasMipmaps())
  {
    retrieveImages();

    glTexParameteri(convertToGL(m_type),
                    GL_TEXTURE_MIN_FILTER,
                    convertToGL(m_filterMode, true));
  }
}

bool Texture::is1D() const
{
  return m_type == TEXTURE_1D;
}

bool Texture::is2D() const
{
  return m_type == TEXTURE_2D || TEXTURE_RECT;
}

bool Texture::is3D() const
{
  return m_type == TEXTURE_3D;
}

bool Texture::isCube() const
{
  return m_type == TEXTURE_CUBE;
}

bool Texture::isPOT() const
{
  return isPowerOfTwo(width()) &&
         isPowerOfTwo(height()) &&
         isPowerOfTwo(depth());
}

void Texture::setFilterMode(FilterMode newMode)
{
  if (newMode != m_filterMode)
  {
    m_context.setCurrentTexture(this);

    glTexParameteri(convertToGL(m_type),
                    GL_TEXTURE_MIN_FILTER,
                    convertToGL(newMode, hasMipmaps()));
    glTexParameteri(convertToGL(m_type),
                    GL_TEXTURE_MAG_FILTER,
                    convertToGL(newMode, false));

    m_filterMode = newMode;
  }

#if WENDY_DEBUG
  checkGL("Error when changing filter mode for texture %s",
          name().c_str());
#endif
}

void Texture::setAddressMode(AddressMode newMode)
{
  if (m_type == TEXTURE_RECT)
  {
    if (newMode != ADDRESS_CLAMP)
    {
      logError("Rectangular textures only support ADDRESS_CLAMP");
      return;
    }
  }

  if (newMode != m_addressMode)
  {
    m_context.setCurrentTexture(this);

    glTexParameteri(convertToGL(m_type), GL_TEXTURE_WRAP_S, convertToGL(newMode));
    glTexParameteri(convertToGL(m_type), GL_TEXTURE_WRAP_T, convertToGL(newMode));
    glTexParameteri(convertToGL(m_type), GL_TEXTURE_WRAP_R, convertToGL(newMode));

    m_addressMode = newMode;
  }

#if WENDY_DEBUG
  checkGL("Error when changing address mode for texture %s",
          name().c_str());
#endif
}

void Texture::setMaxAnisotropy(float newMax)
{
  if (newMax != m_maxAnisotropy)
  {
    if (!GLEW_EXT_texture_filter_anisotropic)
    {
      logError("Cannot set max anisotropy: "
               "GL_EXT_texture_filter_anisotropic is missing");
      return;
    }

    m_context.setCurrentTexture(this);

    glTexParameteri(convertToGL(m_type), GL_TEXTURE_MAX_ANISOTROPY_EXT, int(newMax));

    m_maxAnisotropy = newMax;
  }

#if WENDY_DEBUG
  checkGL("Error when changing max anisotropy for texture %s",
          name().c_str());
#endif
}

size_t Texture::size() const
{
  size_t size = 0;

  for (auto& i : m_images)
    size += i->size();

  return size;
}

TextureImage& Texture::image(uint level, CubeFace face)
{
  if (isCube())
    return *m_images[face * m_levels + level];
  else
    return *m_images[level];
}

const TextureImage& Texture::image(uint level, CubeFace face) const
{
  if (isCube())
    return *m_images[face * m_levels + level];
  else
    return *m_images[level];
}

Ref<Texture> Texture::create(const ResourceInfo& info,
                             Context& context,
                             const TextureParams& params,
                             const TextureData& data)
{
  Ref<Texture> texture(new Texture(info, context));
  if (!texture->init(params, data))
    return nullptr;

  return texture;
}

Ref<Texture> Texture::read(Context& context,
                           const TextureParams& params,
                           const String& imageName)
{
  ResourceCache& cache = context.cache();

  String name;
  name += "source:";
  name += imageName;
  name += " mipmapped:";
  name += (params.flags & TF_MIPMAPPED) ? "true" : "false";
  name += " sRGB:";
  name += (params.flags & TF_SRGB) ? "true" : "false";

  if (Ref<Texture> texture = cache.find<Texture>(name))
    return texture;

  Ref<Image> data = Image::read(cache, imageName);
  if (!data)
  {
    logError("Failed to read image for texture %s", name.c_str());
    return nullptr;
  }

  return create(ResourceInfo(cache, name), context, params, *data);
}

Texture::Texture(const ResourceInfo& info, Context& context):
  Resource(info),
  m_context(context),
  m_textureID(0),
  m_levels(0),
  m_filterMode(FILTER_BILINEAR),
  m_addressMode(ADDRESS_WRAP),
  m_maxAnisotropy(1.f)
{
}

bool Texture::init(const TextureParams& params, const TextureData& data)
{
  m_type = params.type;
  m_format = data.format;

  const bool sRGB = (params.flags & TF_SRGB) ? true : false;
  const bool mipmapped = (params.flags & TF_MIPMAPPED) ? true : false;

  if (!convertToGL(m_format, sRGB))
  {
    logError("Source image for texture %s has unsupported pixel format %s",
             name().c_str(),
             m_format.asString().c_str());
    return false;
  }

  // Figure out which texture target to use

  if (m_type == TEXTURE_RECT)
  {
    if (data.dimensionCount() > 2)
    {
      logError("Source image for rectangular texture %s has more than two dimensions",
               name().c_str());
      return false;
    }

    if (mipmapped)
    {
      logError("Texture %s specified as both rectangular and mipmapped",
               name().c_str());
      return false;
    }
  }
  else if (m_type == TEXTURE_CUBE)
  {
    if (data.dimensionCount() > 2)
    {
      logError("Source image for cubemap texture %s has more than two dimensions",
               name().c_str());
      return false;
    }

    const uint width = data.width;
    const uint height = data.height;

    if ((width % 6 != 0) || (width / 6 != height) || !isPowerOfTwo(height))
    {
      logError("Source image for cubemap texture %s has invalid dimensions",
               name().c_str());
      return false;
    }
  }
  else
  {
    if (!data.isPOT())
    {
      logWarning("Texture %s does not have power-of-two dimensions; this may cause slowdown",
                 name().c_str());
    }
  }

  uint width, height, depth;

  if (m_type == TEXTURE_CUBE)
  {
    width = data.width / 6;
    height = data.height;
    depth = 1;
  }
  else
  {
    width = data.width;
    height = data.height;
    depth = data.depth;
  }

  if (m_type == TEXTURE_1D)
  {
    glTexImage1D(convertToProxyGL(m_type),
                 0,
                 convertToGL(m_format, sRGB),
                 width,
                 0,
                 convertToGL(m_format.semantic()),
                 convertToGL(m_format.type()),
                 nullptr);
  }
  else if (m_type == TEXTURE_3D)
  {
    glTexImage3D(convertToProxyGL(m_type),
                 0,
                 convertToGL(m_format, sRGB),
                 width,
                 height,
                 depth,
                 0,
                 convertToGL(m_format.semantic()),
                 convertToGL(m_format.type()),
                 nullptr);
  }
  else
  {
    glTexImage2D(convertToProxyGL(m_type),
                 0,
                 convertToGL(m_format, sRGB),
                 width,
                 height,
                 0,
                 convertToGL(m_format.semantic()),
                 convertToGL(m_format.type()),
                 nullptr);
  }

  GLint proxyWidth;
  glGetTexLevelParameteriv(convertToProxyGL(m_type),
                           0,
                           GL_TEXTURE_WIDTH,
                           &proxyWidth);

  if (proxyWidth == 0)
  {
    logError("Cannot create texture %s type %s size %ux%ux%u format %s",
             name().c_str(),
             asString(m_type),
             width, height, depth,
             m_format.asString().c_str());

    return false;
  }

  glGenTextures(1, &m_textureID);

  m_context.setCurrentTexture(this);

  if (m_type == TEXTURE_1D)
  {
    glTexImage1D(convertToGL(m_type),
                 0,
                 convertToGL(m_format, sRGB),
                 width,
                 0,
                 convertToGL(m_format.semantic()),
                 convertToGL(m_format.type()),
                 data.texels);
  }
  else if (m_type == TEXTURE_3D)
  {
    glTexImage3D(convertToGL(m_type),
                 0,
                 convertToGL(m_format, sRGB),
                 width, height, depth,
                 0,
                 convertToGL(m_format.semantic()),
                 convertToGL(m_format.type()),
                 data.texels);
  }
  else if (m_type == TEXTURE_CUBE)
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

    glPixelStorei(GL_UNPACK_ROW_LENGTH, data.width);

    for (size_t i = 0;  i < 6;  i++)
    {
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, i * width);

      glTexImage2D(convertToGL(faces[i]),
                   0,
                   convertToGL(m_format, sRGB),
                   width, height,
                   0,
                   convertToGL(m_format.semantic()),
                   convertToGL(m_format.type()),
                   data.texels);
    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  }
  else
  {
    glTexImage2D(convertToGL(m_type),
                 0,
                 convertToGL(m_format, sRGB),
                 width, height,
                 0,
                 convertToGL(m_format.semantic()),
                 convertToGL(m_format.type()),
                 data.texels);
  }

  if (mipmapped)
    generateMipmaps();
  else
    retrieveImages();

  applyDefaults();

  if (!checkGL("OpenGL error during creation of texture %s format %s",
               name().c_str(),
               m_format.asString().c_str()))
  {
    return false;
  }

  if (Stats* stats = m_context.stats())
    stats->addTexture(size());

  return true;
}

void Texture::retrieveImages()
{
  m_images.clear();

  if (m_type == TEXTURE_CUBE)
  {
    for (size_t i = 0;  i < 6;  i++)
      m_levels = retrieveTargetImages(convertToGL(CubeFace(i)), CubeFace(i));
  }
  else
    m_levels = retrieveTargetImages(convertToGL(m_type), NO_CUBE_FACE);
}

uint Texture::retrieveTargetImages(uint target, CubeFace face)
{
  uint level = 0;

  for (;;)
  {
    uint width, height, depth;

    glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, (int*) &width);
    glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, (int*) &height);
    glGetTexLevelParameteriv(target, level, GL_TEXTURE_DEPTH, (int*) &depth);

    if (width == 0)
      break;

    m_images.push_back(new TextureImage(*this, level, width, height, depth, face));

    level++;
  }

  return level;
}

void Texture::applyDefaults()
{
  m_filterMode = FILTER_BILINEAR;

  glTexParameteri(convertToGL(m_type),
                  GL_TEXTURE_MIN_FILTER,
                  convertToGL(m_filterMode, hasMipmaps()));
  glTexParameteri(convertToGL(m_type),
                  GL_TEXTURE_MAG_FILTER,
                  convertToGL(m_filterMode, false));

  m_addressMode = ADDRESS_CLAMP;

  glTexParameteri(convertToGL(m_type), GL_TEXTURE_WRAP_S, convertToGL(m_addressMode));
  glTexParameteri(convertToGL(m_type), GL_TEXTURE_WRAP_T, convertToGL(m_addressMode));
  glTexParameteri(convertToGL(m_type), GL_TEXTURE_WRAP_R, convertToGL(m_addressMode));
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
