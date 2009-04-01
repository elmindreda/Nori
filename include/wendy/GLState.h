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
#ifndef WENDY_GLSTATE_H
#define WENDY_GLSTATE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! Stencil operation enumeration.
 *  @ingroup opengl
 */
enum Operation
{
  OP_KEEP,
  OP_ZERO,
  OP_REPLACE,
  OP_INCREASE,
  OP_DECREASE,
  OP_INVERT,
  OP_INCREASE_WRAP,
  OP_DECREASE_WRAP
};

///////////////////////////////////////////////////////////////////////

/*! @brief Stencil buffer state.
 *  @ingroup opengl
 */
class StencilState
{
public:
  void apply(void) const;
  bool isEnabled(void) const;
  Function getFunction(void) const;
  Operation getStencilFailOperation(void) const;
  Operation getDepthFailOperation(void) const;
  Operation getDepthPassOperation(void) const;
  unsigned int getReference(void) const;
  unsigned int getWriteMask(void) const;
  void setEnabled(bool newState);
  void setFunction(Function newFunction);
  void setReference(unsigned int newReference);
  void setWriteMask(unsigned int newMask);
  void setOperations(Operation stencilFailed,
                     Operation depthFailed,
                     Operation depthPassed);
  void setDefaults(void);
private:
  class Data
  {
  public:
    Data(void);
    void setDefaults(void);
    mutable bool dirty;
    bool enabled;
    Function function;
    unsigned int reference;
    unsigned int writeMask;
    Operation stencilFailed;
    Operation depthFailed;
    Operation depthPassed;
  };
  void force(void) const;
  Data data;
  static Data cache;
};

///////////////////////////////////////////////////////////////////////

/*! @brief State for a single shader uniform.
 *  @ingroup opengl
 */
class UniformState
{
  friend class ProgramState;
public:
  virtual bool getValue(float& result) const;
  virtual void setValue(const float newValue);
  virtual bool getValue(Vector2& result) const;
  virtual void setValue(const Vector2& newValue);
  virtual bool getValue(Vector3& result) const;
  virtual void setValue(const Vector3& newValue);
  virtual bool getValue(Vector4& result) const;
  virtual void setValue(const Vector4& newValue);
  virtual bool getValue(Matrix2& result) const;
  virtual void setValue(const Matrix2& newValue);
  virtual bool getValue(Matrix3& result) const;
  virtual void setValue(const Matrix3& newValue);
  virtual bool getValue(Matrix4& result) const;
  virtual void setValue(const Matrix4& newValue);
  Uniform& getUniform(void) const;
protected:
  UniformState(Uniform& uniform);
  virtual void apply(void) const;
private:
  UniformState(const UniformState& source);
  UniformState& operator = (const UniformState& source);
  Uniform& uniform;
};

///////////////////////////////////////////////////////////////////////

/*! @brief State for a single shader sampler uniform.
 *  @ingroup opengl
 */
class SamplerState
{
  friend class ProgramState;
public:
  bool getTexture(Ref<Texture>& result) const;
  void setTexture(Texture* newTexture);
  Sampler& getSampler(void) const;
protected:
  SamplerState(Sampler& sampler);
  void apply(void) const;
private:
  SamplerState(const SamplerState& source);
  SamplerState& operator = (const SamplerState& source);
  Sampler& sampler;
  Ref<Texture> texture;
};

///////////////////////////////////////////////////////////////////////

class ProgramState
{
public:
  ~ProgramState(void);
  void apply(void) const;
  unsigned int getUniformCount(void) const;
  UniformState& getUniformState(const String& name);
  const UniformState& getUniformState(const String& name) const;
  UniformState& getUniformState(unsigned int index);
  const UniformState& getUniformState(unsigned int index) const;
  unsigned int getSamplerCount(void) const;
  SamplerState& getSamplerState(const String& name);
  const SamplerState& getSamplerState(const String& name) const;
  SamplerState& getSamplerState(unsigned int index);
  const SamplerState& getSamplerState(unsigned int index) const;
  /*! @return The shader program used by this render pass.
   */
  Program* getProgram(void) const;
  /*! Sets the shader program used by this render pass.
   *  @param[in] newProgram The desired shader program, or @c NULL to use the
   *  default shader program.
   */
  void setProgram(Program* newProgram);
private:
  void destroyProgramState(void);
  typedef std::vector<UniformState*> UniformList;
  typedef std::vector<SamplerState*> SamplerList;
  Ref<Program> program;
  UniformList uniforms;
  SamplerList samplers;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLSTATE_H*/
///////////////////////////////////////////////////////////////////////
