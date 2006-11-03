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
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLShader.h>
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

Mapper<ImageFormat::Type, GLenum> formatMap;

Mapper<ImageFormat::Type, GLenum> genericFormatMap;

Mapper<ShaderUniform::Type, GLenum> samplerTypeMap;

}

///////////////////////////////////////////////////////////////////////

Texture::~Texture(void)
{
  if (textureID)
    glDeleteTextures(1, &textureID);
}

bool Texture::copyFrom(const Image& source,
                       unsigned int x,
		       unsigned int y,
		       unsigned int level)
{
  Image final = source;
  final.convert(format);

  // Moira has y-axis down, OpenGL has y-axis up
  final.flipHorizontal();

  if (textureTarget == GL_TEXTURE_1D)
  {
    if (final.getDimensionCount() > 1)
    {
      Log::writeError("Cannot blt to texture; source image has too many dimensions");
      return false;
    }

    glPushAttrib(GL_TEXTURE_BIT | GL_PIXEL_MODE_BIT);
    glBindTexture(textureTarget, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage1D(textureTarget,
                    level,
		    x,
		    final.getWidth(),
                    genericFormatMap[format],
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

    glPushAttrib(GL_TEXTURE_BIT | GL_PIXEL_MODE_BIT);
    glBindTexture(textureTarget, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage2D(textureTarget,
                    level,
		    x, y,
		    final.getWidth(), final.getHeight(),
                    genericFormatMap[final.getFormat()],
                    GL_UNSIGNED_BYTE,
		    final.getPixels());

    glPopAttrib();
  }

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Error during texture image blt: %s", gluErrorString(error));
    return false;
  }
  
  return true;
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

GLint Texture::getMinFilter(void) const
{
  return minFilter;
}

GLint Texture::getMagFilter(void) const
{
  return magFilter;
}

void Texture::setFilters(GLint newMinFilter, GLint newMagFilter)
{
  if (newMinFilter != minFilter || newMagFilter != magFilter)
  {
    glPushAttrib(GL_TEXTURE_BIT);
    glBindTexture(textureTarget, textureID);

    if (newMinFilter != minFilter)
    {
      glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, newMinFilter);
      minFilter = newMinFilter;
    }

    if (newMagFilter != magFilter)
    {
      glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, newMagFilter);
      magFilter = newMagFilter;
    }

    glPopAttrib();
  }
}

Image* Texture::getImage(unsigned int level) const
{
  if (getPhysicalWidth(level) == 0 || getPhysicalHeight(level) == 0)
  {
    Log::writeError("Cannot retrieve image for non-existent level %u", level);
    return NULL;
  }

  Ptr<Image> result = new Image(format,
                                getPhysicalWidth(level),
				getPhysicalHeight(level));

  glPushAttrib(GL_TEXTURE_BIT | GL_PIXEL_MODE_BIT);
  glBindTexture(textureTarget, textureID);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glGetTexImage(textureTarget,
                level,
		genericFormatMap[format],
		GL_UNSIGNED_BYTE,
		result->getPixels());
  
  glPopAttrib();

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Error during texture image retrieval: %s", gluErrorString(error));
    return NULL;
  }

  result->flipHorizontal();

  return result.detachObject();
}

Texture* Texture::readInstance(const String& name, unsigned int flags)
{
  if (Texture* texture = findInstance(name))
  {
    if (texture->getFlags() != flags)
      throw Exception("Flags differ for cached texture");

    return texture;
  }

  if (Image* image = Image::findInstance(name))
    return createInstance(*image, flags, name);

  Ptr<Image> image = Image::readInstance(name);
  if (!image)
    return NULL;

  return createInstance(*image, flags, name);
}

Texture* Texture::readInstance(const Path& path,
			       unsigned int flags,
			       const String& name)
{
  Ptr<Image> image = Image::readInstance(path);
  if (!image)
    return NULL;

  return createInstance(*image, flags, name);
}

Texture* Texture::readInstance(Stream& stream,
			       unsigned int flags,
			       const String& name)
{
  Ptr<Image> image = Image::readInstance(stream);
  if (!image)
    return NULL;

  return createInstance(*image, flags, name);
}

