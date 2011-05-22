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
#include <wendy/XML.h>

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
  Shader(void);
  Shader(const String& text, const Path& path = Path());
  String text;
  Path path;
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
  void bind(size_t stride, size_t offset);
  bool operator == (const String& string) const;
  bool isScalar(void) const;
  bool isVector(void) const;
  Type getType(void) const;
  const String& getName(void) const;
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
  enum Type
  {
    SAMPLER_1D,
    SAMPLER_2D,
    SAMPLER_3D,
    SAMPLER_RECT,
    SAMPLER_CUBE,
  };
  void bind(unsigned int unit);
  bool operator == (const String& string) const;
  bool isShared(void) const;
  Type getType(void) const;
  const String& getName(void) const;
  int getSharedID(void) const;
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
  void copyFrom(const void* data);
  bool operator == (const String& string) const;
  bool isShared(void) const;
  bool isScalar(void) const;
  bool isVector(void) const;
  bool isMatrix(void) const;
  Type getType(void) const;
  const String& getName(void) const;
  unsigned int getElementCount(void) const;
  int getSharedID(void) const;
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
  ~Program(void);
  Attribute* findAttribute(const String& name);
  const Attribute* findAttribute(const String& name) const;
  Sampler* findSampler(const String& name);
  const Sampler* findSampler(const String& name) const;
  Uniform* findUniform(const String& name);
  const Uniform* findUniform(const String& name) const;
  bool isCurrent(void) const;
  unsigned int getAttributeCount(void) const;
  Attribute& getAttribute(unsigned int index);
  const Attribute& getAttribute(unsigned int index) const;
  unsigned int getSamplerCount(void) const;
  Sampler& getSampler(unsigned int index);
  const Sampler& getSampler(unsigned int index) const;
  unsigned int getUniformCount(void) const;
  Uniform& getUniform(unsigned int index);
  const Uniform& getUniform(unsigned int index) const;
  Context& getContext(void) const;
  static Ref<Program> create(const ResourceInfo& info,
                             Context& context,
                             const Shader& vertexShader,
                             const Shader& fragmentShader);
  static Ref<Program> read(Context& context, const Path& path);
private:
  Program(const ResourceInfo& info, Context& context);
  Program(const Program& source);
  bool init(const Shader& vertexShader, const Shader& fragmentShader);
  bool retrieveUniforms(void);
  bool retrieveAttributes(void);
  void bind(void);
  void unbind(void);
  Program& operator = (const Program& source);
  bool isValid(void) const;
  typedef std::vector<Attribute> AttributeList;
  typedef std::vector<Sampler> SamplerList;
  typedef std::vector<Uniform> UniformList;
  Context& context;
  unsigned int vertexShaderID;
  unsigned int fragmentShaderID;
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
  void addSampler(const String& name, Sampler::Type type);
  void addUniform(const String& name, Uniform::Type type);
  void addAttribute(const String& name, Attribute::Type type);
  void addAttributes(const VertexFormat& format);
  bool matches(const Program& program, bool verbose = false) const;
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
 *  @ingroup io
 */
class ProgramReader : public ResourceReader, public XML::Reader
{
public:
  ProgramReader(Context& context);
  Ref<Program> read(const Path& path);
private:
  bool onBeginElement(const String& name);
  bool onEndElement(const String& name);
  Context& context;
  Ref<Program> program;
  ResourceInfo info;
  Ptr<Shader> vertexShader;
  Ptr<Shader> fragmentShader;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLPROGRAM_H*/
///////////////////////////////////////////////////////////////////////
