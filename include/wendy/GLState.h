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

#include <wendy/Core.h>
#include <wendy/Vector.h>
#include <wendy/Color.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

class GlobalUniform;
class GlobalSampler;

///////////////////////////////////////////////////////////////////////

typedef uint16_t StateID;

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

/*! Comparison function enumeration.
 *  @ingroup opengl
 */
enum Function
{
  ALLOW_NEVER,
  ALLOW_ALWAYS,
  ALLOW_EQUAL,
  ALLOW_NOT_EQUAL,
  ALLOW_LESSER,
  ALLOW_LESSER_EQUAL,
  ALLOW_GREATER,
  ALLOW_GREATER_EQUAL,
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
public:
  UniformState(Uniform& uniform);
  void apply(void) const;
  void getValue(float& result) const;
  void setValue(const float newValue);
  void getValue(Vec2& result) const;
  void setValue(const Vec2& newValue);
  void getValue(Vec3& result) const;
  void setValue(const Vec3& newValue);
  void getValue(Vec4& result) const;
  void setValue(const Vec4& newValue);
  void getValue(ColorRGB& result) const;
  void setValue(const ColorRGB& newValue);
  void getValue(ColorRGBA& result) const;
  void setValue(const ColorRGBA& newValue);
  void getValue(Mat2& result) const;
  void setValue(const Mat2& newValue);
  void getValue(Mat3& result) const;
  void setValue(const Mat3& newValue);
  void getValue(Mat4& result) const;
  void setValue(const Mat4& newValue);
  Uniform& getUniform(void) const;
private:
  Uniform* uniform;
  float data[16];
};

///////////////////////////////////////////////////////////////////////

/*! @brief Binding between a single shader uniform and its global state.
 *  @ingroup opengl
 */
class GlobalUniformState
{
public:
  GlobalUniformState(GlobalUniform& global, Uniform& uniform);
  void apply(void) const;
private:
  GlobalUniform* global;
  Uniform* uniform;
};

///////////////////////////////////////////////////////////////////////

/*! @brief State for a single shader sampler uniform.
 *  @ingroup opengl
 */
class SamplerState
{
public:
  SamplerState(Sampler& sampler);
  void apply(void) const;
  Texture* getTexture(void) const;
  void setTexture(Texture* newTexture);
  Sampler& getSampler(void) const;
private:
  Sampler* sampler;
  Ref<Texture> texture;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Binding between a single shader sampler and its global state.
 *  @ingroup opengl
 */
class GlobalSamplerState
{
public:
  GlobalSamplerState(GlobalSampler& global, Sampler& sampler);
  void apply(void) const;
private:
  GlobalSampler* global;
  Sampler* sampler;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Shader program state.
 *  @ingroup opengl
 */
class ProgramState
{
public:
  /*! Constructor.
   */
  ProgramState(void);
  /*! Destructor.
   */
  ~ProgramState(void);
  /*! Applies this shader program state to the current context.
   */
  void apply(void) const;
  /*! @return The number of non-sampler uniforms exposed by this state object.
   */
  unsigned int getUniformCount(void) const;
  /*! @return The state object corresponding to the non-sampler uniform with
   *  the specified name.
   *  @param[in] name The name of the desired uniform.
   */
  UniformState& getUniformState(const String& name);
  /*! @return The state object corresponding to the non-sampler uniform with
   *  the specified name.
   *  @param[in] name The name of the desired uniform.
   */
  const UniformState& getUniformState(const String& name) const;
  /*! @return The non-sampler uniform state object at the specified index.
   *  @param[in] index The index of the desired non-sampler uniform state
   *  object.
   */
  UniformState& getUniformState(unsigned int index);
  /*! @return The non-sampler uniform state object at the specified index.
   *  @param[in] index The index of the desired non-sampler uniform state
   *  object.
   */
  const UniformState& getUniformState(unsigned int index) const;
  /*! @return The number of sampler uniforms exposed by this state object.
   */
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
  StateID getID(void) const;
  void setDefaults(void);
private:
  void destroyProgramState(void);
  typedef std::vector<UniformState> UniformList;
  typedef std::vector<SamplerState> SamplerList;
  typedef std::vector<GlobalUniformState> GlobalUniformList;
  typedef std::vector<GlobalSamplerState> GlobalSamplerList;
  typedef std::deque<StateID> IDQueue;
  Ref<Program> program;
  UniformList uniforms;
  SamplerList samplers;
  GlobalUniformList globalUniforms;
  GlobalSamplerList globalSamplers;
  StateID ID;
  static IDQueue usedIDs;
  static StateID nextID;
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
  /*! Applies this render state to the current context.
   */
  void apply(void) const;
  /*! @return @c true if this render state uses any form of culling, otherwise
   *  @c false.
   */
  bool isCulling(void) const;
  /*! @return @c true if this render state uses any form of blending with the
   *  framebuffer, otherwise @c false.
   */
  bool isBlending(void) const;
  /*! @return @c true if this render state uses depth buffer testing, otherwise
   *  @c false.
   */
  bool isDepthTesting(void) const;
  /*! @return @c true if this render state writes to the depth buffer, otherwise
   *  @c false.
   */
  bool isDepthWriting(void) const;
  /*! @return @c true if this render state writes to the color buffer, otherwise
   *  @c false.
   */
  bool isColorWriting(void) const;
  /*! @return @c true if this render state uses wireframe rendering, otherwise
   *  @c false.
   */
  bool isWireframe(void) const;
  /*! @return The culling mode of this render state.
   */
  CullMode getCullMode(void) const;
  /*! @return The source factor for color buffer blending.
   */
  BlendFactor getSrcFactor(void) const;
  /*! @return The destination factor for color buffer blending.
   */
  BlendFactor getDstFactor(void) const;
  /*! @return The depth buffer testing function used by this render state.
   */
  Function getDepthFunction(void) const;
  /*! Sets whether this render state uses depth buffer testing.
   *  @param[in] enable Set to @c true to enable depth buffer testing, or @c
   *  false to disable it.
   */
  void setDepthTesting(bool enable);
  /*! Sets whether this render state writes to the depth buffer.
   *  @param[in] enable Set to @c true to enable depth buffer writing, or @c
   *  false to disable it.
   */
  void setDepthWriting(bool enable);
  /*! Sets the depth buffer testing function for this render state.
   *  @param[in] function The desired depth testing function.
   */
  void setDepthFunction(Function function);
  /*! Sets whether writing to the color buffer is enabled.
   *  @param[in] enabled @c true to enable writing to the color buffer, or @c
   *  false to disable it.
   */
  void setColorWriting(bool enabled);
  /*! Sets whether wireframe rendering is enabled.
   *  @param[in] enabled @c true to enable wireframe rendering, or @c false to
   *  disable it.
   */
  void setWireframe(bool enabled);
  /*! Sets the specified primitive culling mode.
   *  @param[in] mode The desired new primitive culling mode.
   */
  void setCullMode(CullMode mode);
  /*! Sets the factors for color buffer blending.
   *  @param[in] src The desired source factor.
   *  @param[in] dst The desired destination factor.
   */
  void setBlendFactors(BlendFactor src, BlendFactor dst);
  /*! Resets all values in this render state to their defaults.
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
