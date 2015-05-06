///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
// Copyright (c) 2009 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <nori/Config.hpp>
#include <nori/Core.hpp>
#include <nori/Vertex.hpp>

#include <cctype>
#include <sstream>

namespace nori
{

VertexComponent::VertexComponent(const char* name, size_t count):
  m_name(name),
  m_count(count)
{
}

VertexFormat::VertexFormat()
{
}

VertexFormat::VertexFormat(const char* specification)
{
  if (!createComponents(specification))
    throw Exception("Invalid vertex format specification");
}

bool VertexFormat::createComponent(const char* name, size_t count)
{
  if (count < 1 || count > 4)
  {
    logError("Vertex components must have between 1 and 4 elements");
    return false;
  }

  if (findComponent(name))
  {
    logError("Duplicate vertex component name %s detected; vertex "
             "components must have unique names",
             name);
    return false;
  }

  const size_t offset = size();

  m_components.push_back(VertexComponent(name, count));
  VertexComponent& component = m_components.back();
  component.m_offset = offset;
  return true;
}

bool VertexFormat::createComponents(const char* specification)
{
  const char* c = specification;
  while (*c != '\0')
  {
    if (*c < '1' || *c > '4')
    {
      logError("Invalid vertex component element count");
      return false;
    }

    const size_t count = *c - '0';

    if (*(++c) == '\0')
    {
      logError("Unexpected end of vertex format specification");
      return false;
    }

    if (std::tolower(*c) != 'f')
    {
      if (std::isgraph(*c))
        logError("Invalid vertex component type %c", *c);
      else
        logError("Invalid vertex component type 0x%02x", *c);

      return false;
    }

    if (*(++c) == '\0')
    {
      logError("Unexpected end of vertex format specification");
      return false;
    }

    if (*(c++) != ':')
    {
      logError("Invalid vertex component specification; expected :");
      return false;
    }

    std::string name;

    while (*c != '\0' && *c != ' ')
      name += *c++;

    if (!createComponent(name.c_str(), count))
      return false;

    while (*c != '\0' && *c == ' ')
      c++;
  }

  return true;
}

void VertexFormat::destroyComponents()
{
  m_components.clear();
}

const VertexComponent* VertexFormat::findComponent(const char* name) const
{
  for (const VertexComponent& c : m_components)
  {
    if (c.name() == name)
      return &c;
  }

  return nullptr;
}

bool VertexFormat::operator == (const VertexFormat& other) const
{
  return m_components == other.m_components;
}

bool VertexFormat::operator != (const VertexFormat& other) const
{
  return !(m_components == other.m_components);
}

size_t VertexFormat::size() const
{
  size_t size = 0;

  for (const VertexComponent& c : m_components)
    size += c.size();

  return size;
}

std::string stringCast(const VertexFormat& format)
{
  std::ostringstream result;

  for (const VertexComponent& c : format.components())
    result << c.elementCount() << 'f' << ':' << c.name() << ' ';

  return result.str();
}

const VertexFormat Vertex3fv::format("3f:vPosition");

const VertexFormat Vertex3fn3fv::format("3f:vNormal 3f:vPosition");

const VertexFormat Vertex2fv::format("2f:vPosition");

const VertexFormat Vertex2ft2fv::format("2f:vTexCoord 2f:vPosition");

const VertexFormat Vertex2ft3fv::format("2f:vTexCoord 3f:vPosition");

const VertexFormat Vertex4fc2ft3fv::format("4f:vColor 2f:vTexCoord 3f:vPosition");

const VertexFormat Vertex3fn2ft3fv::format("3f:vNormal 2f:vTexCoord 3f:vPosition");

} /*namespace nori*/

