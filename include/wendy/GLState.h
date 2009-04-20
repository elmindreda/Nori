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

/*! @brief Cull mode enumeration.
 *  @ingroup opengl
 */
enum CullMode
{
  /*! Do not cull any geometry.
   */
  CULL_NONE,
  /*! Cull front-facing geometry (i.e. render back-facing geometry).
   */
  CULL_FRONT,
  /*! Cull back-facing geometry (i.e. render front-facing geometry).
   */
  CULL_BACK,
  /*! Cull all cullable geometry (i.e. front and back faces).
   */
  CULL_BOTH,
};

///////////////////////////////////////////////////////////////////////

/*! Blend factor enumeration.
 *  @ingroup opengl
 */
enum BlendFactor
{
  BLEND_ZERO,
  BLEND_ONE,
  BLEND_SRC_COLOR,
  BLEND_DST_COLOR,
  BLEND_SRC_ALPHA,
  BLEND_DST_ALPHA,
  BLEND_ONE_MINUS_SRC_COLOR,
  BLEND_ONE_MINUS_DST_COLOR,
  BLEND_ONE_MINUS_SRC_ALPHA,
  BLEND_ONE_MINUS_DST_ALPHA,
};

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

class UniformData
{
public:
  UniformData(Uniform::Type type);
  Uniform::Type getType(void) const;
  bool getValue(float& result) const;
  void setValue(const float newValue);
  bool getValue(Vec2& result) const;
  void setValue(const Vec2& newValue);
  bool getValue(Vec3& result) const;
  void setValue(const Vec3& newValue);
  bool getValue(Vec4& result) const;
  void setValue(const Vec4& newValue);
  bool getValue(Mat2& result) const;
  void setValue(const Mat2& newValue);
  bool getValue(Mat3& result) const;
  void setValue(const Mat3& newValue);
  bool getValue(Mat4& result) const;
  void setValue(const Mat4& newValue);
private:
  Uniform::Type type;
  float data[16];
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
  virtual bool getValue(Vec2& result) const;
  virtual void setValue(const Vec2& newValue);
  virtual bool getValue(Vec3& result) const;
  virtual void setValue(const Vec3& newValue);
  virtual bool getValue(Vec4& result) const;
  virtual void setValue(const Vec4& newValue);
  virtual bool getValue(Mat2& result) const;
  virtual void setValue(const Mat2& newValue);
  virtual bool getValue(Mat3& result) const;
  virtual void setValue(const Mat3& newValue);
  virtual bool getValue(Mat4& result) const;
  virtual void setValue(const Mat4& newValue);
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
  ProgramState(void);
  ProgramState(const ProgramState& source);
  ~ProgramState(void);
  void apply(void) const;
  ProgramState& operator = (const ProgramState& source);
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
  void setDefaults(void);
private:
  void destroyProgramState(void);
  typedef std::vector<UniformState*> UniformList;
  typedef std::vector<SamplerState*> SamplerList;
  Ref<Program> program;
  UniformList uniforms;
  SamplerList samplers;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Render state object.
 *  @ingroup opengl
 *
 *  This class and its associated classes encapsulates most of the OpenGL
 *  rendering state, notable exceptions being the transformation and stencil
 *  buffer state.
 *
 *  @remarks Unless you're writing your own custom renderer, you will probably
 *  want to use the classes in the render namespace, and not use these classes
 *  independently.
 *
 *  @remarks Yes, it's big.
 */
class RenderState : public ProgramState
{
public:
  void apply(void) const;
  /*! @return @c true if this render pass uses any form of culling, otherwise
   *  @c false.
   */
  bool isCulling(void) const;
  /*! @return @c true if this render pass uses any form of blending with the
   *  framebuffer, otherwise @c false.
   */
  bool isBlending(void) const;
  /*! @return @c true if this render pass uses depth buffer testing, otherwise
   *  @c false.
   */
  bool isDepthTesting(void) const;
  /*! @return @c true if this render pass writes to the depth buffer, otherwise
   *  @c false.
   */
  bool isDepthWriting(void) const;
  /*! @return @c true if this render pass writes to the color buffer, otherwise
   *  @c false.
   */
  bool isColorWriting(void) const;
  bool isWireframe(void) const;
  /*! @return The culling mode of this render pass.
   */
  CullMode getCullMode(void) const;
  BlendFactor getSrcFactor(void) const;
  BlendFactor getDstFactor(void) const;
  Function getDepthFunction(void) const;
  /*! Sets whether this render pass uses depth buffer testing.
   *  @param enable Set to @c true to enable depth buffer testing, or @c false
   *  to disable it.
   */
  void setDepthTesting(bool enable);
  /*! Sets whether this render pass writes to the depth buffer.
   *  @param enable Set to @c true to enable depth buffer writing, or @c false
   *  to disable it.
   */
  void setDepthWriting(bool enable);
  /*! Sets the depth buffer testing function for this render pass.
   *  @param function The desired depth testing function.
   */
  void setDepthFunction(Function function);
  /*! Sets whether writing to the color buffer is enabled.
   *  @param enabled @c true to enable writing to the color buffer, or @c false
   *  to disable it.
   */
  void setColorWriting(bool enabled);
  void setWireframe(bool enabled);
  void setCullMode(CullMode mode);
  void setBlendFactors(BlendFactor src, BlendFactor dst);
  /*! Resets all values in this render pass to their defaults.
   */
  void setDefaults(void);
  static bool isCullingInverted(void);
  static void setCullingInversion(bool newState);
private:
  class Data
  {
  public:
    Data(void);
    void setDefaults(void);
    mutable bool dirty;
    bool depthTesting;
    bool depthWriting;
    bool colorWriting;
    bool wireframe;
    CullMode cullMode;
    BlendFactor srcFactor;
    BlendFactor dstFactor;
    Function depthFunction;
  };
  void force(void) const;
  void setBooleanState(unsigned int state, bool value) const;
  Data data;
  static Data cache;
  static bool cullingInverted;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLSTATE_H*/
///////////////////////////////////////////////////////////////////////
