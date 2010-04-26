///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2009 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_MANAGED_H
#define WENDY_MANAGED_H
///////////////////////////////////////////////////////////////////////

#include <map>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

String createRandomName(void);

///////////////////////////////////////////////////////////////////////

/*! @brief %Managed object template mixin.
 *
 *  Base class for managed objects with unique literal names.
 *  Names must be unique within each class that derives from this class.
 */
template <typename T>
class Managed
{
public:
  /*! List of instances, as returned by Managed::getInstances.
   */
  typedef std::vector<T*> InstanceList;
  /*! Constructor.
   *  @param name The desired name of this object, or the empty string to
   *  request automatic name generation.
   *
   *  @remarks If the name you specified already exists, an exception will be
   *  thrown.
   */
  inline Managed(const String& name);
  /*! Copy constructor.
   *
   *  @remarks If a managed object is created through a copy constructor, it
   *  receives an automatically generated name.
   */
  inline Managed(const Managed<T>& source);
  /*! Destructor.
   */
  inline virtual ~Managed(void);
  /*! Assignment operator.
   *
   *  @remarks This does not change the name of the object being assigned to.
   */
  inline Managed<T>& operator = (const Managed<T>& source);
  /*! @return The unique name of this object.
   */
  inline const String& getName(void) const;
  /*! Sets the name of this object.
   *  @param[in] newName The desired new name of this object.
   *  @return @c true if this object was successfully renamed, or @c false if
   *  the name already existed.
   *
   *  @note This feature is mainly for editor interfaces, which interact with
   *  users expecting to be able to rename objects at their leisure.  If you
   *  rename an object, make sure you change all the places where the previous
   *  name of the object is stored, as these will not be changed for you.
   *
   *  @note If you specify the empty string as the desired name, the object will
   *  get a new (guaranteed unique) automatically generated name.
   */
  inline bool setName(const String& newName);
  /*! Returns the instance with the specified name, if available.
   *  @param name The name of the desired instance.
   *  @return The instance with the specified name, or @c NULL if no such
   *  instance exists.
   */
  static inline T* findInstance(const String& name);
  /*! Destroys all instances of this type.
   *
   *  @remarks Use with care. This will backfire if you have objects of type T
   *  on the stack, or if higher-level objects own any of the destroyed
   *  instances.
   */
  static inline void destroyInstances(void);
  /*! Builds a list of all current instances of this type.
   *  @param instances The resulting list of instances.
   */
  static inline void getInstances(InstanceList& result);
private:
  static inline String generateName(void);
  typedef std::map<String, Managed<T>*> InstanceMap;
  String name;
  static InstanceMap instances;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
inline Managed<T>::Managed(const String& initName):
  name(initName)
{
  if (name.empty())
    name = generateName();
  else
  {
    if (findInstance(name))
      throw Exception("Duplicate name for managed object");
  }

  instances[name] = this;
}

template <typename T>
inline Managed<T>::Managed(const Managed<T>& source)
{
  name = generateName();

  instances[name] = this;
}

template <typename T>
inline Managed<T>::~Managed(void)
{
  instances.erase(name);
}

template <typename T>
inline Managed<T>& Managed<T>::operator = (const Managed<T>& source)
{
  return *this;
}

template <typename T>
inline const String& Managed<T>::getName(void) const
{
  return name;
}

template <typename T>
inline bool Managed<T>::setName(const String& newName)
{
  if (newName.empty())
  {
    name = generateName();
    return true;
  }
  else
  {
    if (findInstance(newName))
      return false;

    name = newName;
  }

  instances.erase(name);
  instances[name] = this;
  return true;
}

template <typename T>
inline T* Managed<T>::findInstance(const String& name)
{
  if (name.empty())
    return NULL;

  typename InstanceMap::iterator i = instances.find(name);
  if (i == instances.end())
    return NULL;

  return dynamic_cast<T*>(i->second);
}

template <typename T>
inline void Managed<T>::destroyInstances(void)
{
  while (!instances.empty())
    delete instances.begin()->second;
}

template <typename T>
inline void Managed<T>::getInstances(InstanceList& result)
{
  result.clear();

  for (typename InstanceMap::const_iterator i = instances.begin();  i != instances.end();  i++)
    result.push_back(dynamic_cast<T*>(i->second));
}

template <typename T>
inline String Managed<T>::generateName(void)
{
  String name;

  do
  {
    name = createRandomName();
  }
  while (findInstance(name));

  return name;
}

template <typename T>
typename Managed<T>::InstanceMap Managed<T>::instances;

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_MANAGED_H*/
///////////////////////////////////////////////////////////////////////
