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
#include <wendy/Vector.h>
#include <wendy/Color.h>
#include <wendy/Vertex.h>

#include <cctype>
#include <sstream>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

VertexComponent::VertexComponent(const String& initName,
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

size_t VertexComponent::getSize(void) const
{
  switch (type)
  {
    case FLOAT32:
    case INT32:
      return 4 * count;
    case INT16:
      return 2 * count;
    case INT8:
      return 1 * count;
    default:
      return 0;
  }
}

const String& VertexComponent::getName(void) const
{
  return name;
}

VertexComponent::Type VertexComponent::getType(void) const
{
  return type;
}

size_t VertexComponent::getOffset(void) const
{
  return offset;
}

size_t VertexComponent::getElementCount(void) const
{
  return count;
}

///////////////////////////////////////////////////////////////////////

VertexFormat::VertexFormat(void)
{
}

VertexFormat::VertexFormat(const String& specification)
{
  if (!createComponents(specification))
    throw Exception("Invalid vertex format specification");
}

bool VertexFormat::createComponent(const String& name,
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
    logError("Duplicate vertex component name \'%s\' detected; vertex components must have unique names",
             name.c_str());
    return false;
  }

  const size_t size = getSize();

  components.push_back(VertexComponent(name, count, type));
  VertexComponent& component = components.back();
  component.offset = size;
  return true;
}

bool VertexFormat::createComponents(const String& specification)
{
  String::const_iterator command = specification.begin();
  while (command != specification.end())
  {
    if (*command < '1' || *command > '4')
    {
      logError("Invalid vertex component element count");
      return false;
    }

    const size_t count = *command - '0';
    if (++command == specification.end())
    {
      logError("Unexpected end of vertex format specification");
      return false;
    }

    VertexComponent::Type type;

    switch (tolower(*command))
    {
      case 'f':
	type = VertexComponent::FLOAT32;
	break;
      case 'i':
	type = VertexComponent::INT32;
	break;
      case 's':
	type = VertexComponent::INT16;
	break;
      case 'b':
	type = VertexComponent::INT8;
	break;
      default:
	if (std::isgraph(*command))
	  logError("Invalid vertex component type \'%c\'", *command);
	else
	  logError("Invalid vertex component type 0x%02x", *command);
	return false;
    }

    if (++command == specification.end())
    {
      logError("Unexpected end of vertex format specification");
      return false;
    }

    if (*command != ':')
    {
      logError("Invalid vertex component specification; expected \':\'");
      return false;
    }

    String name;

    while (++command != specification.end() && *command != ' ')
      name.append(1, *command);

    if (!createComponent(name, count, type))
      return false;

    while (command != specification.end() && *command == ' ')
      command++;
  }

  return true;
}

void VertexFormat::destroyComponents(void)
{
  components.clear();
}

const VertexComponent* VertexFormat::findComponent(const String& name) const
{
  for (ComponentList::const_iterator i = components.begin();  i != components.end();  i++)
    if (i->getName() == name)
      return &(*i);

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

size_t VertexFormat::getSize(void) const
{
  size_t size = 0;

  for (ComponentList::const_iterator i = components.begin();  i != components.end();  i++)
    size += i->getSize();

  return size;
}

size_t VertexFormat::getComponentCount(void) const
{
  return (size_t) components.size();
}

String VertexFormat::asString(void) const
{
  std::ostringstream result;

  for (ComponentList::const_iterator i = components.begin();  i != components.end();  i++)
  {
    result << i->count;

    switch (i->type)
    {
      case VertexComponent::FLOAT32:
	result << 'f';
	break;
      case VertexComponent::INT32:
	result << 'i';
	break;
      case VertexComponent::INT16:
	result << 's';
	break;
      case VertexComponent::INT8:
	result << 'b';
	break;
      default:
        return "invalid";
    }

    result << ':' << i->name << ' ';
  }

  return result.str();
}

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex3fv::format("3f:position");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex3fn3fv::format("3f:normal 3f:position");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex2fv::format("2f:position");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex2ft2fv::format("2f:mapping 2f:position");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex2ft3fv::format("2f:mapping 3f:position");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex4fc2ft3fv::format("4f:color 2f:mapping 3f:position");

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
