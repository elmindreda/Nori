///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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

#pragma once

#include <wendy/Core.hpp>
#include <wendy/Vertex.hpp>
#include <wendy/Path.hpp>
#include <wendy/Resource.hpp>

namespace wendy
{

class RenderContext;
class Program;

/*! @brief Shader type enumeration.
 */
enum ShaderType
{
  VERTEX_SHADER,
  FRAGMENT_SHADER
};

/*! @brief %Shader.
 */
class Shader : public Resource, public RefObject
{
  friend class Program;
public:
  ~Shader();
  bool isVertexShader() const { return m_type == VERTEX_SHADER; }
  bool isFragmentShader() const { return m_type == FRAGMENT_SHADER; }
  ShaderType type() const { return m_type; }
  RenderContext& context() const { return m_context; }
  static Ref<Shader> create(const ResourceInfo& info,
                            RenderContext& context,
                            ShaderType type,
                            const std::string& text);
  static Ref<Shader> read(RenderContext& context,
                          ShaderType type,
                          const std::string& name);
private:
  Shader(const ResourceInfo& info, RenderContext& context, ShaderType type);
  bool init(const std::string& text);
  RenderContext& m_context;
  ShaderType m_type;
  uint m_shaderID;
};

/*! @brief Program attribute type enumeration.
 */
enum AttributeType
{
  ATTRIBUTE_FLOAT,
  ATTRIBUTE_VEC2,
  ATTRIBUTE_VEC3,
  ATTRIBUTE_VEC4
};

/*! @brief Program vertex attribute.
 */
class Attribute
{
  friend class Program;
  friend class RenderContext;
public:
  /*! Binds this attribute to the specified stride and offset of the
   *  current vertex buffer.
   */
  void bind(size_t stride, size_t offset);
  /*! @return @c true if the name of this attribute matches the specified
   *  string, or @c false otherwise.
   */
  bool operator == (const char* string) const { return m_name == string; }
  /*! @return @c true if the type of this attribute is a single value.
   */
  bool isScalar() const;
  /*! @return @c true if the type of this attribute is a vector.
   */
  bool isVector() const;
  /*! @return The type of this attribute.
   */
  AttributeType type() const { return m_type; }
  /*! @return The name of this attribute.
   */
  const std::string& name() const { return m_name; }
  /*! @return The number of elements in this attribute.
   */
  uint elementCount() const;
private:
  AttributeType m_type;
  std::string m_name;
  int m_location;
};

/*! @brief Uniform type enumeration.
 */
enum UniformType
{
  UNIFORM_SAMPLER_1D,
  UNIFORM_SAMPLER_2D,
  UNIFORM_SAMPLER_3D,
  UNIFORM_SAMPLER_RECT,
  UNIFORM_SAMPLER_CUBE,
  UNIFORM_INT,
  UNIFORM_UINT,
  UNIFORM_FLOAT,
  UNIFORM_VEC2,
  UNIFORM_VEC3,
  UNIFORM_VEC4,
  UNIFORM_MAT2,
  UNIFORM_MAT3,
  UNIFORM_MAT4
};

/*! @brief Program uniform.
 */
class Uniform
{
  friend class Program;
public:
  /*! Copies a new value for this uniform from the specified address.
   *  @param[in] data The address of the value to use.
   *
   *  @remarks It is the responsibility of the caller to ensure that the source
   *  data type matches.
   */
  void copyFrom(const void* data);
  /*! @return @c true if the name of this uniform matches the specified string,
   *  or @c false otherwise.
   */
  bool operator == (const char* string) const { return m_name == string; }
  /*! @return @c true if this uniform is shared, or @c false otherwise.
   *
   *  @remarks Shared uniforms get their values via the currently set shared
   *  program state.
   */
  bool isShared() const { return m_sharedID != -1; }
  /*! @return @c true if the type of this uniform is a single value.
   */
  bool isScalar() const;
  /*! @return @c true if the type of this uniform is a vector.
   */
  bool isVector() const;
  /*! @return @c true if the type of this uniform is a matrix.
   */
  bool isMatrix() const;
  /*! @return @c true if the type of this uniform is a sampler.
   */
  bool isSampler() const;
  /*! @return The type of this uniform.
   */
  UniformType type() const { return m_type; }
  /*! @return The name of this uniform.
   */
  const std::string& name() const { return m_name; }
  /*! @return The number of elements in this uniform.
   */
  uint elementCount() const;
  /*! @return The shared ID of this uniform, or -1 if it is not shared.
   */
  int sharedID() const { return m_sharedID; }
private:
  std::string m_name;
  UniformType m_type;
  int m_location;
  int m_sharedID;
};

const char* stringCast(AttributeType type);
const char* stringCast(UniformType type);

/*! @brief %Shader program.
 */
class Program : public Resource, public RefObject
{
  friend class Pass;
  friend class RenderContext;
public:
  ~Program();
  Attribute* findAttribute(const char* name);
  const Attribute* findAttribute(const char* name) const;
  Uniform* findUniform(const char* name);
  const Uniform* findUniform(const char* name) const;
  uint attributeCount() const;
  Attribute& attribute(uint index);
  const Attribute& attribute(uint index) const;
  uint uniformCount() const;
  Uniform& uniform(uint index);
  const Uniform& uniform(uint index) const;
  RenderContext& context() const;
  static Ref<Program> create(const ResourceInfo& info,
                             RenderContext& context,
                             Shader& vertexShader,
                             Shader& fragmentShader);
  static Ref<Program> read(RenderContext& context,
                           const std::string& vertexShaderName,
                           const std::string& fragmentShaderName);
private:
  Program(const ResourceInfo& info, RenderContext& context);
  Program(const Program&) = delete;
  bool init(Shader& vertexShader, Shader& fragmentShader);
  bool retrieveUniforms();
  bool retrieveAttributes();
  void bind();
  void unbind();
  Program& operator = (const Program&) = delete;
  bool isValid() const;
  std::string infoLog() const;
  RenderContext& m_context;
  Ref<Shader> m_vertexShader;
  Ref<Shader> m_fragmentShader;
  uint m_programID;
  std::vector<Attribute> m_attributes;
  std::vector<Uniform> m_uniforms;
};

/*! @brief Program interface validator.
 */
class ProgramInterface
{
public:
  /*! Adds a uniform to this interface.
   *  @param[in] name The name of the uniform.
   *  @param[in] type The type of the uniform.
   */
  void addUniform(const char* name, UniformType type);
  /*! Adds an attribute to this interface.
   *  @param[in] name The name of the attribute.
   *  @param[in] type The type of the attribute.
   */
  void addAttribute(const char* name, AttributeType type);
  /*! Adds attributes for all components of the specified vertex format.
   *  @param[in] format The vertex format to use.
   */
  void addAttributes(const VertexFormat& format);
  /*! Checks whether all uniforms and attributes of this interface
   *  are exposed by the specified program and are of the correct types.
   *  @param[in] program The program to match this interface against.
   *  @param[in] verbose @c true to %log errors and warnings, or @c false for
   *  silent matching.
   *  @return @c true if this entire interface is exposed by the specified
   *  program, or @c false otherwise.
   */
  bool matches(const Program& program, bool verbose = false) const;
  /*! Checks whether components matching all attributes of this interface are
   *  present in the specified vertex format and are of the correct types.
   *  @param[in] format The vertex format to match this interface against.
   *  @param[in] verbose @c true to %log errors and warnings, or @c false for
   *  silent matching.
   *  @return @c true if all attributes of this interface are exposed by the
   *  specified vertex format, or @c false otherwise.
   */
  bool matches(const VertexFormat& format, bool verbose = false) const;
private:
  std::vector<std::pair<std::string, UniformType>> uniforms;
  std::vector<std::pair<std::string, AttributeType>> attributes;
};

} /*namespace wendy*/