Texture* Texture::createInstance(const Image& image,
				 unsigned int flags,
				 const String& name)
{
  Ptr<Texture> texture = new Texture(name);
  if (!texture->init(image, flags))
    return NULL;

  return texture.detachObject();
}

Texture::Texture(const String& name):
  Resource<Texture>(name),
  textureTarget(0),
  textureID(0),
  minFilter(0),
  magFilter(0),
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

  if (formatMap.isEmpty())
  {
    formatMap[ImageFormat::ALPHA8] = GL_ALPHA8;
    formatMap[ImageFormat::GREY8] = GL_LUMINANCE8;
    formatMap[ImageFormat::GREYALPHA88] = GL_LUMINANCE8_ALPHA8;
    formatMap[ImageFormat::RGB888] = GL_RGB8;
    formatMap[ImageFormat::RGBA8888] = GL_RGBA8;

    formatMap.setDefaults(ImageFormat::INVALID, 0);
  }

  if (genericFormatMap.isEmpty())
  {
    genericFormatMap[ImageFormat::ALPHA8] = GL_ALPHA;
    genericFormatMap[ImageFormat::GREY8] = GL_LUMINANCE;
    genericFormatMap[ImageFormat::GREYALPHA88] = GL_LUMINANCE_ALPHA;
    genericFormatMap[ImageFormat::RGB888] = GL_RGB;
    genericFormatMap[ImageFormat::RGBA8888] = GL_RGBA;

    genericFormatMap.setDefaults(ImageFormat::INVALID, 0);
  }

  flags = initFlags;

  if (image.getDimensionCount() == 1)
    textureTarget = GL_TEXTURE_1D;
  else if (image.getDimensionCount() == 2)
    textureTarget = GL_TEXTURE_2D;
  else
  {
    // TODO: Support 3D textures

    Log::writeError("3D textures not supported");
    return false;
  }

  // The "normal" width and height correspond to the original dimensions
  width = image.getWidth();
  height = image.getHeight();

  Image source = image;

  // Moira has y-axis down, OpenGL has y-axis up
  source.flipHorizontal();

  // Ensure that source image is in GL-compatible format
  if (!source.convert(getConversionFormat(source.getFormat())))
    return false;

  format = source.getFormat();

  if (flags & RECTANGULAR)
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

    physicalWidth = getClosestPower(source.getWidth(), maxSize);
    physicalHeight = getClosestPower(source.getHeight(), maxSize);

    if (!source.resize(physicalWidth, physicalHeight))
      return false;
  }

  // Clear any errors
  glGetError();

  glGenTextures(1, &textureID);

  glPushAttrib(GL_TEXTURE_BIT | GL_PIXEL_MODE_BIT);
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

    levelCount = /* GAH */;
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

    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    levelCount = 1;
  }

  glGetTexParameteriv(textureTarget, GL_TEXTURE_MIN_FILTER, &minFilter);
  glGetTexParameteriv(textureTarget, GL_TEXTURE_MAG_FILTER, &magFilter);

  glPopAttrib();

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::writeError("Error during creation of texture %s: %s",
                    getName().c_str(),
		    gluErrorString(error));
    return false;
  }
  
  return true;
}
  
///////////////////////////////////////////////////////////////////////

TextureLayer::TextureLayer(unsigned int initUnit):
  unit(initUnit)
{
  static bool initialized = false;

  if (!initialized)
  {
    Context::getCreateSignal().connect(onCreateContext);
    Context::getDestroySignal().connect(onDestroyContext);

    if (Context::get() && caches.empty())
      onCreateContext();

    initialized = true;
  }

  if (samplerTypeMap.isEmpty())
  {
    samplerTypeMap[ShaderUniform::SAMPLER_1D] = GL_TEXTURE_1D;
    samplerTypeMap[ShaderUniform::SAMPLER_2D] = GL_TEXTURE_2D;
    samplerTypeMap[ShaderUniform::SAMPLER_3D] = GL_TEXTURE_3D;
    samplerTypeMap.setDefaults((ShaderUniform::Type) 0, 0);
  }
}

