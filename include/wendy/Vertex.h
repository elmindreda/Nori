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
#ifndef WENDY_VERTEX_H
#define WENDY_VERTEX_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief Vertex format component descriptor.
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
    /*! Component elements are 32-bit floating-point (float).
     */
    FLOAT32,
  };
  /*! Constructor.
   */
  VertexComponent(const char* name, size_t count, Type type);
  /*! Equality operator.
   */
  bool operator == (const VertexComponent& other) const;
  /*! Inequality operator.
   */
  bool operator != (const VertexComponent& other) const;
  /*! @return The name of this component.
   */
  const String& getName() const;
  /*! @return The size, in bytes, of this component.
   */
  size_t getSize() const;
  /*! @return The type of the elements in this component.
   */
  Type getType() const;
  /*! @return The offset, in bytes, of this component in a vertex.
   */
  size_t getOffset() const;
  /*! @return The number of elements in this component.
   */
  size_t getElementCount() const;
private:
  String name;
  size_t count;
  Type type;
  size_t offset;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Vertex format descriptor.
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
  VertexFormat();
  /*! Constructor. Creates components according to the specified specification.
   *  @param specification The specification of the desired format.
   *  @remarks This will throw if the specification is syntactically malformed.
   */
  explicit VertexFormat(const char* specification);
  bool createComponent(const char* name,
                       size_t count,
                       VertexComponent::Type type);
  bool createComponents(const char* specification);
  void destroyComponents();
  const VertexComponent* findComponent(const char* name) const;
  const VertexComponent& operator [] (size_t index) const;
  bool operator == (const VertexFormat& other) const;
  bool operator != (const VertexFormat& other) const;
  String asString() const;
  size_t getSize() const;
  size_t getComponentCount() const;
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
  vec3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex3fn3fv
{
public:
  vec3 normal;
  vec3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex2fv
{
public:
  vec2 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex2ft2fv
{
public:
  vec2 texCoord;
  vec2 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex2ft3fv
{
public:
  vec2 texCoord;
  vec3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex4fc2ft3fv
{
public:
  vec4 color;
  vec2 texCoord;
  vec3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Predefined vertex format.
 */
class Vertex3fn2ft3fv
{
public:
  vec3 normal;
  vec2 texCoord;
  vec3 position;
  static const VertexFormat format;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_VERTEX_H*/
///////////////////////////////////////////////////////////////////////
