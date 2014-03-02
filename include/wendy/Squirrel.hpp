///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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
#ifndef WENDY_SQUIRREL_HPP
#define WENDY_SQUIRREL_HPP
///////////////////////////////////////////////////////////////////////

#include <squirrel.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class SqObject;
class SqTable;
class SqInstance;

///////////////////////////////////////////////////////////////////////

/*! @brief Squirrel %VM instance.
 *  @ingroup squirrel
 */
class SqVM
{
  friend class SqObject;
public:
  SqVM(ResourceCache& cache);
  ~SqVM();
  bool execute(const char* name);
  bool execute(const char* name, const char* text);
  operator HSQUIRRELVM ();
  void* foreignPointer() const;
  void setForeignPointer(void* newValue);
  SqTable rootTable();
  SqTable constTable();
  SqTable registryTable();
  ResourceCache& cache() const;
private:
  WENDY_CHECKFORMAT(2, static void onLogMessage(HSQUIRRELVM vm, const SQChar* format, ...));
  WENDY_CHECKFORMAT(2, static void onLogError(HSQUIRRELVM vm, const SQChar* format, ...));
  static void onCompilerError(HSQUIRRELVM vm,
                              const SQChar* description,
                              const SQChar* source,
                              SQInteger line,
                              SQInteger column);
  static SQInteger onRuntimeError(HSQUIRRELVM vm);
  ResourceCache& m_cache;
  HSQUIRRELVM m_vm;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Stack value helper.
 *  @ingroup squirrel
 */
class SqValue
{
public:
  template <typename T>
  static T get(HSQUIRRELVM vm, SQInteger index);
  template <typename T>
  static void push(HSQUIRRELVM vm, T value);
  template <typename T, typename... A>
  static void push(HSQUIRRELVM vm, T value, A... args)
  {
    push(vm, value);
    push(vm, args...);
  }
};

///////////////////////////////////////////////////////////////////////

/*! @brief Squirrel object reference.
 *  @ingroup squirrel
 */
class SqObject
{
public:
  SqObject();
  SqObject(HSQUIRRELVM vm, SQInteger index);
  SqObject(const SqObject& source);
  SqObject(SqObject&& source);
  ~SqObject();
  template <typename T>
  T as() const;
  SqObject clone() const;
  SqObject& operator = (const SqObject& source);
  bool isNull() const { return sq_isnull(m_handle); }
  bool isArray() const { return sq_isarray(m_handle); }
  bool isTable() const { return sq_istable(m_handle); }
  bool isClass() const { return sq_isclass(m_handle); }
  bool isInstance() const { return sq_isinstance(m_handle); }
  String asString() const;
  SQObjectType type() const;
  HSQOBJECT handle() { return m_handle; }
  HSQUIRRELVM vm() const { return m_vm; }
protected:
  SqObject(HSQUIRRELVM vm);
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
  template <typename... A>
  bool call(const char* name, A... args);
  template <typename R, typename... A>
  R eval(const char* name, A... args);
  template <typename T>
  T get(const char* name);
  template <typename T>
  bool set(const char* name, T value);
  SQInteger size() const;
  HSQUIRRELVM m_vm;
  HSQOBJECT m_handle;
};

template <typename T>
inline T SqObject::as() const
{
  sq_pushobject(m_vm, m_handle);
  T value = SqValue::get<T>(m_vm, -1);
  sq_poptop(m_vm);
  return value;
}

template <typename T>
inline bool SqObject::addSlot(const char* name, T value)
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);
  sq_pushstring(m_vm, name, -1);
  SqValue::push(m_vm, value);

  const SQRESULT result = sq_newslot(m_vm, -3, false);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

template <typename T>
inline bool SqObject::addStaticSlot(const char* name, T value)
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);
  sq_pushstring(m_vm, name, -1);
  SqValue::push(m_vm, value);

  const SQRESULT result = sq_newslot(m_vm, -3, true);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

template <typename... A>
inline bool SqObject::call(const char* name, A... args)
{
  sq_pushobject(m_vm, m_handle);
  sq_pushstring(m_vm, name, -1);
  if (SQ_FAILED(sq_get(m_vm, -2)))
  {
    sq_poptop(m_vm);
    return false;
  }

  sq_pushobject(m_vm, m_handle);
  SqValue::push(m_vm, args...);

  const SQRESULT result = sq_call(m_vm, sizeof...(args) + 1, false, true);

  sq_pop(m_vm, 2);
  return SQ_SUCCEEDED(result);
}

