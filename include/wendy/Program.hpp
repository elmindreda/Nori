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
#ifndef WENDY_PROGRAM_HPP
#define WENDY_PROGRAM_HPP
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.hpp>
#include <wendy/Vertex.hpp>
#include <wendy/Path.hpp>
#include <wendy/Resource.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class RenderContext;
class Program;

///////////////////////////////////////////////////////////////////////

/*! @brief Shader type enumeration.
 */
enum ShaderType
{
  VERTEX_SHADER,
  FRAGMENT_SHADER
};

///////////////////////////////////////////////////////////////////////

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
                            const String& text);
  static Ref<Shader> read(RenderContext& context,
                          ShaderType type,
                          const String& name);
private:
  Shader(const ResourceInfo& info, RenderContext& context, ShaderType type);
  bool init(const String& text);
  RenderContext& m_context;
  ShaderType m_type;
  uint m_shaderID;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Program attribute type enumeration.
 */
enum AttributeType
{
  ATTRIBUTE_FLOAT,
  ATTRIBUTE_VEC2,
  ATTRIBUTE_VEC3,
  ATTRIBUTE_VEC4
};

///////////////////////////////////////////////////////////////////////

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
  bool isScalar() const { return m_type == ATTRIBUTE_FLOAT; }
  /*! @return @c true if the type of this attribute is a vector.
   */
  bool isVector() const;
  /*! @return The type of this attribute.
   */
  AttributeType type() const { return m_type; }
  /*! @return The name of this attribute.
   */
  const String& name() const { return m_name; }
  /*! @return The number of elements in this attribute.
   */
  uint elementCount() const;
  /*! @return The GLSL name of the specified attribute type.
   */
  static const char* typeName(AttributeType type);
private:
  AttributeType m_type;
  String m_name;
  int m_location;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Sampler uniform type enumeration.
 */
enum SamplerType
{
  SAMPLER_1D,
  SAMPLER_2D,
  SAMPLER_3D,
  SAMPLER_RECT,
  SAMPLER_CUBE
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Sampler uniform.
 */
class Sampler
{
  friend class Program;
public:
  /*! @return @c true if the name of this sampler matches the specified string,
   *  or @c false otherwise.
   */
  bool operator == (const char* string) const { return m_name == string; }
  /*! @return @c true if this sampler is shared, or @c false otherwise.
   *
   *  @remarks Shared samplers get their values via the currently set shared
   *  program state.
   */
  bool isShared() const { return m_sharedID != -1; }
  /*! @return The type of this sampler.
   */
  SamplerType type() const { return m_type; }
  /*! @return The name of this sampler.
   */
  const String& name() const { return m_name; }
  /*! @return The shared ID of this sampler, or -1 if it is not shared.
   */
  int sharedID() const { return m_sharedID; }
  /*! @return The GLSL name of the specified sampler type.
   */
  static const char* typeName(SamplerType type);
private:
  String m_name;
  SamplerType m_type;
  int m_location;
  int m_sharedID;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Non-sampler uniform type enumeration.
 */
enum UniformType
{
  UNIFORM_FLOAT,
  UNIFORM_VEC2,
  UNIFORM_VEC3,
  UNIFORM_VEC4,
  UNIFORM_MAT2,
  UNIFORM_MAT3,
  UNIFORM_MAT4
};

///////////////////////////////////////////////////////////////////////

/*! @brief Program non-sampler uniform.
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
  bool isScalar() const { return m_type == UNIFORM_FLOAT; }
  /*! @return @c true if the type of this uniform is a vector.
   */
  bool isVector() const;
  /*! @return @c true if the type of this uniform is a matrix.
   */
  bool isMatrix() const;
  /*! @return The type of this uniform.
   */
  UniformType type() const { return m_type; }
  /*! @return The name of this uniform.
   */
  const String& name() const { return m_name; }
  /*! @return The number of elements in this uniform.
   */
  uint elementCount() const;
  /*! @return The shared ID of this uniform, or -1 if it is not shared.
   */
  int sharedID() const { return m_sharedID; }
  /*! @return The GLSL name of the specified uniform type.
   */
  static const char* typeName(UniformType type);
private:
  String m_name;
  UniformType m_type;
  int m_location;
  int m_sharedID;
};

///////////////////////////////////////////////////////////////////////

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
  Sampler* findSampler(const char* name);
  const Sampler* findSampler(const char* name) const;
  Uniform* findUniform(const char* name);
  const Uniform* findUniform(const char* name) const;
  uint attributeCount() const;
  Attribute& attribute(uint index);
  const Attribute& attribute(uint index) const;
  uint samplerCount() const;
  Sampler& sampler(uint index);
  const Sampler& sampler(uint index) const;
  uint uniformCount() const;
  Uniform& uniform(uint index);
  const Uniform& uniform(uint index) const;
  RenderContext& context() const;
  static Ref<Program> create(const ResourceInfo& info,
                             RenderContext& context,
                             Shader& vertexShader,
                             Shader& fragmentShader);
  static Ref<Program> read(RenderContext& context,
                           const String& vertexShaderName,
                           const String& fragmentShaderName);
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
  String infoLog() const;
  RenderContext& m_context;
  Ref<Shader> m_vertexShader;
  Ref<Shader> m_fragmentShader;
  uint m_programID;
  std::vector<Attribute> m_attributes;
  std::vector<Sampler> m_samplers;
  std::vector<Uniform> m_uniforms;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Program interface validator.
 */
class ProgramInterface
{
public:
  /*! Adds a sampler to this interface.
   *  @param[in] name The name of the sampler.
   *  @param[in] type The type of the sampler.
   */
  void addSampler(const char* name, SamplerType type);
  /*! Adds a sampler to this interface.
   *  @param[in] name The name of the sampler.
   *  @param[in] type The type of the sampler.
   */
  void addUniform(const char* name, UniformType type);
  /*! Adds a sampler to this interface.
   *  @param[in] name The name of the sampler.
   *  @param[in] type The type of the sampler.
   */
  void addAttribute(const char* name, AttributeType type);
  /*! Adds attributes for all components of the specified vertex format.
   *  @param[in] format The vertex format to use.
   */
  void addAttributes(const VertexFormat& format);
  /*! Checks whether all samplers, uniforms and attributes of this interface
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
  std::vector<std::pair<String, SamplerType>> samplers;
  std::vector<std::pair<String, UniformType>> uniforms;
  std::vector<std::pair<String, AttributeType>> attributes;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_PROGRAM_HPP*/
///////////////////////////////////////////////////////////////////////
