///////////////////////////////////////////////////////////////////////
// Wendy scene graph
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
#ifndef WENDY_SCENEGRAPH_H
#define WENDY_SCENEGRAPH_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace scene
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @defgroup scene Scene management API
 *  
 *  These classes make up the scene graph and scene management part of Wendy.
 */

///////////////////////////////////////////////////////////////////////

class Node;

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

/*! @brief %Scene graph node factory super class.
 *  @ingroup scene
 */
class NodeType : public Managed<NodeType>
{
public:
  /*! Constructor.
   *  @param[in] name The desired name of this scene node type.
   */
  NodeType(const String& name);
  /*! Destructor.
   */
  virtual ~NodeType(void);
  virtual Node* createNode(void) = 0;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Scene graph node factory template.
 *  @ingroup scene
 */
template <typename T>
class NodeTemplate : public NodeType
{
public:
  inline NodeTemplate(const String& name);
  inline Node* createNode(void);
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Scene graph node base class.
 *  @ingroup scene
 *
 *  This is the base class for all kinds of nodes in a scene graph. It provides
 *  local and world transforms, and a set of callbacks for scene graph events.
 */
class Node
{
  friend class Graph;
public:
  typedef std::vector<Node*> List;
  /*! Constructor.
   */
  Node(void);
  /*! Copy constructor.
   */
  Node(const Node& source);
  /*! Destructor.
   */
  virtual ~Node(void);
  /*! Attaches the specified node as a child to this node.
   *  @param[in] child The node to attach to this node.
   *  @return @c true if successful, or false if the specified node was a
   *  parent of this node.
   */
  bool addChild(Node& child);
  /*! Removes the specified child node from this node.
   *  @param[in] child The child node to remove.
   */
  void removeFromParent(void);
  /*! Deletes all attached children.
   */
  void destroyChildren(void);
  /*! @return @c true if the specified node is a parent of this node, otherwise @c false.
   */
  bool isChildOf(const Node& node) const;
  /*! @return @c true if this scene node is visible, otherwise @c false.
   */
  bool isVisible(void) const;
  /*! @return @c true if this node has any children, otherwise @c false.
   */
  bool hasChildren(void) const;
  /*! @return The parent of this node.
   */
  Node* getParent(void) const;
  /*! @return The list of children of this node.
   */
  const List& getChildren(void) const;
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
  /*! @return The local space bounds of this node.
   */
  const Sphere& getLocalBounds(void) const;
  /*! Sets the local space bounds of this node.
   *  @param[in] newBounds The desired new bounds of this node.
   */
  void setLocalBounds(const Sphere& newBounds);
  /*! @return The local space union of the bounds of this node and all its
   *  child nodes.
   */
  const Sphere& getTotalBounds(void) const;
protected:
  virtual void addedToParent(Node& parent);
  virtual void removedFromParent(void);
  /*! Called when the scene graph is updated.  This is the correct place to put
   *  per-frame operations which affect the transform or bounds.
   *  @param[in] deltaTime The time elapsed, in seconds, since the last update.
   */
  virtual void update(Time deltaTime);
  /*! Called when the scene graph time is reset.
   */
  virtual void restart(void);
  /*! Called when the scene graph is collecting rendering information.  All the
   *  operations required to render this scene node should be put into the
   *  specified render queue.
   *  @param[in,out] queue The render queue for collecting operations.
   *  @param[in] phase The current enqueueing phase.
   */
  virtual void enqueue(render::Queue& queue, QueuePhase phase) const;
private:
  bool updateWorldTransform(void) const;
  Node* parent;
  List children;
  bool visible;
  Transform3 local;
  mutable Transform3 world;
  mutable bool dirtyWorld;
  Sphere localBounds;
  mutable Sphere totalBounds;
  mutable bool dirtyBounds;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Scene graph.
 *  @ingroup scene
 *
 *  This class represents a single scene graph, and is a logical tree root node,
 *  although it doesn't have a transform or bounds.
 */
class Graph : public Resource<Graph>
{
public:
  Graph(const String& name = "");
  ~Graph(void);
  void enqueue(render::Queue& queue) const;
  void query(const Sphere& bounds, Node::List& nodes) const;
  void query(const Frustum& frustum, Node::List& nodes) const;
  void addNode(Node& node);
  void removeNode(Node& node);
  void removeNodes(void);
  const Node::List& getNodes(void) const;
  Time getTimeElapsed(void) const;
  void setTimeElapsed(Time newTime);
private:
  Node::List roots;
  Time currentTime;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class LightNode : public Node
{
public:
  render::Light* getLight(void) const;
  void setLight(render::Light* newLight);
protected:
  void update(Time deltaTime);
  void enqueue(render::Queue& queue, QueuePhase phase) const;
private:
  Ref<render::Light> light;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class MeshNode : public Node
{
public:
  render::Mesh* getMesh(void) const;
  void setMesh(render::Mesh* mesh);
protected:
  void enqueue(render::Queue& queue, QueuePhase phase) const;
private:
  Ref<render::Mesh> mesh;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class CameraNode : public Node
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
class TerrainNode : public Node
{
public:
  render::Terrain* getTerrain(void) const;
  void setTerrain(render::Terrain* newTerrain);
protected:
  void update(Time deltaTime);
  void enqueue(render::Queue& queue, QueuePhase phase) const;
private:
  Ref<render::Terrain> terrain;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class SpriteNode : public Node
{
public:
  SpriteNode(void);
  render::Material* getMaterial(void) const;
  void setMaterial(render::Material* newMaterial);
  const Vec2& getSpriteSize(void) const;
  void setSpriteSize(const Vec2& newSize);
protected:
  void enqueue(render::Queue& queue, QueuePhase phase) const;
private:
  Ref<render::Material> material;
  Vec2 spriteSize;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class ParticleSystemNode : public Node
{
public:
  ParticleSystemNode(void);
  const String& getSystemName(void) const;
  void setSystemName(const String& newSystemName);
protected:
  void update(Time deltaTime);
  void restart(void);
  void enqueue(render::Queue& queue, QueuePhase phase) const;
private:
  String systemName;
  Time elapsed;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
inline NodeTemplate<T>::NodeTemplate(const String& name):
  NodeType(name)
{
}

template <typename T>
inline Node* NodeTemplate<T>::createNode(void)
{
  return new T();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace scene*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_SCENEGRAPH_H*/
///////////////////////////////////////////////////////////////////////