template <typename R, typename... A>
inline R SqObject::eval(const char* name, A... args)
{
  sq_pushobject(m_vm, m_handle);
  sq_pushstring(m_vm, name, -1);
  if (SQ_FAILED(sq_get(m_vm, -2)))
  {
    sq_poptop(m_vm);
    throw Exception("Failed to retrieve closure");
  }

  sq_pushobject(m_vm, m_handle);
  SqValue::push(m_vm, args...);

  if (SQ_FAILED(sq_call(m_vm, sizeof...(args) + 1, true, true)))
  {
    sq_pop(m_vm, 2);
    throw Exception("Failed to call closure");
  }

  const R result = SqValue::get<R>(m_vm, -1);
  sq_pop(m_vm, 3);
  return result;
}

template <typename T>
inline T SqObject::get(const char* name)
{
  sq_pushobject(m_vm, m_handle);
  sq_pushstring(m_vm, name, -1);

  if (SQ_FAILED(sq_get(m_vm, -2)))
  {
    sq_poptop(m_vm);
    throw Exception("The requested slot does not exist");
  }

  T result = SqValue::get<T>(m_vm, -1);
  sq_pop(m_vm, 2);

  return result;
}

template <typename T>
inline bool SqObject::set(const char* name, T value)
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);
  sq_pushstring(m_vm, name, -1);
  SqValue::push(m_vm, value);

  const SQRESULT result = sq_set(m_vm, -3);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

///////////////////////////////////////////////////////////////////////

/*! @brief Squirrel array reference.
 *  @ingroup squirrel
 */
class SqArray : public SqObject
{
public:
  SqArray() { }
  SqArray(HSQUIRRELVM vm);
  SqArray(const SqObject& source);
  SqArray(HSQUIRRELVM vm, SQInteger index);
  template <typename T>
  bool insert(SQInteger index, T value);
  bool remove(SQInteger index);
  template <typename T>
  bool push(T value);
  bool pop();
  bool resize(SQInteger newSize);
  bool reverse();
  using SqObject::clear;
  SqObject operator [] (SQInteger index) const;
  template <typename T>
  T get(SQInteger index) const;
  template <typename T>
  bool set(SQInteger index, T value);
  using SqObject::size;
};

template <typename T>
inline bool SqArray::insert(SQInteger index, T value)
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);
  SqValue::push(m_vm, value);

  const SQRESULT result = sq_arrayinsert(m_vm, -2, index);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

template <typename T>
inline bool SqArray::push(T value)
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);
  SqValue::push(m_vm, value);

  const SQRESULT result = sq_arrayappend(m_vm, -2);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

template <typename T>
inline T SqArray::get(SQInteger index) const
{
  if (isNull())
    panic("Cannot retrieve slot from null");

  sq_pushobject(m_vm, m_handle);
  sq_pushinteger(m_vm, index);

  if (SQ_FAILED(sq_get(m_vm, -2)))
  {
    sq_poptop(m_vm);
    throw Exception("No array element at index");
  }

  T result = SqValue::get<T>(m_vm, -1);
  sq_pop(m_vm, 2);
  return result;
}

template <typename T>
inline bool SqArray::set(SQInteger index, T value)
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);
  sq_pushinteger(m_vm, index);
  SqValue::push(m_vm, value);

  const SQRESULT result = sq_set(m_vm, -3);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

///////////////////////////////////////////////////////////////////////

/*! @brief Squirrel table reference.
 *  @ingroup squirrel
 */
class SqTable : public SqObject
{
public:
  SqTable() { }
  SqTable(HSQUIRRELVM vm);
  SqTable(const SqObject& source);
  SqTable(HSQUIRRELVM vm, SQInteger index);
  using SqObject::addSlot;
  using SqObject::removeSlot;
  using SqObject::clear;
  using SqObject::call;
  using SqObject::eval;
  using SqObject::get;
  using SqObject::set;
  using SqObject::size;
  static SqTable rootTable(HSQUIRRELVM vm);
  static SqTable constTable(HSQUIRRELVM vm);
};

///////////////////////////////////////////////////////////////////////

/*! @brief Native function wrapping helper.
 *  @ingroup squirrel
 */
