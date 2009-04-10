///////////////////////////////////////////////////////////////////////
// Wendy default renderer
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
#ifndef WENDY_RENDERSCENE_H
#define WENDY_RENDERSCENE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @defgroup scene Scene management API
 *  
 *  These classes make up the scene graph and scene management part of Wendy.
 */

///////////////////////////////////////////////////////////////////////

/*! Scene node enqueueing phase type enumeration.
 *  @ingroup scene
 */
enum QueuePhase
{
  /*! Light collection phase.
   */
  COLLECT_LIGHTS,
  /*! Collection phase for operations used to render stencil shadow volumes.
   */
  COLLECT_SHADOW_VOLUMES,
  /*! Collection phase for operations used to render a shadow map.
   */
  COLLECT_SHADOW_OBJECTS,
  /*! Default geometry collection phase.
   */
  COLLECT_GEOMETRY,
};

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
  const Sphere& getLocalBounds(void) const;
  void setLocalBounds(const Sphere& newBounds);
  const Sphere& getTotalBounds(void) const;
protected:
  void addedToParent(SceneNode& parent);
  void removedFromParent(void);
  /*! Called when the scene graph is updated. This is the correct place to put
   *  per-frame operations which affect the transform or bounds.
   */
  virtual void update(Time deltaTime);
  virtual void restart(void);
  /*! Called when the scene graph is collecting rendering information. All the
   *  operations required to render this scene node should be put into the
   *  specified render queue.
   *  @param queue The render queue for collecting operations.
   *  @param phase The current enqueueing phase.
   */
  virtual void enqueue(Queue& queue, QueuePhase phase) const;
private:
  bool updateWorldTransform(void) const;
  bool visible;
  Transform3 local;
  mutable Transform3 world;
  mutable bool dirtyWorld;
  Sphere localBounds;
  mutable Sphere totalBounds;
  mutable bool dirtyBounds;
};

///////////////////////////////////////////////////////////////////////

class SceneNodeType : public Managed<SceneNodeType>
{
public:
  SceneNodeType(const String& name);
  virtual ~SceneNodeType(void);
  virtual SceneNode* createNode(void) = 0;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
class SceneNodeTemplate
{
public:
  inline SceneNodeTemplate(const String& name);
  inline SceneNode* createNode(void);
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
  enum ShadowType
  {
    NO_SHADOWS,
    STENCIL_SHADOWS,
  };
  typedef std::vector<SceneNode*> NodeList;
  Scene(const String& name = "");
  ~Scene(void);
  void enqueue(Queue& queue) const;
  void query(const Sphere& bounds, NodeList& nodes) const;
  void query(const Frustum& frustum, NodeList& nodes) const;
  void addNode(SceneNode& node);
  void removeNode(SceneNode& node);
  void removeNodes(void);
  const NodeList& getNodes(void) const;
  Time getTimeElapsed(void) const;
  void setTimeElapsed(Time newTime);
  ShadowType getShadowType(void) const;
  void setShadowType(ShadowType newType);
private:
  NodeList roots;
  Time currentTime;
  ShadowType shadowType;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class LightNode : public SceneNode
{
public:
  Light* getLight(void) const;
  void setLight(Light* newLight);
protected:
  void update(Time deltaTime);
  void enqueue(Queue& queue, QueuePhase phase) const;
private:
  Ref<Light> light;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class MeshNode : public SceneNode
{
public:
  Mesh* getMesh(void) const;
  void setMesh(Mesh* mesh);
protected:
  void enqueue(Queue& queue, QueuePhase phase) const;
private:
  Ref<Mesh> mesh;
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
  void update(Time deltaTime);
private:
  String cameraName;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class TerrainNode : public SceneNode
{
public:
  Terrain* getTerrain(void) const;
  void setTerrain(Terrain* newTerrain);
protected:
  void update(Time deltaTime);
  void enqueue(Queue& queue, QueuePhase phase) const;
private:
  Ref<Terrain> terrain;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class SpriteNode : public SceneNode
{
public:
  SpriteNode(void);
  Style* getStyle(void) const;
  void setStyle(Style* newStyle);
  const Vector2& getSpriteSize(void) const;
  void setSpriteSize(const Vector2& newSize);
protected:
  void enqueue(Queue& queue, QueuePhase phase) const;
private:
  Ref<Style> style;
  Vector2 spriteSize;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class ParticleSystemNode : public SceneNode
{
public:
  ParticleSystemNode(void);
  const String& getSystemName(void) const;
  void setSystemName(const String& newSystemName);
protected:
  void update(Time deltaTime);
  void restart(void);
  void enqueue(Queue& queue, QueuePhase phase) const;
private:
  String systemName;
  Time elapsed;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
inline SceneNodeTemplate<T>::SceneNodeTemplate(const String& name):
  SceneNodeType(name)
{
}

template <typename T>
inline SceneNode* SceneNodeTemplate<T>::createNode(void)
{
  return new T();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERSCENE_H*/
///////////////////////////////////////////////////////////////////////
