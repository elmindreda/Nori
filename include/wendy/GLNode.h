///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_GLNODE_H
#define WENDY_GLNODE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class SceneNode : public Node<SceneNode>
{
  friend class CameraNode;
public:
  SceneNode(void);
  ~SceneNode(void);
  bool isVisible(void) const;
  void setVisible(bool enabled);
  Transform3& getLocalTransform(void);
  const Transform3& getLocalTransform(void) const;
  const Transform3& getWorldTransform(void) const;
protected:
  virtual void prepare(void);
  virtual void enqueue(RenderQueue& queue) const;
private:
  bool visible;
  Transform3 local;
  mutable Transform3 world;
};

///////////////////////////////////////////////////////////////////////

class LightNode : public SceneNode
{
public:
  const String& getLightName(void) const;
  void setLightName(const String& newLightName);
protected:
  void enqueue(RenderQueue& queue) const;
private:
  String lightName;
};

///////////////////////////////////////////////////////////////////////

class MeshNode : public SceneNode
{
public:
  const String& getMeshName(void) const;
  void setMeshName(const String& newMeshName);
protected:
  void enqueue(RenderQueue& queue) const;
private:
  String meshName;
};

///////////////////////////////////////////////////////////////////////

class CameraNode : public SceneNode
{
public:
  CameraNode(void);
  void prepareTree(void);
  void renderTree(void) const;
  void enqueueTree(RenderQueue& queue) const;
  float getFOV(void) const;
  float getAspectRatio(void) const;
  void setFOV(float newFOV);
  void setAspectRatio(float newAspectRatio);
private:
  float FOV;
  float aspectRatio;
};

///////////////////////////////////////////////////////////////////////

class SpriteNode : public SceneNode
{
public:
  const String& getShaderName(void) const;
  void setShaderName(const String& newShaderName);
  const Vector2& getSpriteSize(void) const;
  void setSpriteSize(const Vector2& newSize);
protected:
  void enqueue(RenderQueue& queue) const;
private:
  String shaderName;
  Vector2 spriteSize;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLNODE_H*/
///////////////////////////////////////////////////////////////////////