template <typename R>
class SqFunction
{
public:
  static SQInteger demarshal(HSQUIRRELVM vm)
  {
    typedef R (*Function)();

    Function* function;
    sq_getuserdata(vm, -1, (SQUserPointer*) &function, nullptr);

    SqValue::push(vm, (**function)());
    return 1;
  }
  template <typename A1>
  static SQInteger demarshal(HSQUIRRELVM vm)
  {
    typedef R (*Function)(A1);

    Function* function;
    sq_getuserdata(vm, -1, (SQUserPointer*) &function, nullptr);

    SqValue::push(vm, (**function)(SqValue::get<A1>(vm, 1)));
    return 1;
  }
  template <typename A1, typename A2>
  static SQInteger demarshal(HSQUIRRELVM vm)
  {
    typedef R (*Function)(A1,A2);

    Function* function;
    sq_getuserdata(vm, -1, (SQUserPointer*) &function, nullptr);

    SqValue::push(vm, (**function)(SqValue::get<A1>(vm, 1),
                                   SqValue::get<A2>(vm, 2)));
    return 1;
  }
};

/*! @brief Native function wrapping helper.
 *  @ingroup squirrel
 */
template <>
class SqFunction<void>
{
public:
  static SQInteger demarshal(HSQUIRRELVM vm)
  {
    typedef void (*Function)();

    Function* function;
    sq_getuserdata(vm, -1, (SQUserPointer*) &function, nullptr);

    (**function)();
    return 0;
  }
  template <typename A1>
  static SQInteger demarshal(HSQUIRRELVM vm)
  {
    typedef void (*Function)(A1);

    Function* function;
    sq_getuserdata(vm, -1, (SQUserPointer*) &function, nullptr);

    (**function)(SqValue::get<A1>(vm, 1));
    return 0;
  }
  template <typename A1, typename A2>
  static SQInteger demarshal(HSQUIRRELVM vm)
  {
    typedef void (*Function)(A1,A2);

    Function* function;
    sq_getuserdata(vm, -1, (SQUserPointer*) &function, nullptr);

    (**function)(SqValue::get<A1>(vm, 1),
                 SqValue::get<A2>(vm, 2));
    return 0;
  }
};

///////////////////////////////////////////////////////////////////////

/*! @brief Native class method wrapping helper.
 *  @ingroup squirrel
 */
template <typename T, typename R>
class SqMethod
{
public:
  static SQInteger demarshal(HSQUIRRELVM vm)
  {
    typedef R (T::*Method)();

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, nullptr);

    T* instance = nullptr;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, nullptr);

    SqValue::push(vm, (instance->**method)());
    return 1;
  }
  static SQInteger demarshalC(HSQUIRRELVM vm)
  {
    typedef R (T::*Method)() const;

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, nullptr);

    T* instance = nullptr;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, nullptr);

    SqValue::push(vm, (instance->**method)());
    return 1;
  }
  template <typename A1>
  static SQInteger demarshal(HSQUIRRELVM vm)
  {
    typedef R (T::*Method)(A1);

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, nullptr);

    T* instance = nullptr;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, nullptr);

    SqValue::push(vm, (instance->**method)(SqValue::get<A1>(vm, 2)));
    return 1;
  }
  template <typename A1>
  static SQInteger demarshalC(HSQUIRRELVM vm)
  {
    typedef R (T::*Method)(A1) const;

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, nullptr);

    T* instance = nullptr;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, nullptr);

    SqValue::push(vm, (instance->**method)(SqValue::get<A1>(vm, 2)));
    return 1;
  }
  template <typename A1, typename A2>
  static SQInteger demarshal(HSQUIRRELVM vm)
  {
    typedef R (T::*Method)(A1,A2);

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, nullptr);

    T* instance = nullptr;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, nullptr);

    SqValue::push(vm, (instance->**method)(SqValue::get<A1>(vm, 2),
                                           SqValue::get<A2>(vm, 3)));
    return 1;
  }
  template <typename A1, typename A2>
  static SQInteger demarshalC(HSQUIRRELVM vm)
  {
    typedef R (T::*Method)(A1,A2) const;

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, nullptr);

    T* instance = nullptr;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, nullptr);

    SqValue::push(vm, (instance->**method)(SqValue::get<A1>(vm, 2),
                                           SqValue::get<A2>(vm, 3)));
    return 1;
  }
};

/*! @brief Native class method wrapping helper.
 *  @ingroup squirrel
 */
template <typename T>
class SqMethod<T, void>
{
public:
  static SQInteger demarshal(HSQUIRRELVM vm)
  {
    typedef void (T::*Method)();

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, nullptr);

