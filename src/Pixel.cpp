///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you
//    must not claim that you wrote the original software. If you use
//    this software in a product, an acknowledgment in the product
//    documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and
//    must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
///////////////////////////////////////////////////////////////////////

#include <wendy/Config.h>
#include <wendy/Core.h>
#include <wendy/Pixel.h>

#include <cstring>
#include <sstream>
#include <cctype>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

PixelFormat::PixelFormat(Semantic initSemantic, Type initType):
  semantic(initSemantic),
  type(initType)
{
}

PixelFormat::PixelFormat(const char* specification):
  semantic(NONE),
  type(DUMMY)
{
  const char* c = specification;

  while (std::isspace(*c))
    c++;

  String semanticName;

  while (std::isalpha(*c))
    semanticName += std::tolower(*c++);

  if (semanticName == "l")
    semantic = L;
  else if (semanticName == "la")
    semantic = LA;
  else if (semanticName == "rgb")
    semantic = RGB;
  else if (semanticName == "rgba")
    semantic = RGBA;
  else if (semanticName == "depth")
    semantic = DEPTH;
  else
    throw Exception("Invalid pixel format semantic name");

  String typeName;

  while (std::isdigit(*c) || std::isalpha(*c))
    typeName += std::tolower(*c++);

  if (typeName == "8")
    type = UINT8;
  else if (typeName == "16")
    type = UINT16;
  else if (typeName == "24")
    type = UINT24;
  else if (typeName == "32")
    type = UINT32;
  else if (typeName == "16f")
    type = FLOAT16;
  else if (typeName == "32f")
    type = FLOAT32;
  else
    throw Exception("Invalid pixel format type name");
}

bool PixelFormat::operator == (const PixelFormat& other) const
{
  return semantic == other.semantic && type == other.type;
}

bool PixelFormat::operator != (const PixelFormat& other) const
{
  return semantic != other.semantic || type != other.type;
}

bool PixelFormat::isValid() const
{
  return semantic != NONE && type != DUMMY;
}

size_t PixelFormat::getSize() const
{
  return getChannelSize() * getChannelCount();
}

size_t PixelFormat::getChannelSize() const
{
  switch (type)
  {
    case DUMMY:
      return 0;
    case UINT8:
      return 1;
    case UINT16:
    case FLOAT16:
      return 2;
    case UINT24:
      return 3;
    case UINT32:
    case FLOAT32:
      return 4;
    default:
      panic("Invalid pixel format type %i", type);
  }
}

PixelFormat::Type PixelFormat::getType() const
{
  return type;
}

PixelFormat::Semantic PixelFormat::getSemantic() const
{
  return semantic;
}

unsigned int PixelFormat::getChannelCount() const
{
  switch (semantic)
  {
    case NONE:
      return 0;
    case L:
    case DEPTH:
      return 1;
    case LA:
      return 2;
    case RGB:
      return 3;
    case RGBA:
      return 4;
    default:
      panic("Invalid pixel format semantic %i", semantic);
  }
}

String PixelFormat::asString() const
{
  std::ostringstream result;

  switch (semantic)
  {
    case L:
      result << "l";
      break;
    case LA:
      result << "la";
      break;
    case RGB:
      result << "rgb";
      break;
    case RGBA:
      result << "rgba";
      break;
    case DEPTH:
      result << "depth";
      break;
    default:
      panic("Invalid pixel format semantic %i", semantic);
  }

  switch (type)
  {
    case UINT8:
      result << "8";
      break;
    case UINT16:
      result << "16";
      break;
    case UINT24:
      result << "24";
      break;
    case UINT32:
      result << "32";
      break;
    case FLOAT16:
      result << "16f";
      break;
    case FLOAT32:
      result << "32f";
      break;
    default:
      panic("Invalid pixel format type %i", type);
  }

  return result.str();
}

const PixelFormat PixelFormat::L8(PixelFormat::L, PixelFormat::UINT8);
const PixelFormat PixelFormat::L16(PixelFormat::L, PixelFormat::UINT16);
const PixelFormat PixelFormat::L16F(PixelFormat::L, PixelFormat::FLOAT16);
const PixelFormat PixelFormat::L32F(PixelFormat::L, PixelFormat::FLOAT32);

const PixelFormat PixelFormat::LA8(PixelFormat::LA, PixelFormat::UINT8);
const PixelFormat PixelFormat::LA16(PixelFormat::LA, PixelFormat::UINT16);
const PixelFormat PixelFormat::LA16F(PixelFormat::LA, PixelFormat::FLOAT16);
const PixelFormat PixelFormat::LA32F(PixelFormat::LA, PixelFormat::FLOAT32);

const PixelFormat PixelFormat::RGB8(PixelFormat::RGB, PixelFormat::UINT8);
const PixelFormat PixelFormat::RGB16(PixelFormat::RGB, PixelFormat::UINT16);
const PixelFormat PixelFormat::RGB16F(PixelFormat::RGB, PixelFormat::FLOAT16);
const PixelFormat PixelFormat::RGB32F(PixelFormat::RGB, PixelFormat::FLOAT32);

const PixelFormat PixelFormat::RGBA8(PixelFormat::RGBA, PixelFormat::UINT8);
const PixelFormat PixelFormat::RGBA16(PixelFormat::RGBA, PixelFormat::UINT16);
const PixelFormat PixelFormat::RGBA16F(PixelFormat::RGBA, PixelFormat::FLOAT16);
const PixelFormat PixelFormat::RGBA32F(PixelFormat::RGBA, PixelFormat::FLOAT32);

const PixelFormat PixelFormat::DEPTH16(PixelFormat::DEPTH, PixelFormat::UINT16);
const PixelFormat PixelFormat::DEPTH24(PixelFormat::DEPTH, PixelFormat::UINT24);
const PixelFormat PixelFormat::DEPTH32(PixelFormat::DEPTH, PixelFormat::UINT32);
const PixelFormat PixelFormat::DEPTH16F(PixelFormat::DEPTH, PixelFormat::FLOAT16);
const PixelFormat PixelFormat::DEPTH32F(PixelFormat::DEPTH, PixelFormat::FLOAT32);

///////////////////////////////////////////////////////////////////////

PixelTransform::~PixelTransform()
{
}

///////////////////////////////////////////////////////////////////////

bool RGBtoRGBA::supports(const PixelFormat& targetFormat,
                         const PixelFormat& sourceFormat)
{
  if (targetFormat.getType() != sourceFormat.getType())
    return false;

  if (targetFormat.getSemantic() != PixelFormat::RGBA ||
      sourceFormat.getSemantic() != PixelFormat::RGB)
  {
    return false;
  }

  return true;
}

void RGBtoRGBA::convert(void* target,
                        const PixelFormat& targetFormat,
                        const void* source,
                        const PixelFormat& sourceFormat,
                        size_t count)
{
  size_t channelSize = targetFormat.getChannelSize();
  size_t targetSize = targetFormat.getSize();
  size_t sourceSize = sourceFormat.getSize();

  while (count--)
  {
    std::memcpy(target, source, sourceSize);
    std::memset((char*) target + sourceSize, 0, channelSize);
    target = (char*) target + targetSize;
    source = (char*) source + sourceSize;
  }
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
