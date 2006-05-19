///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2005 Camilla Berglund <elmindreda@home.se>
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

VertexFormat::VertexFormat(const std::string& specification) throw(Exception)
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
  ComponentList components;

  size_t size = getSize();

  std::string::const_iterator command = specification.begin();
  while (command != specification.end())
  {
    if (*command < '2' || *command > '4')
      return false;

    unsigned int count = *command++ - '0';

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

    components.push_back(VertexComponent(kind, count, type));
    components.back().offset = size;
    size += components.back().getSize();
  }

  for (ComponentList::iterator i = components.begin();  i != components.end();  i++)
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

size_t VertexFormat::getSize(void) const
{
  size_t size = 0;

  for (ComponentList::const_iterator i = components.begin();  i != components.end();  i++)
    size += (*i).getSize();

  return size;
}

unsigned int VertexFormat::getComponentCount(void) const
{
  return components.size();
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
