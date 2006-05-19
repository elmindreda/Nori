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
#ifndef WENDY_GLCANVAS_H
#define WENDY_GLCANVAS_H
///////////////////////////////////////////////////////////////////////

#include <list>
#include <stack>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Texture;

///////////////////////////////////////////////////////////////////////

/*
class Visual
{
  friend class Canvas;
public:
  Visual(void);
  virtual ~Visual(void);
  bool isVisible(void) const;
  void setVisible(bool enabled);
protected:
  virtual void prepare(void) const;
  virtual void render(void) const;
private:
  bool visible;
};

///////////////////////////////////////////////////////////////////////

class Layer
{
public:
  typedef std::list<Visual*> VisualList;
  Layer(const std::string& initName);
  const std::string& getName(void) const;
  VisualList visuals;
private:
  std::string name;
};

///////////////////////////////////////////////////////////////////////

class LayerStack
{
public:
  Layer* createLayer(const std::string& name);
  Layer* findLayer(const std::string& name);
  const Layer* findLayer(const std::string& name) const;
  void destroyLayers(void);
  Layer* getLayer(unsigned int index);
  const Layer* getLayer(unsigned int index) const;
  unsigned int getLayerCount(void) const;
  bool addVisual(Visual& visual, const std::string& layerName);
  void removeVisual(Visual& visual);
  void prepareVisuals(void);
  void renderVisuals(void);
private:
  typedef std::list<Layer> LayerList;
  LayerList layers;
};
*/

///////////////////////////////////////////////////////////////////////

class Canvas
{
public:
  Canvas(void);
  virtual ~Canvas(void);
  virtual void push(void);
  virtual void pop(void);
  void begin2D(const Vector2& resolution = Vector2(1.f, 1.f)) const;
  void begin3D(float aspect,
               float FOV = 90.f,
	       float nearZ = 0.01f,
	       float farZ = 1000.f) const;
  void end(void) const;
  virtual void clearColor(const ColorRGBA& color = ColorRGBA::BLACK);
  virtual void clearDepth(float depth = 1.f);
  virtual void clearStencil(unsigned int value = 0);
  virtual unsigned int getPhysicalWidth(void) const = 0;
  virtual unsigned int getPhysicalHeight(void) const = 0;
  const Vector2& getAreaPosition(void) const;
  const Vector2& getAreaSize(void) const;
  void setArea(const Vector2& newPosition, const Vector2& newSize);
  static Canvas* getCurrent(void);
protected:
  virtual void apply(void) = 0;
  Vector2 position;
  Vector2 size;
private:
  typedef std::stack<Canvas*> CanvasStack;
  static CanvasStack stack;
};

///////////////////////////////////////////////////////////////////////

class ContextCanvas : public Canvas
{
public:
  unsigned int getPhysicalWidth(void) const;
  unsigned int getPhysicalHeight(void) const;
private:
  void apply(void);
};

///////////////////////////////////////////////////////////////////////

class TextureCanvas : public Canvas
{
public:
  void pop(void);
  unsigned int getPhysicalWidth(void) const;
  unsigned int getPhysicalHeight(void) const;
  const Texture& getTexture(void) const;
  static TextureCanvas* createInstance(const std::string& textureName,
                                       unsigned int width,
                                       unsigned int height);
private:
  TextureCanvas(void);
  bool init(const std::string& textureName, unsigned int width, unsigned int height);
  void apply(void);
  Ptr<Texture> texture;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLCANVAS_H*/
///////////////////////////////////////////////////////////////////////
