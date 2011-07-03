///////////////////////////////////////////////////////////////////////
// Wendy Squirrel bindings - based on Sqrat
// Copyright (c) 2009 Brandon Jones
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_SQUIRREL_H
#define WENDY_SQUIRREL_H
///////////////////////////////////////////////////////////////////////

#include <squirrel.h>

///////////////////////////////////////////////////////////////////////

/*! @defgroup squirrel Squirrel bindings
 *
 *  These classes provide an easy way to bind C++ to Squirrel.  They are
 *  based on Sqrat, but smaller and without some of its features.
 */

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace sq
  {

///////////////////////////////////////////////////////////////////////

class Object;
class Table;

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
class VM
{
  friend class Object;
public:
  VM(ResourceIndex& index);
  ~VM(void);
  bool execute(const Path& path);
  bool execute(const char* name, const char* text);
  operator HSQUIRRELVM (void);
  Table getRootTable(void);
  Table getRegistryTable(void);
  ResourceIndex& getIndex(void) const;
private:
  static void onLogMessage(HSQUIRRELVM vm, const SQChar* format, ...);
  static void onLogError(HSQUIRRELVM vm, const SQChar* format, ...);
  static void onCompilerError(HSQUIRRELVM vm,
                              const SQChar* description,
                              const SQChar* source,
                              SQInteger line,
                              SQInteger column);
  static SQInteger onRuntimeError(HSQUIRRELVM vm);
  ResourceIndex& index;
  HSQUIRRELVM vm;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
template <typename T>
class Value
{
public:
  inline static T get(HSQUIRRELVM vm, SQInteger index);
  inline static void push(HSQUIRRELVM vm, T value);
};

/*! @ingroup squirrel
 */
template <>
class Value<bool>
{
public:
  inline static bool get(HSQUIRRELVM vm, SQInteger index)
  {
    SQBool value;
    sq_getbool(vm, index, &value);
    return value ? true : false;
  }
  inline static void push(HSQUIRRELVM vm, bool value)
  {
    sq_pushbool(vm, SQBool(value));
  }
};

/*! @ingroup squirrel
 */
template <>
class Value<int>
{
public:
  inline static int get(HSQUIRRELVM vm, SQInteger index)
  {
    SQInteger value;
    sq_getinteger(vm, index, &value);
    return int(value);
  }
  inline static void push(HSQUIRRELVM vm, int value)
  {
    sq_pushinteger(vm, SQInteger(value));
  }
};

/*! @ingroup squirrel
 */
template <>
class Value<float>
{
public:
  inline static float get(HSQUIRRELVM vm, SQInteger index)
  {
    SQFloat value;
    sq_getfloat(vm, index, &value);
    return float(value);
  }
  inline static void push(HSQUIRRELVM vm, float value)
  {
    sq_pushfloat(vm, SQFloat(value));
  }
};

/*! @ingroup squirrel
 */
template <>
class Value<String>
{
public:
  inline static String get(HSQUIRRELVM vm, SQInteger index)
  {
    const SQChar* value;
    sq_getstring(vm, index, &value);
    return String(value);
  }
  inline static void push(HSQUIRRELVM vm, String value)
  {
    sq_pushstring(vm, value.c_str(), -1);
  }
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
class Object
{
public:
  Object(void);
  Object(HSQUIRRELVM vm, SQInteger index);
  Object(const Object& source);
  ~Object(void);
  template <typename T>
  inline T cast(void) const;
  Object& operator = (const Object& source);
  bool isNull(void) const;
  bool isArray(void) const;
  bool isTable(void) const;
  bool isClass(void) const;
  String asString(void) const;
  HSQOBJECT getHandle(void);
  HSQUIRRELVM getVM(void) const;
protected:
  Object(HSQUIRRELVM vm);
  template <typename T>
  inline bool addSlot(const char* name, T value);
  bool removeSlot(const char* name);
  void addFunction(const char* name,
                   void* pointer,
                   size_t pointerSize,
                   SQFUNCTION function,
                   bool staticMember = false);
  void clear(void);
  template <typename T>
  inline T get(const char* name);
  template <typename T>
  inline void set(const char* name, T value);
  SQInteger getSize(void) const;
  HSQUIRRELVM vm;
  HSQOBJECT handle;
};

template <typename T>
inline T Object::cast(void) const
{
  sq_pushobject(vm, handle);
  T value = Value<T>::get(vm, -1);
  sq_poptop(vm);
  return value;
}

template <typename T>
inline bool Object::addSlot(const char* name, T value)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  Value<T>::push(vm, value);

  const SQRESULT result =  sq_newslot(vm, -3, false);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

template <typename T>
inline T Object::get(const char* name)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  if (SQ_FAILED(sq_get(vm, -2)))
  {
    sq_poptop(vm);
    return T();
  }

  T result = Value<T>::get(vm, -1);
  sq_pop(vm, 2);

  return result;
}

template <typename T>
inline void Object::set(const char* name, T value)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  Value<T>::push(vm, value);
  if (SQ_FAILED(sq_set(vm, -3)))
    throw Exception("No such slot");
  sq_poptop(vm);
}

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
template <>
class Value<Object>
{
public:
  inline static Object get(HSQUIRRELVM vm, SQInteger index)
  {
    return Object(vm, index);
  }
  inline static void push(HSQUIRRELVM vm, Object value)
  {
    sq_pushobject(vm, value.getHandle());
  }
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
class Array : public Object
{
public:
  Array(HSQUIRRELVM vm);
  Array(HSQUIRRELVM vm, SQInteger index);
  template <typename T>
  inline void insert(T value, SQInteger index);
  void remove(SQInteger index);
  template <typename T>
  inline void push(T value);
  void pop(void);
  void resize(SQInteger newSize);
  void reverse(void);
  using Object::clear;
  Object operator [] (SQInteger index) const;
  template <typename T>
  inline T get(SQInteger index) const;
  template <typename T>
  inline void set(SQInteger index, T value);
  using Object::getSize;
};

template <typename T>
inline void Array::insert(T value, SQInteger index)
{
  sq_pushobject(vm, handle);
  Value<T>::push(vm, value);
  sq_arrayinsert(vm, -2, index);
  sq_poptop(vm);
}

template <typename T>
inline void Array::push(T value)
{
  sq_pushobject(vm, handle);
  Value<T>::push(vm, value);
  sq_arrayappend(vm, -2);
  sq_poptop(vm);
}

template <typename T>
inline T Array::get(SQInteger index) const
{
  sq_pushobject(vm, handle);
  sq_pushinteger(vm, index);
  if (SQ_FAILED(sq_get(vm, -2)))
    throw Exception("No array element at index");
  T result = Value<T>::get(vm, -1);
  sq_pop(vm, 2);
  return result;
}

template <typename T>
inline void Array::set(SQInteger index, T value)
{
  sq_pushobject(vm, handle);
  sq_pushinteger(vm, index);
  Value<T>::push(vm, value);
  if (SQ_FAILED(sq_set(vm, -3)))
    throw Exception("No array element at index");
  sq_poptop(vm);
}

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
template <>
class Value<Array>
{
public:
  inline static Array get(HSQUIRRELVM vm, SQInteger index)
  {
    return Array(vm, index);
  }
  inline static void push(HSQUIRRELVM vm, Array value)
  {
    sq_pushobject(vm, value.getHandle());
  }
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
class Table : public Object
{
public:
  Table(HSQUIRRELVM vm);
  Table(HSQUIRRELVM vm, SQInteger index);
  using Object::addSlot;
  using Object::removeSlot;
  using Object::clear;
  using Object::getSize;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
template <>
class Value<Table>
{
public:
  inline static Table get(HSQUIRRELVM vm, SQInteger index)
  {
    return Table(vm, index);
  }
  inline static void push(HSQUIRRELVM vm, Table value)
  {
    sq_pushobject(vm, value.getHandle());
  }
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
template <typename T, typename R>
class Method
{
public:
  inline static SQInteger demarshal0(HSQUIRRELVM vm)
  {
    typedef R (T::*Method)();

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    Value<R>::push(vm, (instance->**method)());
    return 1;
  }
  template <typename A1>
  inline static SQInteger demarshal1(HSQUIRRELVM vm)
  {
    typedef R (T::*Method)(A1);

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    Value<R>::push(vm, (instance->**method)(Value<A1>::get(vm, 2)));
    return 1;
  }
};

/*! @ingroup squirrel
 */
template <typename T>
class Method<T, void>
{
public:
  inline static SQInteger demarshal0(HSQUIRRELVM vm)
  {
    typedef void (T::*Method)();

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    (instance->**method)();
    return 0;
  }
  template <typename A1>
  inline static SQInteger demarshal1(HSQUIRRELVM vm)
  {
    typedef void (T::*Method)(A1);

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    (instance->**method)(Value<A1>::get(vm, 2));
    return 0;
  }
};

///////////////////////////////////////////////////////////////////////

template <typename T, typename R>
inline SQFUNCTION demarshal(R (T::*method)())
{
  return &Method<T,R>::demarshal0;
}

template <typename T, typename R, typename A1>
inline SQFUNCTION demarshal(R (T::*method)(A1))
{
  return &Method<T,R>::template demarshal1<A1>;
}

///////////////////////////////////////////////////////////////////////

class Class : public Object
{
public:
  Class(HSQUIRRELVM vm);
  Class(HSQUIRRELVM vm, SQInteger index);
  template <typename T>
  inline bool addSlot(const char* name, T value);
  bool removeSlot(const char* name);
  void clear(void);
  template <typename T>
  inline T get(const char* name);
  SQInteger getSize(void) const;
protected:
  Class(void);
};

template <typename T>
inline bool Class::addSlot(const char* name, T value)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  Value<T>::push(vm, value);

  const SQRESULT result =  sq_newslot(vm, -3, false);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

template <typename T>
inline T Class::get(const char* name)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  if (SQ_FAILED(sq_get(vm, -2)))
  {
    sq_poptop(vm);
    return T();
  }

  T result = Value<T>::get(vm, -1);
  sq_pop(vm, 2);

  return result;
}

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
template <typename T>
class NativeClass : public Class
{
public:
  inline NativeClass(HSQUIRRELVM vm);
  template <typename M>
  inline void addMethod(const char* name, M method)
  {
    addFunction(name, &method, sizeof(method), demarshal(method), false);
  }
  template <typename M>
  inline void addStaticMethod(const char* name, M method)
  {
    addFunction(name, &method, sizeof(method), demarshal(method), true);
  }
  inline static HSQOBJECT getHandle(void);
private:
  static inline SQInteger create(HSQUIRRELVM vm);
  static inline SQInteger destroy(SQUserPointer pointer, SQInteger size);
  static bool initialized;
  static HSQOBJECT shared;
};

template <typename T>
inline NativeClass<T>::NativeClass(HSQUIRRELVM initVM)
{
  vm = initVM;

  if (!initialized)
  {
    sq_newclass(vm, false);

    sq_resetobject(&shared);
    sq_getstackobj(vm, -1, &shared);
    sq_addref(vm, &shared);

    sq_pushstring(vm, "constructor", -1);
    sq_newclosure(vm, &create, 0);
    sq_newslot(vm, -3, false);

    sq_poptop(vm);

    initialized = true;
  }

  handle = shared;
  sq_addref(vm, &handle);
}

template <typename T>
inline HSQOBJECT NativeClass<T>::getHandle(void)
{
  return shared;
}

template <typename T>
inline SQInteger NativeClass<T>::create(HSQUIRRELVM vm)
{
  T* instance = new T();
  sq_setinstanceup(vm, 1, instance);
  sq_setreleasehook(vm, 1, &destroy);

  return 0;
}

template <typename T>
inline SQInteger NativeClass<T>::destroy(SQUserPointer pointer, SQInteger size)
{
  delete reinterpret_cast<T*>(pointer);

  return 0;
}

template <typename T>
bool NativeClass<T>::initialized = false;

template <typename T>
HSQOBJECT NativeClass<T>::shared;

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
template <typename T>
class Value<NativeClass<T> >
{
public:
  inline static NativeClass<T> get(HSQUIRRELVM vm, SQInteger index)
  {
    throw Exception("I'm the best at space?");
  }
  inline static void push(HSQUIRRELVM vm, NativeClass<T> value)
  {
    sq_pushobject(vm, value.getHandle());
  }
};

///////////////////////////////////////////////////////////////////////

  } /*namespace sq*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_SQUIRREL_H*/
///////////////////////////////////////////////////////////////////////
