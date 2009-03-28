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

VertexComponent::VertexComponent(Kind initKind,
                                 unsigned int initCount,
				 Type initType):
  kind(initKind),
  count(initCount),
  type(initType)
{
}

bool VertexComponent::operator == (const VertexComponent& other) const
{
  return kind == other.kind && count == other.count && type == other.type;
}

bool VertexComponent::operator != (const VertexComponent& other) const
{
  return kind != other.kind || count != other.count || type != other.type;
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

VertexComponent::Kind VertexComponent::getKind(void) const
{
  return kind;
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

VertexFormat::VertexFormat(const std::string& specification)
{
  if (!createComponents(specification))
    throw Exception("Invalid vertex format specification");
}

bool VertexFormat::createComponent(VertexComponent::Kind kind,
                                   unsigned int count,
				   VertexComponent::Type type)
{
  if (kind == VertexComponent::GENERIC)
  {
    if (count < 1 || count > 4)
    {
      Log::writeError("Generic vertex components must have between 1 and 4 elements");
      return false;
    }
  }
  else
  {
    if (count < 2 || count > 4)
    {
      Log::writeError("Regular vertex components must have between 2 and 4 elements");
      return false;
    }

    if (kind != VertexComponent::TEXCOORD)
    {
      if (findComponent(kind))
      {
	Log::writeError("Invalid duplicate regular vertex component");
	return false;
      }
    }
  }

  size_t size = getSize();

  components.push_back(VertexComponent(kind, count, type));
  VertexComponent& component = components.back();
  component.offset = size;
  return true;
}

bool VertexFormat::createComponents(const std::string& specification)
{
  String::const_iterator command = specification.begin();
  while (command != specification.end())
  {
    if (*command < '1' || *command > '4')
    {
      Log::writeError("Invalid vertex component element count");
      return false;
    }

    unsigned int count = *command++ - '0';
    if (command == specification.end())
    {
      Log::writeError("Unexpected end of vertex format specification");
      return false;
    }

    VertexComponent::Type type;

    switch (tolower(*command++))
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
	return false;
    }
    
    if (command == specification.end())
    {
      Log::writeError("Unexpected end of vertex format specification");
      return false;
    }

    VertexComponent::Kind kind;

    switch (tolower(*command++))
    {
      case 'v':
	kind = VertexComponent::POSITION;
	break;
      case 't':
	kind = VertexComponent::TEXCOORD;
	break;
      case 'c':
	kind = VertexComponent::COLOR;
	break;
      case 'n':
	kind = VertexComponent::NORMAL;
	break;
      case 'a':
	kind = VertexComponent::GENERIC;
	break;
      default:
	Log::writeError("Invalid vertex component kind");
	return false;
    }

    if (!createComponent(kind, count, type))
      return false;
  }

  return true;
}

void VertexFormat::destroyComponents(void)
{
  components.clear();
}

const VertexComponent* VertexFormat::findComponent(VertexComponent::Kind kind) const
{
  for (ComponentList::const_iterator i = components.begin();  i != components.end();  i++)
    if ((*i).getKind() == kind)
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

    switch ((*i).kind)
    {
      case VertexComponent::POSITION:
	result << 'v';
	break;
      case VertexComponent::TEXCOORD:
	result << 't';
	break;
      case VertexComponent::COLOR:
	result << 'c';
	break;
      case VertexComponent::NORMAL:
	result << 'n';
	break;
      case VertexComponent::GENERIC:
	result << 'a';
	break;
      default:
	Log::writeError("Invalid vertex component kind %u", (*i).kind);
	break;
    }
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