    T* instance = nullptr;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, nullptr);

    (instance->**method)();
    return 0;
  }
  static SQInteger demarshalC(HSQUIRRELVM vm)
  {
    typedef void (T::*Method)() const;

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, nullptr);

    T* instance = nullptr;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, nullptr);

    (instance->**method)();
    return 0;
  }
  template <typename A1>
  static SQInteger demarshal(HSQUIRRELVM vm)
  {
    typedef void (T::*Method)(A1);

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, nullptr);

    T* instance = nullptr;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, nullptr);

    (instance->**method)(SqValue::get<A1>(vm, 2));
    return 0;
  }
  template <typename A1>
  static SQInteger demarshalC(HSQUIRRELVM vm)
  {
    typedef void (T::*Method)(A1) const;

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, nullptr);

    T* instance = nullptr;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, nullptr);

    (instance->**method)(SqValue::get<A1>(vm, 2));
    return 0;
  }
  template <typename A1, typename A2>
  static SQInteger demarshal(HSQUIRRELVM vm)
  {
    typedef void (T::*Method)(A1,A2);

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, nullptr);

    T* instance = nullptr;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, nullptr);

    (instance->**method)(SqValue::get<A1>(vm, 2),
                         SqValue::get<A2>(vm, 3));
    return 0;
  }
  template <typename A1, typename A2>
  static SQInteger demarshalC(HSQUIRRELVM vm)
  {
    typedef void (T::*Method)(A1,A2) const;

    Method* method;
    sq_getuserdata(vm, -1, (SQUserPointer*) &method, nullptr);

    T* instance = nullptr;
    sq_getinstanceup(vm, 1, (SQUserPointer*) &instance, nullptr);

    (instance->**method)(SqValue::get<A1>(vm, 2),
                         SqValue::get<A2>(vm, 3));
    return 0;
  }
};

///////////////////////////////////////////////////////////////////////

/*! @brief Native function wrapping helper.
 *  @ingroup squirrel
 */
template <typename R, typename... A>
inline SQFUNCTION demarshal(R (*function)(A...))
{
  return &SqFunction<R>::template demarshal<A...>;
}

/*! @brief Native class method wrapping helper.
 *  @ingroup squirrel
 */
template <typename T, typename R, typename... A>
inline SQFUNCTION demarshal(R (T::*method)(A...))
{
  return &SqMethod<T,R>::template demarshal<A...>;
}

/*! @brief Native class method wrapping helper.
 *  @ingroup squirrel
 */
template <typename T, typename R, typename... A>
inline SQFUNCTION demarshal(R (T::*method)(A...) const)
{
  return &SqMethod<T,R>::template demarshalC<A...>;
}

///////////////////////////////////////////////////////////////////////

/*! @brief Squirrel class reference.
 *  @ingroup squirrel
 */
class SqClass : public SqObject
{
public:
  SqClass() { }
  SqClass(HSQUIRRELVM vm);
  SqClass(const SqObject& source);
  SqClass(HSQUIRRELVM vm, SQInteger index);
  SqInstance createInstance() const;
  using SqObject::addSlot;
  using SqObject::addStaticSlot;
  using SqObject::removeSlot;
  using SqObject::clear;
  using SqObject::call;
  using SqObject::eval;
  using SqObject::get;
  using SqObject::set;
  using SqObject::size;
  SqTable attributes();
  SqTable memberAttributes(const char* name);
};

///////////////////////////////////////////////////////////////////////

/*! @brief Squirrel class instance reference.
 *  @ingroup squirrel
 */
