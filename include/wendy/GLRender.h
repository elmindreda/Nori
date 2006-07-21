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
#ifndef WENDY_GLRENDER_H
#define WENDY_GLRENDER_H
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

class Light;
class VertexBuffer;
class IndexBuffer;

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

/*! Render pass state object.
 *  This class encapsulates most of the OpenGL rendering state, notable
 *  exceptions being the matrix stacks and object parameters such as
 *  those set by @c glTexParameter.  It is relatively cheap to
 *  construct and use, even for small render batches.
 *  @remarks If you mix Wendy rendering code with other OpenGL code
 *  that affects rendering state, you will need to call
 *  RenderPass::invalidateCache as appropriate, to notify the Wendy
 *  rendering pipeline that its state cache is dirty.
 */
class RenderPass
{
public:
  RenderPass(void);
  /*! Applies the settings in this render pass to OpenGL and stores
   *  them in the internal cache.
   *  @remarks If the cache is dirty, all relevant OpenGL states will
   *  be forced to known values and the cache will then be considered
   *  clean.
   */
  void apply(void) const;
  /*! @return @c true if this render pass has been changed since the
   *  last call to RenderPass::apply, otherwise @c false.
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
   *  on the next call to RenderPass::apply.
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
class RenderStyle : public Managed<RenderStyle>
{
public:
  RenderStyle(const String& name = "");
  RenderPass& createPass(void);
  void destroyPasses(void);
  void applyPass(unsigned int index) const;
  bool operator < (const RenderStyle& other) const;
  bool isBlending(void) const;
  RenderPass& getPass(unsigned int index);
  const RenderPass& getPass(unsigned int index) const;
  unsigned int getPassCount(void) const;
private:
  typedef std::list<RenderPass> PassList;
  PassList passes;
};

///////////////////////////////////////////////////////////////////////

class RenderOperation
{
public:
  RenderOperation(void);
  bool operator < (const RenderOperation& other) const;
  const VertexBuffer* vertexBuffer;
  const IndexBuffer* indexBuffer;
  const RenderStyle* style;
  unsigned int start;
  unsigned int count;
  Matrix4 transform;
  GLenum renderMode;
};

///////////////////////////////////////////////////////////////////////

class RenderQueue
{
public:
  typedef std::vector<Light*> LightList;
  typedef std::vector<RenderOperation> OperationList;
  RenderQueue(void);
  void addLight(Light& light);
  void removeLights(void);
  void addOperation(RenderOperation& operation);
  void removeOperations(void);
  void renderOperations(void);
  const LightList& getLights(void) const;
  const OperationList& getOperations(void) const;
private:
  void sortOperations(void);
  LightList lights;
  OperationList operations;
  bool sorted;
};

///////////////////////////////////////////////////////////////////////

class Renderer : public Trackable, public Singleton<Renderer>
{
public:
  bool allocateIndices(IndexBufferRange& range,
		       unsigned int count,
                       IndexBuffer::Type type = IndexBuffer::UBYTE);
  bool allocateVertices(VertexBufferRange& range,
			unsigned int count,
                        const VertexFormat& format);
  static bool create(void);
private:
  class IndexBufferSlot;
  class VertexBufferSlot;
  Renderer(void);
  bool init(void);
  void onFinish(void);
  typedef std::list<IndexBufferSlot> IndexBufferList;
  typedef std::list<VertexBufferSlot> VertexBufferList;
  IndexBufferList indexBuffers;
  VertexBufferList vertexBuffers;
};

///////////////////////////////////////////////////////////////////////

class Renderer::IndexBufferSlot
{
public:
  Ptr<IndexBuffer> indexBuffer;
  unsigned int available;
};

///////////////////////////////////////////////////////////////////////

class Renderer::VertexBufferSlot
{
public:
  Ptr<VertexBuffer> vertexBuffer;
  unsigned int available;
};

///////////////////////////////////////////////////////////////////////

/*
class Renderer : public Singleton<Renderer>
{
public:
  void begin(void);
  void end(void);
  void drawLine(const Vector2& start, const Vector2& end) const;
  void drawCircle(const Vector2& center, float radius) const;
  void drawBezier(const BezierCurve2& curve) const;
  void drawRectangle(const Rectangle& rectangle) const;
  bool isStroking(void) const;
  void setStroking(bool newState);
  bool isFilling(void) const;
  void setFilling(bool newState);
  const ColorRGBA& getStrokeColor(void) const;
  void setStrokeColor(const ColorRGBA& newColor);
  const ColorRGBA& getFillColor(void) const;
  void setFillColor(const ColorRGBA& newColor);
  float getStrokeWidth(void) const;
  void setStrokeWidth(float newWidth);
  static bool create(void);
private:
  class Context;
  Renderer(void);
  bool init(void);
  float getLineScale(void) const;
  Context& getContext(void);
  const Context& getContext(void) const;
  typedef std::stack<Context> ContextStack;
  ContextStack stack;
};

///////////////////////////////////////////////////////////////////////

class Renderer::Context
{
public:
  Context(void);
  RenderPass strokePass;
  RenderPass fillPass;
  bool stroking;
  bool filling;
};
*/

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLRENDER_H*/
///////////////////////////////////////////////////////////////////////
