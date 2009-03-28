///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_GLPASS_H
#define WENDY_GLPASS_H
///////////////////////////////////////////////////////////////////////

#include <list>
#include <vector>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Uniform;
class Sampler;
class Program;

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

/*! State for a single shader uniform.
 *  @ingroup opengl
 */
class UniformState
{
  friend class Pass;
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

/*! State for a single shader sampler uniform.
 *  @ingroup opengl
 */
class SamplerState
{
  friend class Pass;
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

/*! @brief Render pass state object.
 *  @ingroup opengl
 *
 *  This class and its associated classes encapsulates most of the OpenGL
 *  rendering state, notable exceptions being the object parameters.
 *
 *  @remarks Unless you're writing your own custom renderer, you will probably
 *  want to use the classes in the render namespace, and not use these classes
 *  independently.
 *
 *  @remarks Yes, it's big.
 */
class Pass
{
public:
  /*! Constructor.
   */
  Pass(const String& name = "");
  /*! Destructor.
   */
  ~Pass(void);
  /*! Applies the settings in this render pass to OpenGL and stores them in the
   *  internal cache.
   *
   *  @remarks If the cache is dirty, all relevant OpenGL states will be forced
   *  to known values and the cache will then be considered clean.
   */
  void apply(void) const;
  bool isCompatible(void) const;
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
  bool hasSampler(const String& name) const;
  /*! @return The culling mode of this render pass.
   */
  CullMode getCullMode(void) const;
  BlendFactor getSrcFactor(void) const;
  BlendFactor getDstFactor(void) const;
  Function getDepthFunction(void) const;
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
  const String& getName(void) const;
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
  /*! Sets the shader program used by this render pass.
   *  @param[in] newProgram The desired shader program, or @c NULL to use the
   *  default shader program.
   */
  void setProgram(Program* newProgram);
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
    Ref<Program> program;
  };
  void force(void) const;
  void setBooleanState(unsigned int state, bool value) const;
  void destroyProgramState(void);
  typedef std::vector<UniformState*> UniformList;
  typedef std::vector<SamplerState*> SamplerList;
  UniformList uniforms;
  SamplerList samplers;
  Data data;
  String name;
  static Data cache;
  static bool cullingInverted;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLPASS_H*/
///////////////////////////////////////////////////////////////////////
