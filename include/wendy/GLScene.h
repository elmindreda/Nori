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
#ifndef WENDY_GLSCENE_H
#define WENDY_GLSCENE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @defgroup scene Scene management API
 *  
 *  These classes make up the scene graph and scene management part of Wendy.
 */

///////////////////////////////////////////////////////////////////////

/*! @brief %Scene graph node base class.
 *  @ingroup scene
 *
 *  This is the base class for all kinds of nodes in a scene graph. It provides
 *  local and world transforms, and a set of callbacks for scene graph events.
 */
class SceneNode : public Node<SceneNode>
{
  friend class Scene;
public:
  /*! Constructor.
   */
  SceneNode(void);
  /*! Destructor.
   */
  ~SceneNode(void);
  /*! @return @c true if this scene node is visible, otherwise @c false.
   */
  bool isVisible(void) const;
  /*! Sets the visible state of this scene node.
   *  @param enabled @c true to make this scene node visible, or @c false to
   *  hide it.
   */
  void setVisible(bool enabled);
  /*! @return The local-to-parent transform of this scene node.
   */
  Transform3& getLocalTransform(void);
  /*! @return The local-to-parent transform of this scene node.
   */
  const Transform3& getLocalTransform(void) const;
  /*! @return The local-to-world transform of this scene node.
   */
  const Transform3& getWorldTransform(void) const;
protected:
  void addedToParent(SceneNode& parent);
  void removedFromParent(void);
  /*! Called when the scene graph is updated. This is the correct place to put
   *  per-frame operations which affect the transform or bounds.
   */
  virtual void update(void);
  /*! Called when the scene graph is collecting rendering information. All the
   *  operations required to render this scene node should be put into the
   *  specified render queue.
   *  @param queue The render queue for collecting operations.
   */
  virtual void enqueue(RenderQueue& queue) const;
private:
  bool updateWorldTransform(void) const;
  bool visible;
  Transform3 local;
  mutable Transform3 world;
  mutable bool dirtyWorld;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Scene graph container.
 *  @ingroup scene
 *
 *  This class represents a single scene graph, and is a logical tree root node,
 *  although it doesn't have a transform or bounds.
 */
class Scene : public Managed<Scene>
{
public:
  Scene(const String& name = "");
  void updateTree(void);
  void renderTree(const Camera& camera) const;
  void enqueueTree(RenderQueue& queue) const;
  void addRootNode(SceneNode& node);
  void removeRootNode(SceneNode& node);
  void removeRootNodes(void);
  bool isFogging(void) const;
  void setFogging(bool newState);
  const ColorRGB& getFogColor(void) const;
  void setFogColor(const ColorRGB& newColor);
private:
  typedef std::list<SceneNode*> NodeList;
  NodeList roots;
  bool fogging;
  ColorRGB fogColor;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
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

/*! @ingroup scene
 */
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

/*! @ingroup scene
 */
class CameraNode : public SceneNode
{
public:
  const String& getCameraName(void) const;
  void setCameraName(const String& newName);
protected:
  void update(void);
private:
  String cameraName;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class TerrainNode : public SceneNode
{
public:
  const String& getTerrainName(void) const;
  void setTerrainName(const String& newTerrainName);
protected:
  void enqueue(RenderQueue& queue) const;
private:
  String terrainName;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class SpriteNode : public SceneNode
{
public:
  const String& getStyleName(void) const;
  void setStyleName(const String& newStyleName);
  const Vector2& getSpriteSize(void) const;
  void setSpriteSize(const Vector2& newSize);
protected:
  void enqueue(RenderQueue& queue) const;
private:
  String styleName;
  Vector2 spriteSize;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class ParticleSystemNode : public SceneNode
{
public:
  const String& getSystemName(void) const;
  void setSystemName(const String& newSystemName);
protected:
  void update(void);
  void enqueue(RenderQueue& queue) const;
private:
  String styleName;
  String systemName;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLSCENE_H*/
///////////////////////////////////////////////////////////////////////
