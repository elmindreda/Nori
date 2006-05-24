///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@home.se>
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

enum CullMode
{
  CULL_NONE,
  CULL_FRONT = GL_FRONT,
  CULL_BACK = GL_BACK,
  CULL_BOTH = GL_FRONT_AND_BACK,
};

///////////////////////////////////////////////////////////////////////

class ShaderPass
{
public:
  ShaderPass(void);
  void apply(void) const;
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
  const ColorRGBA& getDefaultColor(void) const;
  const ColorRGBA& getAmbientColor(void) const;
  const ColorRGBA& getDiffuseColor(void) const;
  const ColorRGBA& getSpecularColor(void) const;
  const ColorRGBA& getCombineColor(void) const;
  const std::string& getTextureName(void) const;
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
  void setDefaultColor(const ColorRGBA& color);
  void setAmbientColor(const ColorRGBA& color);
  void setDiffuseColor(const ColorRGBA& color);
  void setSpecularColor(const ColorRGBA& color);
  void setCombineColor(const ColorRGBA& color);
  void setTextureName(const std::string& name);
  void setDefaults(void);
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
    ColorRGBA defaultColor;
    ColorRGBA ambientColor;
    ColorRGBA diffuseColor;
    ColorRGBA specularColor;
    ColorRGBA combineColor;
    std::string textureName;
    GLenum textureTarget;
  };
  void setBooleanState(GLenum state, bool value) const;
  Data data;
  static Data cache;
};

///////////////////////////////////////////////////////////////////////

class Shader : public Managed<Shader>
{
public:
  virtual ~Shader(void);
  ShaderPass& createPass(void);
  void destroyPasses(void);
  void applyPass(unsigned int index) const;
  bool operator < (const Shader& other) const;
  bool isBlending(void) const;
  ShaderPass& getPass(unsigned int index);
  const ShaderPass& getPass(unsigned int index) const;
  unsigned int getPassCount(void) const;
  static Shader* createInstance(const std::string& name);
private:
  Shader(const std::string& name);
  bool init(void);
  typedef std::list<ShaderPass> PassList;
  PassList passes;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLSHADER_H*/
///////////////////////////////////////////////////////////////////////
