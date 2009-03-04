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

class ShaderProgram;

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
  CULL_FRONT = GL_FRONT,
  /*! Cull back-facing geometry (i.e. render front-facing geometry).
   */
  CULL_BACK = GL_BACK,
  /*! Cull all cullable geometry (i.e. front and back faces).
   */
  CULL_BOTH = GL_FRONT_AND_BACK,
};

///////////////////////////////////////////////////////////////////////

class SamplerState
{
public:
  SamplerState(const String& name, Texture& texture);
  const String& getName(void) const;
  void setName(const String& newName);
  Texture& getTexture(void) const;
  void setTexture(Texture& newTexture);
private:
  String name;
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
  /*! @return The width of lines, in percent of the height of the current
   * render target.
   */
  float getLineWidth(void) const;
  /*! @return The culling mode of this render pass.
   */
  CullMode getCullMode(void) const;
  /*! @return The polygon rendering mode of this render pass.
   *
   *  @remarks The value may be @c GL_POINT, @c GL_LINE or @c GL_FILL.
   *  @remarks This corresponds to the @c glPolygonMode command.
   */
  GLenum getPolygonMode(void) const;
  GLenum getSrcFactor(void) const;
  GLenum getDstFactor(void) const;
  GLenum getDepthFunction(void) const;
  GLenum getAlphaFunction(void) const;
  /*! @return The shader program used by this render pass.
   */
  ShaderProgram* getShaderProgram(void) const;
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
  void setDepthFunction(GLenum function);
  void setAlphaFunction(GLenum function);
  /*! Sets whether writing to the color buffer is enabled.
   *  @param enabled @c true to enable writing to the color buffer, or @c false
   *  to disable it.
   */
  void setColorWriting(bool enabled);
  /*! Sets the specified line width.
   *  @param width The desired line width, in percent of the height of the
   *  current render target.
   */
  void setLineWidth(float width);
  void setCullMode(CullMode mode);
  /*! Sets the polygon rendering mode of this render pass.
   *  @param mode The desired polygon rendering mode. Possible values are:
   *  @arg GL_POINT
   *  @arg GL_LINE
   *  @arg GL_FILL
   *
   *  @remarks This corresponds to the @c glPolygonMode command.
   */
  void setPolygonMode(GLenum mode);
  void setBlendFactors(GLenum src, GLenum dst);
  /*! Sets the shader program used by this render pass.
   *  @param[in] newProgram The desired shader program, or @c NULL to use the
   *  default shader program.
   */
  void setShaderProgram(ShaderProgram* newProgram);
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
    float lineWidth;
    CullMode cullMode;
    GLenum polygonMode;
    GLenum srcFactor;
    GLenum dstFactor;
    GLenum depthFunction;
    GLenum alphaFunction;
    Ref<ShaderProgram> program;
  };
  void force(void) const;
  void setBooleanState(GLenum state, bool value) const;
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
