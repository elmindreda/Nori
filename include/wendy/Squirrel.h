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
 *  These classes provide an easy way to bind C++ to Squirrel.  The design is
 *  based on Sqrat and borrows many clever tricks from it, but it is smaller
 *  and lacks some of the features of Sqrat.
 */

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace sq
  {

///////////////////////////////////////////////////////////////////////

class Object;
class Table;
class Instance;

///////////////////////////////////////////////////////////////////////

/*! @brief Squirrel %VM instance.
 *  @ingroup squirrel
 */
class VM
{
  friend class Object;
public:
  VM(ResourceCache& cache);
  ~VM();
  bool execute(const Path& path);
  bool execute(const char* name, const char* text);
  operator HSQUIRRELVM ();
  void* getForeignPointer() const;
  void setForeignPointer(void* newValue);
  Table getRootTable();
  Table getConstTable();
  Table getRegistryTable();
  ResourceCache& getCache() const;
private:
  WENDY_CHECKFORMAT(2, static void onLogMessage(HSQUIRRELVM vm, const SQChar* format, ...));
  WENDY_CHECKFORMAT(2, static void onLogError(HSQUIRRELVM vm, const SQChar* format, ...));
  static void onCompilerError(HSQUIRRELVM vm,
                              const SQChar* description,
                              const SQChar* source,
                              SQInteger line,
                              SQInteger column);
  static SQInteger onRuntimeError(HSQUIRRELVM vm);
  ResourceCache& cache;
  HSQUIRRELVM vm;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Stack value helper.
 *  @ingroup squirrel
 */
template <typename T>
class Value
{
public:
  static T get(HSQUIRRELVM vm, SQInteger index);
  static void push(HSQUIRRELVM vm, T value);
};

///////////////////////////////////////////////////////////////////////

/*! @brief Squirrel object reference.
 *  @ingroup squirrel
 */
class Object
{
public:
  Object();
  Object(HSQUIRRELVM vm, SQInteger index);
  Object(const Object& source);
  ~Object();
  template <typename T>
  T cast() const;
  Object clone() const;
  Object& operator = (const Object& source);
  bool isNull() const;
  bool isArray() const;
  bool isTable() const;
  bool isClass() const;
  bool isInstance() const;
  String asString() const;
  SQObjectType getType() const;
  HSQOBJECT getHandle();
  HSQUIRRELVM getVM() const;
protected:
  Object(HSQUIRRELVM vm);
  template <typename T>
  bool addSlot(const char* name, T value);
  template <typename T>
  bool addStaticSlot(const char* name, T value);
  bool removeSlot(const char* name);
  bool addFunction(const char* name,
                   void* pointer,
                   size_t pointerSize,
                   SQFUNCTION function,
                   bool staticMember = false);
  bool clear();
  bool call(const char* name);
  template <typename A1>
  bool call(const char* name, A1 a1);
  template <typename A1, typename A2>
  bool call(const char* name, A1 a1, A2 a2);
  template <typename R>
  R eval(const char* name);
  template <typename R, typename A1>
  R eval(const char* name, A1 a1);
  template <typename R, typename A1, typename A2>
  R eval(const char* name, A1 a1, A2 a2);
  template <typename T>
  T get(const char* name);
  template <typename T>
  bool set(const char* name, T value);
  SQInteger getSize() const;
  HSQUIRRELVM vm;
  HSQOBJECT handle;
};

template <typename T>
inline T Object::cast() const
{
  sq_pushobject(vm, handle);
  T value = Value<T>::get(vm, -1);
  sq_poptop(vm);
  return value;
}

template <typename T>
inline bool Object::addSlot(const char* name, T value)
{
  if (isNull())
    return false;

  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  Value<T>::push(vm, value);

  const SQRESULT result = sq_newslot(vm, -3, false);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

template <typename T>
inline bool Object::addStaticSlot(const char* name, T value)
{
  if (isNull())
    return false;

  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  Value<T>::push(vm, value);

  const SQRESULT result = sq_newslot(vm, -3, true);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

template <typename A1>
inline bool Object::call(const char* name, A1 a1)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  if (SQ_FAILED(sq_get(vm, -2)))
  {
    sq_poptop(vm);
    return false;
  }

  sq_pushobject(vm, handle);
  Value<A1>::push(vm, a1);

  const SQRESULT result = sq_call(vm, 2, false, true);

  sq_pop(vm, 2);
  return SQ_SUCCEEDED(result);
}

template <typename A1, typename A2>
inline bool Object::call(const char* name, A1 a1, A2 a2)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  if (SQ_FAILED(sq_get(vm, -2)))
  {
    sq_poptop(vm);
    return false;
  }

  sq_pushobject(vm, handle);
  Value<A1>::push(vm, a1);
  Value<A2>::push(vm, a2);

  const SQRESULT result = sq_call(vm, 3, false, true);

  sq_pop(vm, 2);
  return SQ_SUCCEEDED(result);
}

template <typename R>
inline R Object::eval(const char* name)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  if (SQ_FAILED(sq_get(vm, -2)))
  {
    sq_poptop(vm);
    throw Exception("Failed to retrieve closure");
  }

  sq_pushobject(vm, handle);

  if (SQ_FAILED(sq_call(vm, 1, true, true)))
  {
    sq_pop(vm, 2);
    throw Exception("Failed to call closure");
  }

  const R result = Value<R>::get(vm, -1);
  sq_pop(vm, 3);
  return result;
}

template <typename R, typename A1>
inline R Object::eval(const char* name, A1 a1)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  if (SQ_FAILED(sq_get(vm, -2)))
  {
    sq_poptop(vm);
    throw Exception("Failed to retrieve closure");
  }

  sq_pushobject(vm, handle);
  Value<A1>::push(vm, a1);

  if (SQ_FAILED(sq_call(vm, 2, true, true)))
  {
    sq_pop(vm, 2);
    throw Exception("Failed to call closure");
  }

  const R result = Value<R>::get(vm, -1);
  sq_pop(vm, 3);
  return result;
}

