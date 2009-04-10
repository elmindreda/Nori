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

/*! @brief Vertex format component descriptor.
 *  @ingroup opengl
 *
 *  This class describes a single logical component of a vertex format.
 *  A component may have multiple (up to four) members.
 */
class VertexComponent
{
  friend class VertexFormat;
public:
  /*! Vertex format element type enumeration.
   */
  enum Type
  {
    /*! Component elements are doubles.
     */
    DOUBLE,
    /*! Component elements are floats.
     */
    FLOAT,
    /*! Component elements are ints.
     */
    INT,
  };
  /*! Constructor.
   */
  VertexComponent(const String& name, unsigned int count, Type type = FLOAT);
  /*! Equality operator.
   */
  bool operator == (const VertexComponent& other) const;
  /*! Inequality operator.
   */
  bool operator != (const VertexComponent& other) const;
  /*! @return The name of this component.
   */
  const String& getName(void) const;
  /*! @return The size, in bytes, of this component.
   */
  size_t getSize(void) const;
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
  String name;
  unsigned int count;
  Type type;
  size_t offset;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Vertex format descriptor.
 *  @ingroup opengl
 *
 *  This class describes a mapping between the physical layout and the semantic
 *  structure of a given vertex format.
 *
 *  It allows the renderer to work with vertex buffers of (almost) arbitrary
 *  layout without client intervention.
 */
class VertexFormat
{
public:
  /*! Constructor.
   */
  VertexFormat(void);
  /*! Constructor. Creates components according to the specified specification.
   *  @param specification The specification of the desired format.
   *  @remarks This will throw if the specification is syntactically malformed.
   */
  VertexFormat(const String& specification);
  bool createComponent(const String& name,
                       unsigned int count,
		       VertexComponent::Type type = VertexComponent::FLOAT);
  bool createComponents(const String& specification);
  void destroyComponents(void);
  const VertexComponent* findComponent(const String& name) const;
  const VertexComponent& operator [] (unsigned int index) const;
  bool operator == (const VertexFormat& other) const;
  bool operator != (const VertexFormat& other) const;
  size_t getSize(void) const;
  unsigned int getComponentCount(void) const;
  String getSpecification(void) const;
private:
  typedef std::vector<VertexComponent> ComponentList;
  ComponentList components;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex3fv
{
public:
  Vector3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex3fn3fv
{
public:
  Vector3 normal;
  Vector3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex2fv
{
public:
  Vector2 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex2ft2fv
{
public:
  Vector2 mapping;
  Vector2 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex2ft3fv
{
public:
  Vector2 mapping;
  Vector3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex4fc2ft3fv
{
public:
  ColorRGBA color;
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
