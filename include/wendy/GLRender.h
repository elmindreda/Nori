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

class Shader;
class Light;
class VertexBuffer;
class IndexBuffer;

///////////////////////////////////////////////////////////////////////

class VertexAllocation
{
public:
  VertexBuffer* vertexBuffer;
  unsigned int start;
  unsigned int count;
};

///////////////////////////////////////////////////////////////////////

class RenderOperation
{
public:
  RenderOperation(void);
  bool operator < (const RenderOperation& other) const;
  const VertexBuffer* vertexBuffer;
  const IndexBuffer* indexBuffer;
  const Shader* shader;
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
  ShaderPass strokePass;
  ShaderPass fillPass;
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
