///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#include <wendy/Texture.hpp>
#include <wendy/RenderBuffer.hpp>
#include <wendy/Program.hpp>
#include <wendy/RenderContext.hpp>

#include <GREG/greg.h>

#include <internal/OpenGL.hpp>

#include <glm/gtx/bit.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
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

GLenum convertToGL(TextureType type, CubeFace face)
{
  if (face == NO_CUBE_FACE)
    return convertToGL(type);
  else
    return convertToGL(face);
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

TextureImage::TextureImage(uint initLevel, CubeFace initFace):
  level(initLevel),
  face(initFace)
{
}

///////////////////////////////////////////////////////////////////////

Texture::~Texture()
{
  if (m_textureID)
    glDeleteTextures(1, &m_textureID);

  if (RenderStats* stats = m_context.stats())
    stats->removeTexture(size());
}

bool Texture::copyFrom(const TextureImage& image,
                       const TextureData& data,
                       uint x, uint y, uint z)
{
  if (data.format != m_format)
  {
    // TODO: Convert to compatible pixel format

    logError("Cannot copy texture data from source data of different pixel format");
    return false;
  }

  if (is1D())
  {
    if (data.dimensionCount() > 1)
    {
      logError("Cannot blt to texture; source image has too many dimensions");
      return false;
    }

    m_context.setCurrentTexture(this);

    glTexSubImage1D(convertToGL(m_type),
                    image.level,
                    x,
                    data.width,
                    convertToGL(m_format.semantic()),
                    convertToGL(m_format.type()),
                    data.texels);
  }
  else if (is3D())
  {
    m_context.setCurrentTexture(this);

    glTexSubImage3D(convertToGL(m_type),
                    image.level,
                    x, y, z,
                    data.width, data.height, data.depth,
                    convertToGL(m_format.semantic()),
                    convertToGL(m_format.type()),
                    data.texels);
  }
  else
  {
    if (data.dimensionCount() > 2)
    {
      logError("Cannot blt to texture; source image has too many dimensions");
      return false;
    }

    m_context.setCurrentTexture(this);

    glTexSubImage2D(convertToGL(m_type, image.face),
                    image.level,
                    x, y,
                    data.width, data.height,
                    convertToGL(m_format.semantic()),
                    convertToGL(m_format.type()),
                    data.texels);
  }

#if WENDY_DEBUG
  if (!checkGL("Error during copy from image into level %u of texture %s",
               image.level,
               name().c_str()))
  {
    return false;
  }
#endif

  return true;
}

void Texture::generateMipmaps()
{
  glGenerateMipmap(convertToGL(m_type));

  if (!hasMipmaps())
  {
    retrieveSizes();

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
  return isPowerOfTwo(width(0)) &&
         isPowerOfTwo(height(0)) &&
         isPowerOfTwo(depth(0));
}

Ref<Image> Texture::data(const TextureImage& image)
{
  uvec3 size;

  if (isCube())
    size = m_sizes[image.face * m_levels + image.level];
  else
    size = m_sizes[image.level];

  Ref<Image> result = Image::create(cache(), m_format, size.x, size.y, size.z);

  m_context.setCurrentTexture(this);

  glGetTexImage(convertToGL(m_type, image.face),
                image.level,
                convertToGL(m_format.semantic()),
                convertToGL(m_format.type()),
                result->pixels());

#if WENDY_DEBUG
  if (!checkGL("Error during copy to image from level %u of texture %s",
               image.level,
               name().c_str()))
  {
    return nullptr;
  }
#endif

  return result;
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
    if (!GREG_EXT_texture_filter_anisotropic)
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

Ref<Texture> Texture::create(const ResourceInfo& info,
                             RenderContext& context,
                             const TextureParams& params,
                             const TextureData& data)
{
  Ref<Texture> texture(new Texture(info, context));
  if (!texture->init(params, data))
    return nullptr;

  return texture;
}

Ref<Texture> Texture::read(RenderContext& context,
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

Texture::Texture(const ResourceInfo& info, RenderContext& context):
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
                 width, height, depth,
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
                 width, height,
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

    for (uint i = 0;  i < 6;  i++)
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
    retrieveSizes();

  applyDefaults();

  if (!checkGL("OpenGL error during creation of texture %s format %s",
               name().c_str(),
               m_format.asString().c_str()))
  {
    return false;
  }

  if (RenderStats* stats = m_context.stats())
    stats->addTexture(size());

  return true;
}

void Texture::retrieveSizes()
{
  m_sizes.clear();
  m_size = 0;

  if (m_type == TEXTURE_CUBE)
  {
    for (uint i = 0;  i < 6;  i++)
      m_levels = retrieveTargetSizes(convertToGL(CubeFace(i)), CubeFace(i));
  }
  else
    m_levels = retrieveTargetSizes(convertToGL(m_type), NO_CUBE_FACE);

  for (auto s : m_sizes)
    m_size += s.x * s.y * s.z * m_format.size();
}

uint Texture::retrieveTargetSizes(uint target, CubeFace face)
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

    m_sizes.push_back(uvec3(width, height, depth));

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

void Texture::attach(int attachment, const TextureImage& image, uint z)
{
  if (is1D())
  {
    glFramebufferTexture1D(GL_FRAMEBUFFER,
                           attachment,
                           convertToGL(m_type),
                           m_textureID,
                           image.level);
  }
  else if (is3D())
  {
    glFramebufferTexture3D(GL_FRAMEBUFFER,
                           attachment,
                           convertToGL(m_type),
                           m_textureID,
                           image.level,
                           z);
  }
  else
  {
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           attachment,
                           convertToGL(m_type),
                           m_textureID,
                           image.level);
  }

#if WENDY_DEBUG
  checkGL("Error when attaching level %u of texture %s to framebuffer",
          image.level,
          name().c_str());
#endif
}

void Texture::detach(int attachment)
{
  if (is1D())
  {
    glFramebufferTexture1D(GL_FRAMEBUFFER,
                           attachment,
                           convertToGL(m_type),
                           0, 0);
  }
  else if (is3D())
  {
    glFramebufferTexture3D(GL_FRAMEBUFFER,
                           attachment,
                           convertToGL(m_type),
                           0, 0, 0);
  }
  else
  {
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           attachment,
                           convertToGL(m_type),
                           0, 0);
  }

#if WENDY_DEBUG
  checkGL("Error when detaching texture %s from framebuffer", name().c_str());
#endif
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
