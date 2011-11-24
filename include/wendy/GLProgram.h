///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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
#ifndef WENDY_GLPROGRAM_H
#define WENDY_GLPROGRAM_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Vertex.h>
#include <wendy/Path.h>
#include <wendy/Resource.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

class Context;
class Program;

///////////////////////////////////////////////////////////////////////

/*! @brief Shader container.
 *  @ingroup opengl
 */
class Shader
{
public:
  Shader(const char* text = "",
         const Path& path = Path(),
         unsigned int version = 120);
  String text;
  Path path;
  unsigned int version;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Program vertex attribute.
 *  @ingroup opengl
 */
class Attribute
{
  friend class Program;
  friend class Context;
public:
  enum Type
  {
    FLOAT,
    VEC2,
    VEC3,
    VEC4,
  };
  /*! Binds this attribute to the specified stride and offset of the
   *  current vertex buffer.
   */
  void bind(size_t stride, size_t offset);
  /*! @return @c true if the name of this attribute matches the specified
   *  string, or @c false otherwise.
   */
  bool operator == (const char* string) const;
  /*! @return @c true if the type of this attribute is a single value.
   */
  bool isScalar() const;
  /*! @return @c true if the type of this attribute is a vector.
   */
  bool isVector() const;
  /*! @return The type of this attribute.
   */
  Type getType() const;
  /*! @return The name of this attribute.
   */
  const String& getName() const;
  /*! @return The GLSL name of the specified attribute type.
   */
  static const char* getTypeName(Type type);
private:
  Type type;
  String name;
  int location;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Program sampler uniform.
 *  @ingroup opengl
 */
class Sampler
{
  friend class Program;
public:
  /*! Sampler uniform type enumeration.
   */
  enum Type
  {
    SAMPLER_1D,
    SAMPLER_2D,
    SAMPLER_3D,
    SAMPLER_RECT,
    SAMPLER_CUBE,
  };
  /*! Binds this sampler to the specified texture unit.
   */
  void bind(unsigned int unit);
  /*! @return @c true if the name of this sampler matches the specified string,
   *  or @c false otherwise.
   */
  bool operator == (const char* string) const;
  /*! @return @c true if this sampler is shared, or @c false otherwise.
   *
   *  @remarks Shared samplers get their values via the currently set shared
   *  program state.
   */
  bool isShared() const;
  /*! @return The type of this sampler.
   */
  Type getType() const;
  /*! @return The name of this sampler.
   */
  const String& getName() const;
  /*! @return The shared ID of this sampler, or INVALID_SHARED_STATE_ID if
   *  it is not shared.
   */
  int getSharedID() const;
  /*! @return The GLSL name of the specified sampler type.
   */
  static const char* getTypeName(Type type);
private:
  String name;
  Type type;
  int location;
  int sharedID;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Program non-sampler uniform.
 *  @ingroup opengl
 */
class Uniform
{
  friend class Program;
public:
  /*! Non-sampler uniform type enumeration.
   */
  enum Type
  {
    FLOAT,
    VEC2,
    VEC3,
    VEC4,
    MAT2,
    MAT3,
    MAT4,
  };
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
  bool operator == (const char* string) const;
  /*! @return @c true if this uniform is shared, or @c false otherwise.
   *
   *  @remarks Shared uniforms get their values via the currently set shared
   *  program state.
   */
  bool isShared() const;
  /*! @return @c true if the type of this uniform is a single value.
   */
  bool isScalar() const;
  /*! @return @c true if the type of this uniform is a vector.
   */
  bool isVector() const;
  /*! @return @c true if the type of this uniform is a matrix.
   */
  bool isMatrix() const;
  /*! @return The type of this uniform.
   */
  Type getType() const;
  /*! @return The name of this uniform.
   */
  const String& getName() const;
  /*! @return The number of elements in this uniform.
   */
  unsigned int getElementCount() const;
  /*! @return The shared ID of this uniform, or INVALID_SHARED_STATE_ID if
   *  it is not shared.
   */
  int getSharedID() const;
  /*! @return The GLSL name of the specified uniform type.
   */
  static const char* getTypeName(Type type);
private:
  String name;
  Type type;
  int location;
  int sharedID;
};

///////////////////////////////////////////////////////////////////////

/*! @brief GPU program;
 *  @ingroup opengl
 *
 *  Represents a complete set of GPU programs.
 */
class Program : public Resource
{
  friend class Context;
public:
  ~Program();
  Attribute* findAttribute(const char* name);
  const Attribute* findAttribute(const char* name) const;
  Sampler* findSampler(const char* name);
  const Sampler* findSampler(const char* name) const;
  Uniform* findUniform(const char* name);
  const Uniform* findUniform(const char* name) const;
  bool isCurrent() const;
  bool hasGeometryShader() const;
  bool hasTessellation() const;
  unsigned int getAttributeCount() const;
  Attribute& getAttribute(unsigned int index);
  const Attribute& getAttribute(unsigned int index) const;
  unsigned int getSamplerCount() const;
  Sampler& getSampler(unsigned int index);
  const Sampler& getSampler(unsigned int index) const;
  unsigned int getUniformCount() const;
  Uniform& getUniform(unsigned int index);
  const Uniform& getUniform(unsigned int index) const;
  Context& getContext() const;
  static Ref<Program> create(const ResourceInfo& info,
                             Context& context,
                             const Shader& vertexShader,
                             const Shader& fragmentShader);
  static Ref<Program> create(const ResourceInfo &info,
                             Context &context,
                             const Shader& vertexShader,
                             const Shader& fragmentShader,
                             const Shader& geometryShader);
  static Ref<Program> create(const ResourceInfo &info,
                             Context &context,
                             const Shader& vertexShader,
                             const Shader& fragmentShader,
                             const Shader& tessCtrlShader,
                             const Shader& tessEvalShader);
  static Ref<Program> create(const ResourceInfo &info,
                             Context &context,
                             const Shader& vertexShader,
                             const Shader& fragmentShader,
                             const Shader& geometryShader,
                             const Shader& tessCtrlShader,
                             const Shader& tessEvalShader);
  static Ref<Program> read(Context& context, const Path& path);
private:
  Program(const ResourceInfo& info, Context& context);
  Program(const Program& source);
  bool attachShader(const Shader& shader, unsigned int type);
  bool link();
  bool retrieveUniforms();
  bool retrieveAttributes();
  void bind();
  void unbind();
  Program& operator = (const Program& source);
  bool isValid() const;
  typedef std::vector<Attribute> AttributeList;
  typedef std::vector<Sampler> SamplerList;
  typedef std::vector<Uniform> UniformList;
  Context& context;
  unsigned int vertexShaderID;
  unsigned int fragmentShaderID;
  unsigned int geometryShaderID;
  unsigned int tessCtrlShaderID;
  unsigned int tessEvalShaderID;
  unsigned int programID;
  AttributeList attributes;
  SamplerList samplers;
  UniformList uniforms;
};

///////////////////////////////////////////////////////////////////////

/*! @brief GPU program interface validator.
 *  @ingroup opengl
 */
class ProgramInterface
{
public:
  /*! Adds a sampler to this interface.
   *  @param[in] name The name of the sampler.
   *  @param[in] type The type of the sampler.
   */
  void addSampler(const char* name, Sampler::Type type);
  /*! Adds a sampler to this interface.
   *  @param[in] name The name of the sampler.
   *  @param[in] type The type of the sampler.
   */
  void addUniform(const char* name, Uniform::Type type);
  /*! Adds a sampler to this interface.
   *  @param[in] name The name of the sampler.
   *  @param[in] type The type of the sampler.
   */
  void addAttribute(const char* name, Attribute::Type type);
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
  typedef std::vector<std::pair<String, Sampler::Type> > SamplerList;
  typedef std::vector<std::pair<String, Uniform::Type> > UniformList;
  typedef std::vector<std::pair<String, Attribute::Type> > AttributeList;
  SamplerList samplers;
  UniformList uniforms;
  AttributeList attributes;
};

///////////////////////////////////////////////////////////////////////

/*! @brief GPU program XML codec.
 *  @ingroup opengl
 */
class ProgramReader : public ResourceReader
{
public:
  ProgramReader(Context& context);
  Ref<Program> read(const Path& path);
private:
  Context& context;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLPROGRAM_H*/
///////////////////////////////////////////////////////////////////////
