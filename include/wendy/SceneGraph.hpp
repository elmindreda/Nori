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
#ifndef WENDY_SCENEGRAPH_HPP
#define WENDY_SCENEGRAPH_HPP
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace scene
  {

///////////////////////////////////////////////////////////////////////

/*! @defgroup scene Scene graph API
 */

///////////////////////////////////////////////////////////////////////

class Node;
class Graph;

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
  Node(bool needsUpdate = false);
  /*! Destructor.
   */
  virtual ~Node();
  /*! Attaches the specified node as a child to this node.
   *  @param[in] child The node to attach to this node.
   *  @return @c true if successful, or false if the specified node was a
   *  parent of this node.
   */
  bool addChild(Node& child);
  /*! Removes the specified child node from this node.
   *  @param[in] child The child node to remove.
   */
  void removeFromParent();
  /*! Deletes all attached children.
   */
  void destroyChildren();
  /*! @return @c true if the specified node is a parent of this node, otherwise
   * @c false.
   */
  bool isChildOf(const Node& node) const;
  /*! @return @c true if this node has any children, otherwise @c false.
   */
  bool hasChildren() const { return !m_children.empty(); }
  Graph* graph() const { return m_graph; }
  /*! @return The parent of this node.
   */
  Node* parent() const { return m_parent; }
  /*! @return The list of children of this node.
   */
  const List& children() const { return m_children; }
  /*! @return The local-to-parent transform of this scene node.
   */
  const Transform3& localTransform() const { return m_local; }
  /*! Sets the local-to-parent transform of this scene node.
   */
  void setLocalTransform(const Transform3& newTransform);
  void setLocalPosition(const vec3& newPosition);
  void setLocalRotation(const quat& newRotation);
  void setLocalScale(float newScale);
  /*! @return The local-to-world transform of this scene node.
   */
  const Transform3& worldTransform() const;
  /*! @return The local space bounds of this node.
   */
  const Sphere& localBounds() const;
  /*! Sets the local space bounds of this node.
   *  @param[in] newBounds The desired new bounds of this node.
   */
  void setLocalBounds(const Sphere& newBounds);
  /*! @return The local space union of the bounds of this node and all its
   *  child nodes.
   */
  const Sphere& totalBounds() const;
protected:
  /*! Called when the scene graph is updated.  This is the correct place to put
   *  per-frame operations which affect the transform or bounds.
   */
  virtual void update();
  /*! Called when the scene graph is collecting rendering information.  All the
   *  operations required to render this scene node should be put into the
   *  specified render queue.
   *  @param[in,out] queue The render queue for collecting operations.
   */
  virtual void enqueue(render::Scene& scene, const Camera& camera) const;
private:
  Node(const Node& source);
  Node& operator = (const Node& source);
  void invalidateBounds();
  void invalidateWorldTransform();
  void setGraph(Graph* newGraph);
  bool m_needsUpdate;
  Node* m_parent;
  Graph* m_graph;
  List m_children;
  Transform3 m_local;
  mutable Transform3 m_world;
  mutable bool m_dirtyWorld;
  Sphere m_localBounds;
  mutable Sphere m_totalBounds;
  mutable bool m_dirtyBounds;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Scene graph.
 *  @ingroup scene
 *
 *  This class represents a single scene graph, and is a logical tree root node,
 *  although it doesn't have a transform or bounds.
 */
class Graph
{
  friend class Node;
public:
  ~Graph();
  void update();
  void enqueue(render::Scene& scene, const Camera& camera) const;
  void query(const Sphere& sphere, Node::List& nodes) const;
  void query(const Frustum& frustum, Node::List& nodes) const;
  void addRootNode(Node& node);
  void destroyRootNodes();
  const Node::List& roots() const { return m_roots; }
private:
  Node::List m_roots;
  Node::List m_updated;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class LightNode : public Node
{
public:
  LightNode();
  render::Light* light() const;
  void setLight(render::Light* newLight);
protected:
  void update();
  void enqueue(render::Scene& scene, const Camera& camera) const;
private:
  Ref<render::Light> m_light;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class ModelNode : public Node
{
public:
  ModelNode();
  bool isShadowCaster() const;
  void setCastsShadows(bool enabled);
  render::Model* model() const;
  void setModel(render::Model* newModel);
protected:
  void enqueue(render::Scene& scene, const Camera& camera) const;
private:
  Ref<render::Model> m_model;
  bool m_shadowCaster;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup scene
 */
class CameraNode : public Node
{
public:
  CameraNode();
  Camera* camera() const;
  void setCamera(Camera* newCamera);
protected:
  void update();
private:
  Ref<Camera> m_camera;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace scene*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_SCENEGRAPH_HPP*/
///////////////////////////////////////////////////////////////////////
