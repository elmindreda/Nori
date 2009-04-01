///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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

#include <wendy/GLVertex.h>

#include <cctype>
#include <sstream>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

namespace
{

Pattern componentNamePattern("[a-zA-Z_][a-zA-Z0-9_]+");

}

///////////////////////////////////////////////////////////////////////

VertexComponent::VertexComponent(const String& initName,
                                 unsigned int initCount,
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
    case DOUBLE:
      return sizeof(double) * count;
    case FLOAT:
      return sizeof(float) * count;
    case INT:
      return sizeof(int) * count;
    case SHORT:
      return sizeof(short) * count;
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

unsigned int VertexComponent::getElementCount(void) const
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
                                   unsigned int count,
				   VertexComponent::Type type)
{
  if (count < 1 || count > 4)
  {
    Log::writeError("Vertex components must have between 1 and 4 elements");
    return false;
  }

  if (findComponent(name))
  {
    Log::writeError("Duplicate vertex component %s detected; vertex components must have unique names",
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
      Log::writeError("Invalid vertex component element count");
      return false;
    }

    const unsigned int count = *command - '0';
    if (++command == specification.end())
    {
      Log::writeError("Unexpected end of vertex format specification");
      return false;
    }

    VertexComponent::Type type;

    switch (tolower(*command))
    {
      case 'd':
	type = VertexComponent::DOUBLE;
	break;
      case 'f':
	type = VertexComponent::FLOAT;
	break;
      case 'i':
	type = VertexComponent::INT;
	break;
      case 's':
	type = VertexComponent::SHORT;
	break;
      default:
	if (std::isgraph(*command))
	  Log::writeError("Invalid vertex component type \'%c\'", *command);
	else
	  Log::writeError("Invalid vertex component type 0x%02x", *command);
	return false;
    }
    
    if (++command == specification.end())
    {
      Log::writeError("Unexpected end of vertex format specification");
      return false;
    }

    if (*command != ':')
    {
      Log::writeError("Invalid vertex component specification; expected \':\'");
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
    if ((*i).getName() == name)
      return &(*i);

  return NULL;
}

const VertexComponent& VertexFormat::operator [] (unsigned int index) const
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
    size += (*i).getSize();

  return size;
}

unsigned int VertexFormat::getComponentCount(void) const
{
  return (unsigned int) components.size();
}

String VertexFormat::getSpecification(void) const
{
  std::stringstream result;

  for (ComponentList::const_iterator i = components.begin();  i != components.end();  i++)
  {
    result << (*i).count;

    switch ((*i).type)
    {
      case VertexComponent::DOUBLE:
	result << 'd';
	break;
      case VertexComponent::FLOAT:
	result << 'f';
	break;
      case VertexComponent::INT:
	result << 'i';
	break;
      case VertexComponent::SHORT:
	result << 's';
	break;
      default:
	Log::writeError("Invalid vertex component type %u", (*i).type);
	break;
    }

    result << ':' << (*i).name << ' ';
  }

  result.str();
}

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex3fv::format("3fv");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex3fn3fv::format("3fn3fv");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex2fv::format("2fv");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex2ft2fv::format("2ft2fv");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex2ft3fv::format("2ft3fv");

///////////////////////////////////////////////////////////////////////

const VertexFormat Vertex4fc2ft3fv::format("4fc2ft3fv");

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