void TextureLayer::apply(void) const
{
  if (unit > unitCount)
  {
    Log::writeError("Cannot apply texture layer to non-existent texture unit");
    return;
  }

  Data& cache = caches[unit];

  if (cache.dirty)
  {
    force();
    return;
  }

  if (GLEW_ARB_multitexture)
  {
    if (unit != activeUnit)
    {
      glActiveTextureARB(GL_TEXTURE0_ARB + unit);
      activeUnit = unit;
    }
  }

  if (data.textureName.empty())
  {
    if (!cache.textureName.empty())
    {
      glDisable(textureTargets[unit]);
      textureTargets[unit] = 0;
      cache.textureName.clear();
    }

    if (!data.samplerName.empty())
      Log::writeError("Texture layer %u with no texture bound to GLSL sampler uniform %s", unit, data.samplerName.c_str());
  }
  else
  {
    // Retrieve texture object.
    Texture* texture = Texture::findInstance(data.textureName);
    if (texture)
    {
      const GLenum textureTarget = texture->getTarget();

      if (textureTarget != textureTargets[unit])
      {
        if (textureTargets[unit])
          glDisable(textureTargets[unit]);

        glEnable(textureTarget);
        textureTargets[unit] = textureTarget;
      }
      
      if (data.textureName != cache.textureName)
      {
        glBindTexture(textureTarget, texture->textureID);
        cache.textureName = data.textureName;
      }

      if (data.combineMode != cache.combineMode)
      {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, data.combineMode);
        cache.combineMode = data.combineMode;
      }

      // Set texture environment color.
      if (data.combineColor != cache.combineColor)
      {
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, data.combineColor);
        cache.combineColor = data.combineColor;
      }

      if (data.sphereMapped != cache.sphereMapped)
      {
	setBooleanState(GL_TEXTURE_GEN_S, data.sphereMapped);
	setBooleanState(GL_TEXTURE_GEN_T, data.sphereMapped);

        if (data.sphereMapped)
        {
          glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
          glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        }

        cache.sphereMapped = data.sphereMapped;
      }

      if (!data.samplerName.empty())
	applySampler(*texture);
    }
    else
      Log::writeError("Texture layer uses non-existent texture %s", data.textureName.c_str());
  }

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeError("Error when applying texture layer %u: %s", unit, gluErrorString(error));

  data.dirty = cache.dirty = false;
}

bool TextureLayer::isSphereMapped(void) const
{
  return data.sphereMapped;
}

GLenum TextureLayer::getCombineMode(void) const
{
  return data.combineMode;
}

const ColorRGBA& TextureLayer::getCombineColor(void) const
{
  return data.combineColor;
}

const String& TextureLayer::getTextureName(void) const
{
  return data.textureName;
}

const String& TextureLayer::getSamplerName(void) const
{
  return data.samplerName;
}

unsigned int TextureLayer::getUnit(void) const
{
  return unit;
}

void TextureLayer::setSphereMapped(bool newState)
{
  data.sphereMapped = newState;
  data.dirty = true;
}

void TextureLayer::setCombineMode(GLenum newMode)
{
  data.combineMode = newMode;
  data.dirty = true;
}

void TextureLayer::setCombineColor(const ColorRGBA& newColor)
{
  data.combineColor = newColor;
  data.dirty = true;
}

void TextureLayer::setTextureName(const String& newName)
{
  data.textureName = newName;
  data.dirty = true;
}

void TextureLayer::setSamplerName(const String& newName)
{
  data.samplerName = newName;
  data.dirty = true;
}

void TextureLayer::setDefaults(void)
{
  data.setDefaults();
}

unsigned int TextureLayer::getUnitCount(void)
{
  if (!Context::get())
  {
    Log::writeError("Cannot query texture unit count before context creation");
    return 0;
  }

  if (!unitCount)
  {
    if (GLEW_ARB_multitexture)
      glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, (GLint*) &unitCount);
    else
      unitCount = 1;
  }

  return unitCount;
}

