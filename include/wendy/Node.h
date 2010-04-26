///////////////////////////////////////////////////////////////////////
// Wendy core library
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
#ifndef WENDY_NODE_H
#define WENDY_NODE_H
///////////////////////////////////////////////////////////////////////

#include <vector>
#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief Tree node mix-in template.
 *
 *  Inherit from this to participate in trees of class T.
 */
template <typename T>
class Node
{
public:
  typedef std::vector<T*> List;
  /*! Default constructor.
   */
  inline Node(void);
  /*! Copy constructor.
   *  @remarks Performs a deep copy of children, but does not inherit
   *  parent.
   */
  inline Node(const Node<T>& source);
  /*! Destructor.
   */
  inline virtual ~Node(void);
  /*! Attaches the specified node as a child to this node.
   *  @param[in] child The node to attach to this node.
   *  @return @c true if successful, or false if the specified node was a
   *  parent of this node.
   */
  inline bool addChild(T& child);
  /*! Attaches the specified node at the specified index as a child to this node.
   *  @param[in] child The node to attach to this node.
   *  @param[in] index The index to attach the node at.
   *  @return @c true if successful, or false if the specified node was a
   *  parent of this node.
   *  @remarks If the index is too large, the specified node will be attached
   *  last.
   */
  inline bool addChildAt(T& child, unsigned int index);
  /*! Deletes all attached children.
   */
  inline void destroyChildren(void);
  /*! Detaches the node from its parent.
   */
  inline void removeFromParent(void);
  /*! Assignment operator.
   * @remarks Performs a deep copy of children, but does not inherit parent.
   */
  inline Node<T>& operator = (const Node<T>& source);
  /*! @return @c true if the specified node is a parent of this node, otherwise @c false.
   */
  inline bool isChildOf(const T& node) const;
  /*! @return @c true if this node has any children, otherwise @c false.
   */
  inline bool hasChildren(void) const;
  /*! @return The root in the tree this node is attached to.
   */
  inline T* getRoot(void) const;
  /*! @return The parent of this node.
   */
  inline T* getParent(void) const;
  /*! @return The list of children of this node.
   */
  inline const List& getChildren(void) const
  { return children; }
private:
  T* parent;
  List children;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
inline Node<T>::Node(void):
  parent(NULL)
{
}

template <typename T>
inline Node<T>::Node(const Node<T>& source):
  parent(NULL)
{
}

template <typename T>
inline Node<T>::~Node(void)
{
  destroyChildren();
  removeFromParent();
}

template <typename T>
inline bool Node<T>::addChild(T& child)
{
  if (isChildOf(child))
    return false;

  static_cast<Node<T>&>(child).removeFromParent();

  children.push_back(&child);
  static_cast<Node<T>&>(child).parent = dynamic_cast<T*>(this);
  return true;
}

template <typename T>
inline bool Node<T>::addChildAt(T& child, unsigned int index)
{
  if (isChildOf(child))
    return false;

  static_cast<Node<T>&>(child).removeFromParent();

  typename List::iterator i = children.begin();
  std::advance(i, std::min(index, children.size()));
  children.insert(i, &child);

  static_cast<Node<T>&>(child).parent = dynamic_cast<T*>(this);
  return true;
}

template <typename T>
inline void Node<T>::destroyChildren(void)
{
  while (!children.empty())
    delete children.back();
}

template <typename T>
inline void Node<T>::removeFromParent(void)
{
  if (parent)
  {
    List& siblings = static_cast<Node<T>*>(parent)->children;
    typename List::iterator i = std::find(siblings.begin(), siblings.end(), this);
    if (i != siblings.end())
      siblings.erase(i);

    parent = NULL;
  }
}

template <typename T>
inline Node<T>& Node<T>::operator = (const Node<T>& source)
{
  destroyChildren();

  for (typename List::const_iterator i = source.children.begin();  i != source.children.end();  i++)
    addChild(new T(**i));

  return *this;
}

template <typename T>
inline bool Node<T>::isChildOf(const T& node) const
{
  if (parent != NULL)
  {
    if (parent == &node)
      return true;

    return static_cast<Node<T>*>(parent)->isChildOf(node);
  }

  return false;
}

template <typename T>
inline bool Node<T>::hasChildren(void) const
{
  return !children.empty();
}

template <typename T>
inline T* Node<T>::getRoot(void) const
{
  if (parent)
    return static_cast<Node<T>*>(parent)->getRoot();

  return dynamic_cast<T*>(this);
}

template <typename T>
inline T* Node<T>::getParent(void) const
{
  return parent;
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_NODE_H*/
///////////////////////////////////////////////////////////////////////
