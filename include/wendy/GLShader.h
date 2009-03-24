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
#ifndef WENDY_GLSHADER_H
#define WENDY_GLSHADER_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Context;
class Program;

///////////////////////////////////////////////////////////////////////

/*! @brief Shader uniform.
 *  @ingroup opengl
 */
class Uniform
{
  friend class Program;
public:
  enum Type
  {
    FLOAT,
    FLOAT_VEC2,
    FLOAT_VEC3,
    FLOAT_VEC4,
    FLOAT_MAT2,
    FLOAT_MAT3,
    FLOAT_MAT4,
  };
  bool isScalar(void) const;
  bool isVector(void) const;
  bool isMatrix(void) const;
  Type getType(void) const;
  const String& getName(void) const;
  void setValue(float newValue);
  void setValue(const Vector2& newValue);
  void setValue(const Vector3& newValue);
  void setValue(const Vector4& newValue);
  void setValue(const Matrix2& newValue);
  void setValue(const Matrix3& newValue);
  void setValue(const Matrix4& newValue);
  Program& getProgram(void) const;
private:
  Uniform(Program& program);
  Uniform(const Uniform& source);
  Uniform& operator = (const Uniform& source);
  Program& program;
  String name;
  Type type;
  void* uniformID;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Shader sampler uniform.
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
  Type getType(void) const;
  const String& getName(void) const;
  void setTexture(const Texture& newTexture);
  Program& getProgram(void) const;
private:
  Sampler(Program& program);
  Sampler(const Sampler& source);
  Sampler& operator = (const Sampler& source);
  Program& program;
  String name;
  Type type;
  void* samplerID;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Vertex shader.
 *  @ingroup opengl
 */
class VertexShader : public RefObject<VertexShader>,
                     public Resource<VertexShader>
{
  friend class Program;
public:
  ~VertexShader(void);
  const String& getText(void) const;
  static VertexShader* createInstance(Context& context,
                                      const String& text,
                                      const String& name = "");
private:
  VertexShader(Context& context, const String& name);
  VertexShader(const VertexShader& source);
  bool init(const String& initText);
  VertexShader& operator = (const VertexShader& source);
  Context& context;
  void* shaderID;
  String text;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Fragment shader.
 *  @ingroup opengl
 */
class FragmentShader : public RefObject<FragmentShader>,
                       public Resource<FragmentShader>
{
  friend class Program;
public:
  ~FragmentShader(void);
  const String& getText(void) const;
  static FragmentShader* createInstance(Context& context,
                                        const String& text,
                                        const String& name = "");
private:
  FragmentShader(Context& context, const String& name);
  FragmentShader(const FragmentShader& source);
  bool init(const String& initText);
  FragmentShader& operator = (const FragmentShader& source);
  Context& context;
  void* shaderID;
  String text;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Shader program.
 *  @ingroup opengl
 */
class Program : public RefObject<Program>, public Resource<Program>
{
public:
  void apply(void);
  Uniform* findUniform(const String& name);
  const Uniform* findUniform(const String& name) const;
  Sampler* findSampler(const String& name);
  const Sampler* findSampler(const String& name) const;
  unsigned int getUniformCount(void) const;
  Uniform& getUniform(unsigned int index);
  const Uniform& getUniform(unsigned int index) const;
  unsigned int getSamplerCount(void) const;
  Sampler& getSampler(unsigned int index);
  const Sampler& getSampler(unsigned int index) const;
  VertexShader& getVertexShader(void) const;
  FragmentShader& getFragmentShader(void) const;
  static Program* createInstance(Context& context,
                                 VertexShader& vertexShader,
                                 FragmentShader& fragmentShader,
				 const String& name = "");
private:
  Program(Context& context, const String& name);
  Program(const Program& source);
  bool init(VertexShader& vertexShader, FragmentShader& fragmentShader);
  Program& operator = (const Program& source);
  typedef std::vector<Uniform*> UniformList;
  typedef std::vector<Sampler*> SamplerList;
  Context& context;
  Ref<VertexShader> vertexShader;
  Ref<FragmentShader> fragmentShader;
  void* programID;
  UniformList uniforms;
  SamplerList samplers;
};

///////////////////////////////////////////////////////////////////////

/*
class ProgramState
{
public:
  ProgramState(void);
  ProgramState(Program& program);
  void apply(void) const;
  unsigned int getUniformStateCount(void) const;
  UniformState& getUniformState(unsigned int index);
  const UniformState& getUniformState(unsigned int index) const;
  UniformState& getUniformState(const String& name);
  const UniformState& getUniformState(const String& name) const;
  Program* getProgram(void) const;
private:
  typedef std::vector<UniformState*> StateList;
  Ref<Program> program;
  StateList states;
};
*/

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLSHADER_H*/
///////////////////////////////////////////////////////////////////////
