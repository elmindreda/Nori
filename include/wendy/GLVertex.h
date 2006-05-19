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
#ifndef WENDY_GLVERTEX_H
#define WENDY_GLVERTEX_H
///////////////////////////////////////////////////////////////////////

#if WENDY_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <vector>
#include <string>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class VertexComponent
{
  friend class VertexFormat;
public:
  /*! Vertex component kind enumeration.
   */
  enum Kind
  {
    /*! Component is a vertex position.
     */
    VERTEX,
    /*! Component is a texture coordinate.
     */
    TEXCOORD,
    /*! Component is a color value.
     */
    COLOR,
    /*! Component is a vertex normal.
     */
    NORMAL,
  };
  /*! Vertex format element type enumeration.
   */
  enum Type
  {
    /*! Component elements are doubles.
     */
    DOUBLE = GL_DOUBLE,
    /*! Component elements are floats.
     */
    FLOAT = GL_FLOAT,
    /*! Component elements are ints.
     */
    INT = GL_INT,
    /*! Component elements are shorts.
     */
    SHORT = GL_SHORT,
  };
  /*! Constructor.
   */
  VertexComponent(Kind initKind, unsigned int initCount = 3, Type initType = FLOAT);
  /*! @return The size, in bytes, of this component.
   */
  size_t getSize(void) const;
  /*! @return The kind of this component.
   */
  Kind getKind(void) const;
  /*! @return The type of the elements in this component.
   */
  Type getType(void) const;
  /*! @return The offset, in bytes, of this component in a vertex.
   */
  size_t getOffset(void) const;
  /*! @return The number of elements in this component.
   */
  unsigned int getElementCount(void) const;
private:
  Kind kind;
  Type type;
  size_t offset;
  unsigned int count;
};

///////////////////////////////////////////////////////////////////////

class VertexFormat
{
public:
  VertexFormat(void);
  VertexFormat(const std::string& specification) throw(Exception);
  bool addComponent(const VertexComponent& component);
  bool addComponents(const std::string& specification);
  const VertexComponent* findComponent(VertexComponent::Kind kind) const;
  const VertexComponent& operator [] (unsigned int index) const;
  size_t getSize(void) const;
  unsigned int getComponentCount(void) const;
private:
  typedef std::vector<VertexComponent> ComponentList;
  ComponentList components;
};

///////////////////////////////////////////////////////////////////////

class Vertex3fv
{
public:
  void send(void) const;
  Vector3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

class Vertex3fn3fv
{
public:
  void send(void) const;
  Vector3 normal;
  Vector3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

class Vertex2ft2fv
{
public:
  void send(void) const;
  Vector2 mapping;
  Vector2 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

class Vertex2ft3fv
{
public:
  void send(void) const;
  Vector2 mapping;
  Vector3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLVERTEX_H*/
///////////////////////////////////////////////////////////////////////
