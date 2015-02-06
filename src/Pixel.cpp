///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#include <wendy/Config.hpp>
#include <wendy/Core.hpp>
#include <wendy/Pixel.hpp>

#include <cstring>
#include <sstream>
#include <cctype>

namespace wendy
{

PixelFormat::PixelFormat(Semantic semantic, Type type):
  m_semantic(semantic),
  m_type(type)
{
}

PixelFormat::PixelFormat(const char* specification):
  m_semantic(NONE),
  m_type(DUMMY)
{
  const char* c = specification;

  while (std::isspace(*c))
    c++;

  std::string semanticName;

  while (std::isalpha(*c))
    semanticName += std::tolower(*c++);

  if (semanticName == "l")
    m_semantic = L;
  else if (semanticName == "la")
    m_semantic = LA;
  else if (semanticName == "rgb")
    m_semantic = RGB;
  else if (semanticName == "rgba")
    m_semantic = RGBA;
  else if (semanticName == "depth")
    m_semantic = DEPTH;
  else
    throw Exception("Invalid pixel format semantic name");

  std::string typeName;

  while (std::isdigit(*c) || std::isalpha(*c))
    typeName += std::tolower(*c++);

  if (typeName == "8")
    m_type = UINT8;
  else if (typeName == "16")
    m_type = UINT16;
  else if (typeName == "24")
    m_type = UINT24;
  else if (typeName == "32")
    m_type = UINT32;
  else if (typeName == "16f")
    m_type = FLOAT16;
  else if (typeName == "32f")
    m_type = FLOAT32;
  else
    throw Exception("Invalid pixel format type name");
}

bool PixelFormat::operator == (const PixelFormat& other) const
{
  return m_semantic == other.m_semantic && m_type == other.m_type;
}

bool PixelFormat::operator != (const PixelFormat& other) const
{
  return m_semantic != other.m_semantic || m_type != other.m_type;
}

size_t PixelFormat::channelSize() const
{
  switch (m_type)
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
      panic("Invalid pixel format type %i", m_type);
  }
}

uint PixelFormat::channelCount() const
{
  switch (m_semantic)
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
      panic("Invalid pixel format semantic %i", m_semantic);
  }
}

const char* stringCast(PixelFormat::Semantic semantic)
{
  switch (semantic)
  {
    case PixelFormat::L:
      return "l";
    case PixelFormat::LA:
      return "la";
    case PixelFormat::RGB:
      return "rgb";
    case PixelFormat::RGBA:
      return "rgba";
    case PixelFormat::DEPTH:
      return "depth";
    default:
      panic("Invalid pixel format semantic %i", semantic);
  }
}

const char* stringCast(PixelFormat::Type type)
{
  switch (type)
  {
    case PixelFormat::UINT8:
      return "8";
    case PixelFormat::UINT16:
      return "16";
    case PixelFormat::UINT24:
      return "24";
    case PixelFormat::UINT32:
      return "32";
    case PixelFormat::FLOAT16:
      return "16f";
    case PixelFormat::FLOAT32:
      return "32f";
    default:
      panic("Invalid pixel format type %i", type);
  }
}

std::string stringCast(PixelFormat format)
{
  return std::string(stringCast(format.semantic())) + stringCast(format.type());
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

} /*namespace wendy*/

