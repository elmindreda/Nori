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

#include <moira/Config.h>
#include <moira/Portability.h>
#include <moira/Core.h>
#include <moira/Vector.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLVertex.h>

#include <ctype.h>

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
  if (!addComponents(specification))
    throw Exception("Invalid vertex format specification");
}

bool VertexFormat::addComponent(const VertexComponent& component)
{
  if (findComponent(component.getKind()))
    return false;

  const size_t size = getSize();
  components.push_back(component);
  components.back().offset = size;
  return true;
}

bool VertexFormat::addComponents(const std::string& specification)
{
  ComponentList temp;

  size_t size = getSize();

  std::string::const_iterator command = specification.begin();
  while (command != specification.end())
  {
    if (*command < '2' || *command > '4')
      return false;

    unsigned int count = *command++ - '0';
    if (command == specification.end())
      return false;

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
      return false;

    VertexComponent::Kind kind;

    switch (tolower(*command++))
    {
      case 'v':
	kind = VertexComponent::VERTEX;
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
      default:
	return false;
    }

    if (findComponent(kind))
      return false;

    temp.push_back(VertexComponent(kind, count, type));
    temp.back().offset = size;
    size += temp.back().getSize();
  }

  for (ComponentList::iterator i = temp.begin();  i != temp.end();  i++)
    components.push_back(*i);

  return true;
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

///////////////////////////////////////////////////////////////////////

void Vertex3fv::send(void) const
{
  glVertex3fv(position);
}

const VertexFormat Vertex3fv::format("3fv");

///////////////////////////////////////////////////////////////////////

void Vertex3fn3fv::send(void) const
{
  glNormal3fv(normal);
  glVertex3fv(position);
}

const VertexFormat Vertex3fn3fv::format("3fn3fv");

///////////////////////////////////////////////////////////////////////

void Vertex2ft2fv::send(void) const
{
  glTexCoord2fv(mapping);
  glVertex2fv(position);
}

const VertexFormat Vertex2ft2fv::format("2ft2fv");

///////////////////////////////////////////////////////////////////////

void Vertex2ft3fv::send(void) const
{
  glTexCoord2fv(mapping);
  glVertex3fv(position);
}

const VertexFormat Vertex2ft3fv::format("2ft3fv");

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