class SqInstance : public SqObject
{
public:
  SqInstance() { }
  SqInstance(const SqObject& source);
  SqInstance(HSQUIRRELVM vm, SQInteger index);
  using SqObject::call;
  using SqObject::eval;
  using SqObject::get;
  using SqObject::set;
  void* pointer();
  SqClass class_() const;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
template <typename T>
class SqDataClass : public SqClass
{
public:
  SqDataClass(HSQUIRRELVM vm);
  SqDataClass(HSQUIRRELVM vm, SQInteger index);
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
private:
  static SQInteger constructor(HSQUIRRELVM vm);
  static SQInteger destructor(SQUserPointer pointer, SQInteger size);
  static int m_tag;
};

template <typename T>
inline SqDataClass<T>::SqDataClass(HSQUIRRELVM vm)
{
  m_vm = vm;

  sq_newclass(m_vm, false);

  sq_resetobject(&m_handle);
  sq_getstackobj(m_vm, -1, &m_handle);
  sq_setclassudsize(m_vm, -1, sizeof(T));
  sq_settypetag(m_vm, -1, &m_tag);
  sq_addref(m_vm, &m_handle);

  sq_pushstring(m_vm, "constructor", -1);
  sq_newclosure(m_vm, &constructor, 0);
  sq_newslot(m_vm, -3, false);

  sq_poptop(m_vm);
}

template <typename T>
SqDataClass<T>::SqDataClass(HSQUIRRELVM vm, SQInteger index):
  SqClass(vm, index)
{
  SQUserPointer tag;
  sq_gettypetag(vm, index, &tag);

  if (tag != &m_tag)
    panic("Class type tag mismatch");
}

template <typename T>
inline SQInteger SqDataClass<T>::constructor(HSQUIRRELVM vm)
{
  HSQOBJECT object;
  SQUserPointer pointer;
  sq_resetobject(&object);
  sq_getstackobj(vm, 1, &object);
  sq_setreleasehook(vm, 1, &destructor);
  sq_getinstanceup(vm, 1, (SQUserPointer*) &pointer, nullptr);

  new (pointer) T();
  return 0;
}

template <typename T>
inline SQInteger SqDataClass<T>::destructor(SQUserPointer pointer, SQInteger size)
{
  static_cast<T*>(pointer)->~T();
  return 0;
}

template <typename T>
int SqDataClass<T>::m_tag = 0;

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
template <typename T>
class SqRefClass : public SqClass, public RefBase
{
public:
  SqRefClass(HSQUIRRELVM vm);
  SqRefClass(HSQUIRRELVM vm, SQInteger index);
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
protected:
  static T* createNativeInstance();
private:
  static SQInteger constructor(HSQUIRRELVM vm);
  static SQInteger destructor(SQUserPointer pointer, SQInteger size);
  static int m_tag;
};

template <typename T>
inline SqRefClass<T>::SqRefClass(HSQUIRRELVM vm)
{
  m_vm = vm;

  sq_newclass(m_vm, false);

  sq_resetobject(&m_handle);
  sq_getstackobj(m_vm, -1, &m_handle);
  sq_settypetag(m_vm, -1, &m_tag);
  sq_addref(m_vm, &m_handle);

  sq_pushstring(m_vm, "constructor", -1);
  sq_newclosure(m_vm, &constructor, 0);
  sq_newslot(m_vm, -3, false);

  sq_poptop(m_vm);
}

template <typename T>
SqRefClass<T>::SqRefClass(HSQUIRRELVM vm, SQInteger index):
  SqClass(vm, index)
{
  SQUserPointer tag;
  sq_gettypetag(vm, index, &tag);

  if (tag != &m_tag)
    panic("Class type tag mismatch");
}

template <typename T>
inline SQInteger SqRefClass<T>::constructor(HSQUIRRELVM vm)
{
  HSQOBJECT object;
  SQUserPointer pointer;
  sq_resetobject(&object);
  sq_getstackobj(vm, 1, &object);
  sq_setreleasehook(vm, 1, &destructor);

  T* instance = createNativeInstance();
  increment(instance);
  sq_setinstanceup(vm, 1, (SQUserPointer*) instance);
  return 0;
}

template <typename T>
inline SQInteger SqRefClass<T>::destructor(SQUserPointer pointer, SQInteger size)
{
  T* instance = static_cast<T*>(pointer);
  decrement(instance);
  if (unreferenced(instance))
    delete instance;

  return 0;
}

template <typename T>
int SqRefClass<T>::m_tag = 0;

///////////////////////////////////////////////////////////////////////

/*! @ingroup squirrel
 */
template <typename T>
class SqNativeInstance : public SqInstance
{
public:
  SqNativeInstance(const SqInstance& instance);
  SqNativeInstance(HSQUIRRELVM vm, SQInteger index);
  using SqObject::get;
  using SqObject::set;
  T* native();
};

template <typename T>
inline SqNativeInstance<T>::SqNativeInstance(const SqInstance& instance):
  SqInstance(instance)
{
  // TODO: Check type tag
}

template <typename T>
inline SqNativeInstance<T>::SqNativeInstance(HSQUIRRELVM vm, SQInteger index):
  SqInstance(vm, index)
{
  // TODO: Check type tag
}

template <typename T>
inline T* SqNativeInstance<T>::native()
{
  return static_cast<T*>(pointer());
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_SQUIRREL_HPP*/
///////////////////////////////////////////////////////////////////////