void TextureLayer::force(void) const
{
  if (GLEW_ARB_multitexture)
  {
    glActiveTextureARB(GL_TEXTURE0_ARB + unit);
    activeUnit = unit;
  }

  Data& cache = caches[unit];

  cache = data;

  glDisable(GL_TEXTURE_1D);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_3D);

  textureTargets[unit] = 0;

  setBooleanState(GL_TEXTURE_GEN_S, data.sphereMapped);
  setBooleanState(GL_TEXTURE_GEN_T, data.sphereMapped);
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, data.combineMode);
  glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, data.combineColor);

  Texture* texture = NULL;

  if (data.textureName.empty())
  {
    if (!data.samplerName.empty())
      Log::writeError("Texture layer %u with no texture bound to GLSL sampler uniform %s", unit, data.samplerName.c_str());
  }
  else
  {
    if (texture = Texture::findInstance(data.textureName))
    {
      const GLenum textureTarget = texture->getTarget();

      glEnable(textureTarget);
      glBindTexture(textureTarget, texture->textureID);

      textureTargets[unit] = textureTarget;

      if (!data.samplerName.empty())
	forceSampler(*texture);
    }
    else
      Log::writeError("Texture layer %u uses non-existent texture %s", unit, data.textureName.c_str());
  }

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeError("Error when forcing texture layer %u: %s", unit, gluErrorString(error));

  data.dirty = cache.dirty = false;
}

void TextureLayer::applySampler(Texture& texture) const
{
  forceSampler(texture);
}

void TextureLayer::forceSampler(Texture& texture) const
{
  ShaderProgram* program = ShaderProgram::getCurrent();
  if (!program)
  {
    Log::writeError("Cannot bind texture layer %u to GLSL sampler uniform %s without a current program", unit, data.samplerName.c_str());
    return;
  }

  ShaderUniform* sampler = program->getUniform(data.samplerName);
  if (!sampler)
  {
    Log::writeError("Texture layer %u bound to non-existent GLSL sampler uniform %s", unit, data.samplerName.c_str());
    return;
  }

  if (samplerTypeMap[sampler->getType()] != texture.getTarget())
  {
    Log::writeWarning("Type mismatch between texture %s and GLSL sampler uniform %s for texture layer %u", texture.getName().c_str(), data.samplerName.c_str(), unit);
    return;
  }

  sampler->setValue((int) unit);
}

void TextureLayer::setBooleanState(GLenum state, bool value) const
{
  if (value)
    glEnable(state);
  else
    glDisable(state);
}

void TextureLayer::onCreateContext(void)
{
  caches.resize(getUnitCount());
  textureTargets.insert(textureTargets.end(), getUnitCount(), 0);
}

void TextureLayer::onDestroyContext(void)
{
  caches.clear();
  textureTargets.clear();
  activeUnit = 0;
  unitCount = 0;
}

TextureLayer::DataList TextureLayer::caches;

TextureLayer::TargetList TextureLayer::textureTargets;

unsigned int TextureLayer::activeUnit = 0;

unsigned int TextureLayer::unitCount = 0;

///////////////////////////////////////////////////////////////////////

TextureLayer::Data::Data(void)
{
  setDefaults();
}

void TextureLayer::Data::setDefaults(void)
{
  dirty = true;
  sphereMapped = false;
  combineMode = GL_MODULATE;
  combineColor.set(1.f, 1.f, 1.f, 1.f);
  textureName.clear();
}

///////////////////////////////////////////////////////////////////////

TextureStack::TextureStack(void)
{
  if (!defaults.size())
  {
    const unsigned int textureUnitCount = TextureLayer::getUnitCount();

    for (unsigned int i = 0;  i < textureUnitCount;  i++)
      defaults.push_back(TextureLayer(i));
  }
}

void TextureStack::apply(void) const
{
  const unsigned int count = std::min(layers.size(), defaults.size());

  for (unsigned int i = 0;  i < count;  i++)
    layers[i].apply();

  for (unsigned int i = count;  i < defaults.size();  i++)
    defaults[i].apply();
}

TextureLayer& TextureStack::createTextureLayer(void)
{
  layers.push_back(TextureLayer(layers.size()));
  return layers.back();
}

void TextureStack::destroyTextureLayers(void)
{
  layers.clear();
}

unsigned int TextureStack::getTextureLayerCount(void) const
{
  return layers.size();
}

TextureLayer& TextureStack::getTextureLayer(unsigned int index)
{
  return layers[index];
}

const TextureLayer& TextureStack::getTextureLayer(unsigned int index) const
{
  return layers[index];
}

TextureStack::LayerList TextureStack::defaults;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
