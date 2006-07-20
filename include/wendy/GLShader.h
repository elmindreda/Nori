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

/*! Cull mode enumeration.
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

/*! Render pass.
 *  This class encapsulates most of the OpenGL rendering state, notable
 *  exceptions being the matrix stacks and object parameters such as
 *  those set by @c glTexParameter.  It is relatively cheap to
 *  construct and use, even for small render batches.
 *  @remarks If you mix Wendy rendering code with other OpenGL code
 *  that affects rendering state, you will need to call
 *  ShaderPass::invalidateCache as appropriate, to notify the Wendy
 *  rendering pipeline that its state cache is dirty.
 */
class ShaderPass
{
public:
  ShaderPass(void);
  /*! Applies the settings in this render pass to OpenGL and stores
   *  them in the internal cache.
   *  @remarks If the cache is dirty, all relevant OpenGL states will
   *  be forced to known values and the cache will then be considered
   *  clean.
   */
  void apply(void) const;
  /*! @return @c true if this render pass has been changed since the
   *  last call to ShaderPass::apply, otherwise @c false.
   */
  bool isDirty(void) const;
  bool isCulling(void) const;
  bool isBlending(void) const;
  bool isDepthTesting(void) const;
  bool isDepthWriting(void) const;
  bool isStencilTesting(void) const;
  bool isSphereMapped(void) const;
  bool isLit(void) const;
  float getLineWidth(void) const;
  CullMode getCullMode(void) const;
  GLenum getCombineMode(void) const;
  GLenum getPolygonMode(void) const;
  GLenum getShadeMode(void) const;
  GLenum getSrcFactor(void) const;
  GLenum getDstFactor(void) const;
  GLenum getDepthFunction(void) const;
  GLenum getAlphaFunction(void) const;
  GLenum getStencilFunction(void) const;
  GLenum getStencilFailOperation(void) const;
  GLenum getDepthFailOperation(void) const;
  GLenum getDepthPassOperation(void) const;
  unsigned int getStencilReference(void) const;
  unsigned int getStencilMask(void) const;
  float getShininess(void) const;
  const ColorRGBA& getDefaultColor(void) const;
  const ColorRGBA& getAmbientColor(void) const;
  const ColorRGBA& getDiffuseColor(void) const;
  const ColorRGBA& getSpecularColor(void) const;
  const ColorRGBA& getCombineColor(void) const;
  const String& getTextureName(void) const;
  const String& getVertexProgramName(void) const;
  const String& getFragmentProgramName(void) const;
  void setLit(bool enable);
  void setSphereMapped(bool enabled);
  void setDepthTesting(bool enable);
  void setDepthWriting(bool enable);
  void setStencilTesting(bool enable);
  void setDepthFunction(GLenum function);
  void setAlphaFunction(GLenum function);
  void setStencilFunction(GLenum function);
  void setStencilValues(unsigned int reference, unsigned int mask);
  void setStencilOperations(GLenum stencilFailed,
                            GLenum depthFailed,
                            GLenum depthPassed);
  void setLineWidth(float width);
  void setCullMode(CullMode mode);
  void setCombineMode(GLenum mode);
  void setPolygonMode(GLenum mode);
  void setShadeMode(GLenum mode);
  void setBlendFactors(GLenum src, GLenum dst);
  void setShininess(float newValue);
  void setDefaultColor(const ColorRGBA& color);
  void setAmbientColor(const ColorRGBA& color);
  void setDiffuseColor(const ColorRGBA& color);
  void setSpecularColor(const ColorRGBA& color);
  void setCombineColor(const ColorRGBA& color);
  void setTextureName(const String& name);
  void setVertexProgramName(const String& newName);
  void setFragmentProgramName(const String& newName);
  void setDefaults(void);
  /*! Flags the cache as dirty.  All states will be forced to known values
   *  on the next call to ShaderPass::apply.
   */
  static void invalidateCache(void);
private:
  class Data
  {
  public:
    Data(void);
    void setDefaults(void);
    mutable bool dirty;
    bool lighting;
    bool depthTesting;
    bool depthWriting;
    bool stencilTesting;
    bool sphereMapped;
    float lineWidth;
    CullMode cullMode;
    GLenum combineMode;
    GLenum polygonMode;
    GLenum shadeMode;
    GLenum srcFactor;
    GLenum dstFactor;
    GLenum depthFunction;
    GLenum alphaFunction;
    GLenum stencilFunction;
    unsigned int stencilRef;
    unsigned int stencilMask;
    GLenum stencilFailed;
    GLenum depthFailed;
    GLenum depthPassed;
    float shininess;
    ColorRGBA defaultColor;
    ColorRGBA ambientColor;
    ColorRGBA diffuseColor;
    ColorRGBA specularColor;
    ColorRGBA combineColor;
    String textureName;
    String vertexProgramName;
    String fragmentProgramName;
    GLenum textureTarget;
  };
  void setBooleanState(GLenum state, bool value) const;
  Data data;
  static Data cache;
};

///////////////////////////////////////////////////////////////////////

/*! Render style.
 *  This is the highest-level render style object, encapsulating
 *  multiple render passes, each of which controls virtually all
 *  relevant state for a single render pass.
 */
class Shader : public Managed<Shader>
{
public:
  Shader(const String& name = "");
  ShaderPass& createPass(void);
  void destroyPasses(void);
  void applyPass(unsigned int index) const;
  bool operator < (const Shader& other) const;
  bool isBlending(void) const;
  ShaderPass& getPass(unsigned int index);
  const ShaderPass& getPass(unsigned int index) const;
  unsigned int getPassCount(void) const;
private:
  typedef std::list<ShaderPass> PassList;
  PassList passes;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLSHADER_H*/
///////////////////////////////////////////////////////////////////////
