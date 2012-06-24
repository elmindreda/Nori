///////////////////////////////////////////////////////////////////////
// Wendy core library
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

#include <wendy/Config.h>
#include <wendy/Core.h>
#include <wendy/Vertex.h>

#include <cctype>
#include <sstream>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

VertexComponent::VertexComponent(const char* initName,
                                 size_t initCount,
                                 Type initType):
  name(initName),
  count(initCount),
  type(initType)
{
}

bool VertexComponent::operator == (const VertexComponent& other) const
{
  return name == other.name && count == other.count && type == other.type;
}

bool VertexComponent::operator != (const VertexComponent& other) const
{
  return name != other.name || count != other.count || type != other.type;
}

size_t VertexComponent::getSize() const
{
  switch (type)
  {
    case FLOAT32:
      return 4 * count;
    default:
      panic("Invalid vertex component type");
  }
}

const String& VertexComponent::getName() const
{
  return name;
}

VertexComponent::Type VertexComponent::getType() const
{
  return type;
}

size_t VertexComponent::getOffset() const
{
  return offset;
}

size_t VertexComponent::getElementCount() const
{
  return count;
}

///////////////////////////////////////////////////////////////////////

VertexFormat::VertexFormat()
{
}

VertexFormat::VertexFormat(const char* specification)
{
  if (!createComponents(specification))
    throw Exception("Invalid vertex format specification");
}

bool VertexFormat::createComponent(const char* name,
                                   size_t count,
                                   VertexComponent::Type type)
{
  if (count < 1 || count > 4)
  {
    logError("Vertex components must have between 1 and 4 elements");
    return false;
  }

  if (findComponent(name))
  {
    logError("Duplicate vertex component name \'%s\' detected; vertex "
             "components must have unique names",
             name);
    return false;
  }

  const size_t size = getSize();

  components.push_back(VertexComponent(name, count, type));
  VertexComponent& component = components.back();
  component.offset = size;
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

    VertexComponent::Type type;

    switch (std::tolower(*c))
    {
      case 'f':
        type = VertexComponent::FLOAT32;
        break;
      default:
        if (std::isgraph(*c))
          logError("Invalid vertex component type \'%c\'", *c);
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
      logError("Invalid vertex component specification; expected \':\'");
      return false;
    }

    String name;

    while (*c != '\0' && *c != ' ')
      name += *c++;

    if (!createComponent(name.c_str(), count, type))
      return false;

    while (*c != '\0' && *c == ' ')
      c++;
  }

  return true;
}

void VertexFormat::destroyComponents()
{
  components.clear();
}

const VertexComponent* VertexFormat::findComponent(const char* name) const
{
  for (auto c = components.begin();  c != components.end();  c++)
    if (c->getName() == name)
      return &(*c);

  return NULL;
}

const VertexComponent& VertexFormat::operator [] (size_t index) const
{
  return components[index];
}

bool VertexFormat::operator == (const VertexFormat& other) const
{
  return components == other.components;
}

bool VertexFormat::operator != (const VertexFormat& other) const
{
  return !(components == other.components);
}

size_t VertexFormat::getSize() const
{
  size_t size = 0;

  for (auto c = components.begin();  c != components.end();  c++)
    size += c->getSize();

  return size;
}

size_t VertexFormat::getComponentCount() const
{
  return (size_t) components.size();
}

String VertexFormat::asString() const
{
  std::ostringstream result;

  for (auto c = components.begin();  c != components.end();  c++)
  {
    result << c->count;

    switch (c->type)
    {
      case VertexComponent::FLOAT32:
        result << 'f';
        break;
      default:
        panic("Invalid vertex component type");
    }

    result << ':' << c->name << ' ';
  }

  return result.str();
}

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex3fv::format("3f:wyPosition");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex3fn3fv::format("3f:wyNormal 3f:wyPosition");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex2fv::format("2f:wyPosition");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex2ft2fv::format("2f:wyTexCoord 2f:wyPosition");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex2ft3fv::format("2f:wyTexCoord 3f:wyPosition");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex4fc2ft3fv::format("4f:wyColor 2f:wyTexCoord 3f:wyPosition");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex3fn2ft3fv::format("3f:wyNormal 2f:wyTexCoord 3f:wyPosition");

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
