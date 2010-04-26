///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/Portability.h>
#include <wendy/Core.h>
#include <wendy/Block.h>
#include <wendy/Color.h>
#include <wendy/Vector.h>
#include <wendy/Rectangle.h>
#include <wendy/Path.h>
#include <wendy/Pixel.h>
#include <wendy/Stream.h>
#include <wendy/Managed.h>
#include <wendy/Resource.h>
#include <wendy/Image.h>
#include <wendy/XML.h>
#include <wendy/Font.h>

#include <internal/FontIO.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

namespace
{

const unsigned int FONT_XML_VERSION = 1;

}

///////////////////////////////////////////////////////////////////////

FontCodecXML::FontCodecXML(void):
  FontCodec("XML font codec")
{
  addSuffix("font");
}

Font* FontCodecXML::read(const Path& path, const String& name)
{
  return FontCodec::read(path, name);
}

Font* FontCodecXML::read(Stream& stream, const String& name)
{
  fontName = name;

  if (!XML::Codec::read(stream))
  {
    font = NULL;
    return NULL;
  }

  return font.detachObject();
}

bool FontCodecXML::write(const Path& path, const Font& font)
{
  return FontCodec::write(path, font);
}

bool FontCodecXML::write(Stream& stream, const Font& font)
{
  try
  {
    setStream(&stream);

    beginElement("font");
    addAttribute("version", FONT_XML_VERSION);
    addAttribute("characters", font.getCharacters());

    // TODO: Figure out how to handle writing image data.

    endElement();

    setStream(NULL);
  }
  catch (Exception& exception)
  {
    Log::writeError("Failed to write font specification %s: %s",
                    font.getName().c_str(),
		    exception.what());
    setStream(NULL);
    return false;
  }

  return true;
}

bool FontCodecXML::onBeginElement(const String& name)
{
  if (name == "font")
  {
    if (font)
    {
      Log::writeError("Only one font per file allowed");
      return false;
    }

    const unsigned int version = readInteger("version");
    if (version != FONT_XML_VERSION)
    {
      Log::writeError("Font specification XML format version mismatch");
      return false;
    }

    String characters = readString("characters");
    if (characters.empty())
    {
      Log::writeError("No characters specified for font");
      return false;
    }

    String imageName = readString("image");

    Ref<Image> image = Image::readInstance(imageName);
    if (!image)
    {
      Log::writeError("Cannot find image %s for font", imageName.c_str());
      return false;
    }

    font = Font::createInstance(*image, characters, fontName);
    if (!font)
      return false;

    return true;
  }

  return true;
}

bool FontCodecXML::onEndElement(const String& name)
{
  return true;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
