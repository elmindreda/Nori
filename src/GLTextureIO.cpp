///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2008 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/GLTextureIO.h>

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

const unsigned int TEXTURE_XML_VERSION = 1;

}

///////////////////////////////////////////////////////////////////////

TextureCodec::TextureCodec(void):
  ResourceCodec<Texture>("XML texture specification codec")
{
  addSuffix("texture");
}

Texture* TextureCodec::read(const Path& path, const String& name)
{
  return ResourceCodec<Texture>::read(path, name);
}

Texture* TextureCodec::read(Stream& stream, const String& name)
{
  textureName = name;

  if (!XML::Codec::read(stream))
  {
    texture = NULL;
    return NULL;
  }

  if (!texture)
  {
    Log::writeError("No texture specification found in file");
    return NULL;
  }

  return texture.detachObject();
}

bool TextureCodec::write(const Path& path, const Texture& texture)
{
  return ResourceCodec<Texture>::write(path, texture);
}

bool TextureCodec::write(Stream& stream, const Texture& texture)
{
  try
  {
    setStream(&stream);

    beginElement("texture");
    addAttribute("version", TEXTURE_XML_VERSION);

    // TODO: Uhm...

    endElement();

    setStream(NULL);
  }
  catch (Exception& exception)
  {
    Log::writeError("Failed to write texture specification \'%s\': %s",
                    texture.getName().c_str(),
		    exception.what());
    setStream(NULL);
    return false;
  }

  return true;
}

bool TextureCodec::onBeginElement(const String& name)
{
  if (name == "texture")
  {
    if (texture)
    {
      Log::writeError("Only one texture specification per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != TEXTURE_XML_VERSION)
    {
      Log::writeError("Texture specification XML format version mismatch");
      return false;
    }

    String imageName = readString("image");
    if (imageName.empty())
    {
      Log::writeError("No image specified for texture %s", textureName.c_str());
      return false;
    }

    Ref<Image> image = Image::readInstance(imageName);
    if (!image)
    {
      Log::writeError("Failed to load image %s for texture %s",
                      imageName.c_str(),
		      textureName.c_str());
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

      defaultValue = (Texture::DEFAULT | Texture::DONT_GROW) ? true : false;
      if (readBoolean("dont-grow", defaultValue) != defaultValue)
	flags ^= Texture::DONT_GROW;
    }

    texture = Texture::createInstance(*image, flags, textureName);
    if (!texture)
      return false;

    return true;
  }

  return true;
}

bool TextureCodec::onEndElement(const String& name)
{
  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