template <typename R, typename A1, typename A2>
inline R Object::eval(const char* name, A1 a1, A2 a2)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  if (SQ_FAILED(sq_get(vm, -2)))
  {
    sq_poptop(vm);
    throw Exception("Failed to retrieve closure");
  }

  sq_pushobject(vm, handle);
  Value<A1>::push(vm, a1);
  Value<A2>::push(vm, a2);

  if (SQ_FAILED(sq_call(vm, 3, true, true)))
  {
    sq_pop(vm, 2);
    throw Exception("Failed to call closure");
  }

  const R result = Value<R>::get(vm, -1);
  sq_pop(vm, 3);
  return result;
}

template <typename T>
inline T Object::get(const char* name)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);

  if (SQ_FAILED(sq_get(vm, -2)))
  {
    sq_poptop(vm);
    throw Exception("The requested slot does not exist");
  }

  T result = Value<T>::get(vm, -1);
  sq_pop(vm, 2);

  return result;
}

template <typename T>
inline bool Object::set(const char* name, T value)
{
  if (isNull())
    return false;

  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  Value<T>::push(vm, value);

  const SQRESULT result = sq_set(vm, -3);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

///////////////////////////////////////////////////////////////////////

/*! @brief Squirrel array reference.
 *  @ingroup squirrel
 */
class Array : public Object
{
public:
  Array();
  Array(HSQUIRRELVM vm);
  Array(const Object& source);
  Array(HSQUIRRELVM vm, SQInteger index);
  template <typename T>
  bool insert(SQInteger index, T value);
  bool remove(SQInteger index);
  template <typename T>
  bool push(T value);
  bool pop();
  bool resize(SQInteger newSize);
  bool reverse();
  using Object::clear;
  Object operator [] (SQInteger index) const;
  template <typename T>
  T get(SQInteger index) const;
  template <typename T>
  bool set(SQInteger index, T value);
  using Object::getSize;
};

template <typename T>
inline bool Array::insert(SQInteger index, T value)
{
  if (isNull())
    return false;

  sq_pushobject(vm, handle);
  Value<T>::push(vm, value);

  const SQRESULT result = sq_arrayinsert(vm, -2, index);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

template <typename T>
inline bool Array::push(T value)
{
  if (isNull())
    return false;

  sq_pushobject(vm, handle);
  Value<T>::push(vm, value);

  const SQRESULT result = sq_arrayappend(vm, -2);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

template <typename T>
inline T Array::get(SQInteger index) const
{
  if (isNull())
    panic("Cannot retrieve slot from null");

  sq_pushobject(vm, handle);
  sq_pushinteger(vm, index);

  if (SQ_FAILED(sq_get(vm, -2)))
  {
    sq_poptop(vm);
    throw Exception("No array element at index");
  }

  T result = Value<T>::get(vm, -1);
  sq_pop(vm, 2);
  return result;
}

template <typename T>
inline bool Array::set(SQInteger index, T value)
{
  if (isNull())
    return false;

  sq_pushobject(vm, handle);
  sq_pushinteger(vm, index);
  Value<T>::push(vm, value);

  const SQRESULT result = sq_set(vm, -3);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

///////////////////////////////////////////////////////////////////////

/*! @brief Squirrel table reference.
 *  @ingroup squirrel
 */
class Table : public Object
{
public:
  Table();
  Table(HSQUIRRELVM vm);
  Table(const Object& source);
  Table(HSQUIRRELVM vm, SQInteger index);
  using Object::addSlot;
  using Object::removeSlot;
  using Object::clear;
  using Object::call;
  using Object::eval;
  using Object::get;
  using Object::set;
  using Object::getSize;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Native function wrapping helper.
 *  @ingroup squirrel
 */
template <typename R>
class Function
{
public:
  static SQInteger demarshal0(HSQUIRRELVM vm)
  {
    typedef R (*Function)();

    Function* function;
    sq_getuserdata(vm, -1, (SQUserPointer*) &function, NULL);

    Value<R>::push(vm, (**function)());
    return 1;
  }
  template <typename A1>
  static SQInteger demarshal1(HSQUIRRELVM vm)
  {
    typedef R (*Function)(A1);

    Function* function;
    sq_getuserdata(vm, -1, (SQUserPointer*) &function, NULL);

    Value<R>::push(vm, (**function)(Value<A1>::get(vm, 2)));
    return 1;
  }
  template <typename A1, typename A2>
  static SQInteger demarshal2(HSQUIRRELVM vm)
  {
    typedef R (*Function)(A1,A2);

    Function* function;
    sq_getuserdata(vm, -1, (SQUserPointer*) &function, NULL);

    Value<R>::push(vm, (**function)(Value<A1>::get(vm, 2),
                                    Value<A2>::get(vm, 3)));
    return 1;
  }
};

/*! @brief Native function wrapping helper.
 *  @ingroup squirrel
 */
template <>
class Function<void>
{
public:
  static SQInteger demarshal0(HSQUIRRELVM vm)
  {
    typedef void (*Function)();

    Function* function;
    sq_getuserdata(vm, -1, (SQUserPointer*) &function, NULL);

    (**function)();
    return 0;
  }
  template <typename A1>
  static SQInteger demarshal1(HSQUIRRELVM vm)
  {
    typedef void (*Function)(A1);

    Function* function;
    sq_getuserdata(vm, -1, (SQUserPointer*) &function, NULL);

    (**function)(Value<A1>::get(vm, 2));
    return 0;
  }
  template <typename A1, typename A2>
  static SQInteger demarshal2(HSQUIRRELVM vm)
  {
    typedef void (*Function)(A1,A2);

    Function* function;
    sq_getuserdata(vm, -1, (SQUserPointer*) &function, NULL);

    (**function)(Value<A1>::get(vm, 2),
                 Value<A2>::get(vm, 3));
    return 0;
  }
};

///////////////////////////////////////////////////////////////////////

/*! @brief Native class method wrapping helper.
 *  @ingroup squirrel
 */
template <typename T, typename R>
class Method
{
public:
  static SQInteger demarshal0(HSQUIRRELVM vm)
  {
    typedef R (T::*Method)();

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    Value<R>::push(vm, (instance->**method)());
    return 1;
  }
  static SQInteger demarshal0C(HSQUIRRELVM vm)
  {
    typedef R (T::*Method)() const;

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    Value<R>::push(vm, (instance->**method)());
    return 1;
  }
  template <typename A1>
  static SQInteger demarshal1(HSQUIRRELVM vm)
  {
    typedef R (T::*Method)(A1);

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    Value<R>::push(vm, (instance->**method)(Value<A1>::get(vm, 2)));
    return 1;
  }
  template <typename A1>
  static SQInteger demarshal1C(HSQUIRRELVM vm)
  {
    typedef R (T::*Method)(A1) const;

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    Value<R>::push(vm, (instance->**method)(Value<A1>::get(vm, 2)));
    return 1;
  }
  template <typename A1, typename A2>
  static SQInteger demarshal2(HSQUIRRELVM vm)
  {
    typedef R (T::*Method)(A1,A2);

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    Value<R>::push(vm, (instance->**method)(Value<A1>::get(vm, 2),
                                            Value<A2>::get(vm, 3)));
    return 1;
  }
  template <typename A1, typename A2>
  static SQInteger demarshal2C(HSQUIRRELVM vm)
  {
    typedef R (T::*Method)(A1,A2) const;

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    Value<R>::push(vm, (instance->**method)(Value<A1>::get(vm, 2),
                                            Value<A2>::get(vm, 3)));
    return 1;
  }
};

/*! @brief Native class method wrapping helper.
 *  @ingroup squirrel
 */
template <typename T>
class Method<T, void>
{
public:
  static SQInteger demarshal0(HSQUIRRELVM vm)
  {
    typedef void (T::*Method)();

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    (instance->**method)();
    return 0;
  }
  static SQInteger demarshal0C(HSQUIRRELVM vm)
  {
    typedef void (T::*Method)() const;

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    (instance->**method)();
    return 0;
  }
  template <typename A1>
  static SQInteger demarshal1(HSQUIRRELVM vm)
  {
    typedef void (T::*Method)(A1);

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    (instance->**method)(Value<A1>::get(vm, 2));
    return 0;
  }
  template <typename A1>
  static SQInteger demarshal1C(HSQUIRRELVM vm)
  {
    typedef void (T::*Method)(A1) const;

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    (instance->**method)(Value<A1>::get(vm, 2));
    return 0;
  }
  template <typename A1, typename A2>
  static SQInteger demarshal2(HSQUIRRELVM vm)
  {
    typedef void (T::*Method)(A1,A2);

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    (instance->**method)(Value<A1>::get(vm, 2),
                         Value<A2>::get(vm, 3));
    return 0;
  }
  template <typename A1, typename A2>
  static SQInteger demarshal2C(HSQUIRRELVM vm)
  {
    typedef void (T::*Method)(A1,A2) const;

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, NULL);

    T* instance = NULL;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, NULL);

    (instance->**method)(Value<A1>::get(vm, 2),
                         Value<A2>::get(vm, 3));
    return 0;
  }
};

///////////////////////////////////////////////////////////////////////

/*! @brief Native function wrapping helper.
 *  @ingroup squirrel
 */
template <typename R>
inline SQFUNCTION demarshal(R (*function)())
{
  return &Function<R>::demarshal0;
}

/*! @brief Native function wrapping helper.
 *  @ingroup squirrel
 */
template <typename R, typename A1>
inline SQFUNCTION demarshal(R (*function)(A1))
{
  return &Function<R>::template demarshal1<A1>;
}

/*! @brief Native function wrapping helper.
 *  @ingroup squirrel
 */
template <typename R, typename A1, typename A2>
inline SQFUNCTION demarshal(R (*function)(A1,A2))
{
  return &Function<R>::template demarshal2<A1,A2>;
}

/*! @brief Native class method wrapping helper.
 *  @ingroup squirrel
 */
template <typename T, typename R>
inline SQFUNCTION demarshal(R (T::*method)())
{
  return &Method<T,R>::demarshal0;
}

/*! @brief Native class method wrapping helper.
 *  @ingroup squirrel
 */
template <typename T, typename R>
inline SQFUNCTION demarshal(R (T::*method)() const)
{
  return &Method<T,R>::demarshal0C;
}

/*! @brief Native class method wrapping helper.
 *  @ingroup squirrel
 */
template <typename T, typename R, typename A1>
inline SQFUNCTION demarshal(R (T::*method)(A1))
{
  return &Method<T,R>::template demarshal1<A1>;
}

/*! @brief Native class method wrapping helper.
 *  @ingroup squirrel
 */
template <typename T, typename R, typename A1>
inline SQFUNCTION demarshal(R (T::*method)(A1) const)
{
  return &Method<T,R>::template demarshal1C<A1>;
}

/*! @brief Native class method wrapping helper.
 *  @ingroup squirrel
 */
template <typename T, typename R, typename A1, typename A2>
inline SQFUNCTION demarshal(R (T::*method)(A1,A2))
{
  return &Method<T,R>::template demarshal2<A1,A2>;
}

/*! @brief Native class method wrapping helper.
 *  @ingroup squirrel
 */
template <typename T, typename R, typename A1, typename A2>
inline SQFUNCTION demarshal(R (T::*method)(A1,A2) const)
{
  return &Method<T,R>::template demarshal2C<A1,A2>;
}

///////////////////////////////////////////////////////////////////////

/*! @brief Squirrel class reference.
 *  @ingroup squirrel
 */
class Class : public Object
{
public:
  Class();
  Class(HSQUIRRELVM vm);
  Class(const Object& source);
  Class(HSQUIRRELVM vm, SQInteger index);
  Instance createInstance() const;
  using Object::addSlot;
  using Object::addStaticSlot;
  using Object::removeSlot;
  using Object::clear;
  using Object::call;
  using Object::eval;
  using Object::get;
  using Object::set;
  using Object::getSize;
  Table getAttributes();
  Table getMemberAttributes(const char* name);
};

///////////////////////////////////////////////////////////////////////

/*! @brief Squirrel class instance reference.
 *  @ingroup squirrel
 */
class Instance : public Object
{
public:
  Instance();
  Instance(const Object& source);
  Instance(HSQUIRRELVM vm, SQInteger index);
  using Object::call;
  using Object::eval;
  using Object::get;
  using Object::set;
  Class getClass() const;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
template <typename T>
class SharedClass : public Class
{
public:
  SharedClass(HSQUIRRELVM vm);
  template <typename M>
  bool addMethod(const char* name, M method)
  {
    return addFunction(name, &method, sizeof(method), demarshal(method), false);
  }
  template <typename M>
  bool addStaticMethod(const char* name, M method)
  {
    return addFunction(name, &method, sizeof(method), demarshal(method), true);
  }
  static T* connectNativeInstance(HSQUIRRELVM vm, HSQOBJECT object);
  static void destroyNativeInstance(T* instance);
private:
  static SQInteger constructor(HSQUIRRELVM vm);
  static SQInteger destructor(SQUserPointer pointer, SQInteger size);
};

template <typename T>
inline SharedClass<T>::SharedClass(HSQUIRRELVM initVM)
{
  vm = initVM;

  sq_newclass(vm, false);

  sq_resetobject(&handle);
  sq_getstackobj(vm, -1, &handle);
  sq_addref(vm, &handle);

  sq_pushstring(vm, "constructor", -1);
  sq_newclosure(vm, &constructor, 0);
  sq_newslot(vm, -3, false);

  sq_poptop(vm);
}

template <typename T>
inline T* SharedClass<T>::connectNativeInstance(HSQUIRRELVM vm, HSQOBJECT object)
{
  return new T();
}

template <typename T>
inline void SharedClass<T>::destroyNativeInstance(T* instance)
{
  delete instance;
}

template <typename T>
inline SQInteger SharedClass<T>::constructor(HSQUIRRELVM vm)
{
  HSQOBJECT object;
  sq_resetobject(&object);
  sq_getstackobj(vm, 1, &object);

  T* instance = connectNativeInstance(vm, object);
  sq_setinstanceup(vm, 1, instance);
  sq_setreleasehook(vm, 1, &destructor);

  return 0;
}

template <typename T>
inline SQInteger SharedClass<T>::destructor(SQUserPointer pointer, SQInteger size)
{
  destroyNativeInstance(static_cast<T*>(pointer));
  return 0;
}

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
template <typename T>
class SharedInstance : public Instance
{
public:
  SharedInstance(HSQUIRRELVM vm, SQInteger index);
  using Object::get;
  using Object::set;
  T* getNative();
  SharedClass<T> getClass() const;
};

template <typename T>
inline SharedInstance<T>::SharedInstance(HSQUIRRELVM vm, SQInteger index):
  Instance(vm, index)
{
  // TODO: Check type tag
}

template <typename T>
inline T* SharedInstance<T>::getNative()
{
  sq_pushobject(vm, handle);
  T* result = NULL;
  sq_getinstanceup(vm, -1, (SQUserPointer*) &result, NULL);
  sq_poptop(vm);
  return result;
}

template <typename T>
inline SharedClass<T> SharedInstance<T>::getClass() const
{
  sq_pushobject(vm, handle);
  sq_getclass(vm, -1);
  SharedClass<T> result(vm, -1);
  sq_pop(vm, 2);
  return result;
}

///////////////////////////////////////////////////////////////////////

/*! @brief Stack value helper.
 *  @ingroup squirrel
 */
template <>
class Value<bool>
{
public:
  static bool get(HSQUIRRELVM vm, SQInteger index)
  {
    SQBool value;
    sq_getbool(vm, index, &value);
    return value ? true : false;
  }
  static void push(HSQUIRRELVM vm, bool value)
  {
    sq_pushbool(vm, SQBool(value));
  }
};

/*! @brief Stack value helper.
 *  @ingroup squirrel
 */
template <>
class Value<int>
{
public:
  static int get(HSQUIRRELVM vm, SQInteger index)
  {
    SQInteger value;
    sq_getinteger(vm, index, &value);
    return int(value);
  }
  static void push(HSQUIRRELVM vm, int value)
  {
    sq_pushinteger(vm, SQInteger(value));
  }
};

/*! @brief Stack value helper.
 *  @ingroup squirrel
 */
template <>
class Value<float>
{
public:
  static float get(HSQUIRRELVM vm, SQInteger index)
  {
    SQFloat value;
    sq_getfloat(vm, index, &value);
    return float(value);
  }
  static void push(HSQUIRRELVM vm, float value)
  {
    sq_pushfloat(vm, SQFloat(value));
  }
};

/*! @brief Stack value helper.
 *  @ingroup squirrel
 */
template <>
class Value<const char*>
{
public:
  static const char* get(HSQUIRRELVM vm, SQInteger index)
  {
    const SQChar* value;
    sq_getstring(vm, index, &value);
    return value;
  }
  static void push(HSQUIRRELVM vm, const char* value)
  {
    sq_pushstring(vm, value, -1);
  }
};

/*! @brief Stack value helper.
 *  @ingroup squirrel
 */
template <>
class Value<String>
{
public:
  static String get(HSQUIRRELVM vm, SQInteger index)
  {
    const SQChar* value;
    sq_getstring(vm, index, &value);
    return String(value);
  }
  static void push(HSQUIRRELVM vm, String value)
  {
    sq_pushstring(vm, value.c_str(), -1);
  }
};

/*! @brief Stack value helper.
 *  @ingroup squirrel
 */
template <>
class Value<Object>
{
public:
  static Object get(HSQUIRRELVM vm, SQInteger index)
  {
    return Object(vm, index);
  }
  static void push(HSQUIRRELVM vm, Object value)
  {
    sq_pushobject(vm, value.getHandle());
  }
};

/*! @brief Stack value helper.
 *  @ingroup squirrel
 */
template <>
class Value<Array>
{
public:
  static Array get(HSQUIRRELVM vm, SQInteger index)
  {
    return Array(vm, index);
  }
  static void push(HSQUIRRELVM vm, Array value)
  {
    sq_pushobject(vm, value.getHandle());
  }
};

/*! @brief Stack value helper.
 *  @ingroup squirrel
 */
template <>
class Value<Table>
{
public:
  static Table get(HSQUIRRELVM vm, SQInteger index)
  {
    return Table(vm, index);
  }
  static void push(HSQUIRRELVM vm, Table value)
  {
    sq_pushobject(vm, value.getHandle());
  }
};

/*! @brief Stack value helper.
 *  @ingroup squirrel
 */
template <>
class Value<Class>
{
public:
  static Class get(HSQUIRRELVM vm, SQInteger index)
  {
    return Class(vm, index);
  }
  static void push(HSQUIRRELVM vm, Class value)
  {
    sq_pushobject(vm, value.getHandle());
  }
};

/*! @brief Stack value helper.
 *  @ingroup squirrel
 */
template <>
class Value<Instance>
{
public:
  static Instance get(HSQUIRRELVM vm, SQInteger index)
  {
    return Instance(vm, index);
  }
  static void push(HSQUIRRELVM vm, Instance value)
  {
    sq_pushobject(vm, value.getHandle());
  }
};

/*! @brief Stack value helper.
 *  @ingroup squirrel
 */
template <typename T>
class Value<SharedClass<T> >
{
public:
  static SharedClass<T> get(HSQUIRRELVM vm, SQInteger index)
  {
    return SharedClass<T>(vm, index);
  }
  static void push(HSQUIRRELVM vm, SharedClass<T> value)
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
