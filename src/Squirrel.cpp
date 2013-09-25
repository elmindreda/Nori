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

#include <wendy/Config.h>
#include <wendy/Core.h>
#include <wendy/Path.h>
#include <wendy/Resource.h>

#include <wendy/Squirrel.h>

#include <sqstdmath.h>
#include <sqstdstring.h>

#include <sstream>
#include <cstring>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace sq
  {

///////////////////////////////////////////////////////////////////////

namespace
{

String escapeString(const SQChar* string)
{
  String result;
  result.reserve(std::strlen(string));

  for (const SQChar* c = string;  *c;  c++)
  {
    if (*c == '\t')
      result += "\\t";
    else if (*c == '\a')
      result += "\\a";
    else if (*c == '\b')
      result += "\\b";
    else if (*c == '\n')
      result += "\\n";
    else if (*c == '\r')
      result += "\\r";
    else if (*c == '\v')
      result += "\\v";
    else if (*c == '\f')
      result += "\\f";
    else
      result += *c;
  }

  return result;
}

void logErrorCallStack(HSQUIRRELVM vm)
{
  std::ostringstream stream;

  const SQChar* error = "Unknown error";

  sq_getlasterror(vm);
  if (sq_gettype(vm, -1) == OT_STRING)
    sq_getstring(vm, -1, &error);
  sq_pop(vm, 2);

  stream << error;

  SQInteger level = 1;
  SQStackInfos si;

  while (SQ_SUCCEEDED(sq_stackinfos(vm, level, &si)))
  {
    if (!si.funcname)
      si.funcname = "UNNAMED";
    if (!si.source)
      si.source = "UNNAMED";

    stream << '\n' << si.source << ':' << si.line << ':' << si.funcname;

    SQInteger index = 0;
    const SQChar* name;

    while ((name = sq_getlocal(vm, level, index)))
    {
      stream << "\n  " << name << " = ";

      switch (sq_gettype(vm, -1))
      {
        case OT_NULL:
        {
          stream << "null";
          break;
        }

        case OT_INTEGER:
        {
          SQInteger value;
          sq_getinteger(vm, -1, &value);
          stream << value;
          break;
        }

        case OT_FLOAT:
        {
          SQFloat value;
          sq_getfloat(vm, -1, &value);
          stream << value;
          break;
        }

        case OT_BOOL:
        {
          SQBool value;
          sq_getbool(vm, -1, &value);
          stream << (value ? "true" : "false");
          break;
        }

        case OT_STRING:
        {
          const SQChar* value;
          sq_getstring(vm, -1, &value);
          stream << '\"' << escapeString(value) << '\"';
          break;
        }

        case OT_TABLE:
        {
          stream << "{ ... }";
          break;
        }

        case OT_ARRAY:
        {
          stream << "[ ... ]";
          break;
        }

        case OT_CLOSURE:
        {
          stream << "function() [Squirrel]";
          break;
        }

        case OT_NATIVECLOSURE:
        {
          stream << "function() [C++]";
          break;
        }

        case OT_GENERATOR:
        {
          stream << "generator()";
          break;
        }

        case OT_USERDATA:
        case OT_USERPOINTER:
        {
          SQUserPointer value;
          sq_getuserpointer(vm, -1, &value);
          stream << value;
          break;
        }

        case OT_THREAD:
        {
          stream << "thread";
          break;
        }

        case OT_FUNCPROTO:
        {
          stream << "whathuh";
          break;
        }

        case OT_CLASS:
        {
          stream << "class";
          break;
        }

        case OT_INSTANCE:
        {
          stream << "instance";
          break;
        }

        case OT_WEAKREF:
        {
          stream << "weakref";
          break;
        }

        case OT_OUTER:
        {
          stream << "huhwhat";
          break;
        }
      }

      sq_poptop(vm);
      index++;
    }

    level++;
  }

  logError("%s", stream.str().c_str());
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

VM::VM(ResourceCache& cache):
  m_cache(cache),
  m_vm(nullptr)
{
  m_vm = sq_open(1024);

  sq_setforeignptr(m_vm, this);
  sq_setprintfunc(m_vm, onLogMessage, onLogError);
  sq_setcompilererrorhandler(m_vm, onCompilerError);

  sq_pushroottable(m_vm);
  sqstd_register_mathlib(m_vm);
  sqstd_register_stringlib(m_vm);
  sq_newclosure(m_vm, onRuntimeError, 0);
  sq_seterrorhandler(m_vm);
  sq_poptop(m_vm);
}

VM::~VM()
{
  if (m_vm)
    sq_close(m_vm);
}

bool VM::execute(const char* name)
{
  const Path path = m_cache.findFile(name);
  if (path.isEmpty())
  {
    logError("Failed to find script %s", name);
    return false;
  }

  std::ifstream stream(path.asString().c_str());
  if (stream.fail())
  {
    logError("Failed to open script %s", name);
    return nullptr;
  }

  stream.seekg(0, std::ios::end);

  String text;
  text.resize((uint) stream.tellg());

  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());
  stream.close();

  return execute(name, text.c_str());
}

bool VM::execute(const char* name, const char* text)
{
  if (SQ_FAILED(sq_compilebuffer(m_vm, text, std::strlen(text), name, true)))
    return false;

  sq_pushroottable(m_vm);

  const SQRESULT result = sq_call(m_vm, 1, false, true);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

VM::operator HSQUIRRELVM ()
{
  return m_vm;
}

void* VM::foreignPointer() const
{
  return sq_getforeignptr(m_vm);
}

void VM::setForeignPointer(void* newValue)
{
  sq_setforeignptr(m_vm, newValue);
}

Table VM::rootTable()
{
  sq_pushroottable(m_vm);
  Table table(m_vm, -1);
  sq_poptop(m_vm);

  return table;
}

Table VM::constTable()
{
  sq_pushconsttable(m_vm);
  Table table(m_vm, -1);
  sq_poptop(m_vm);

  return table;
}

Table VM::registryTable()
{
  sq_pushregistrytable(m_vm);
  Table table(m_vm, -1);
  sq_poptop(m_vm);

  return table;
}

ResourceCache& VM::cache() const
{
  return m_cache;
}

void VM::onLogMessage(HSQUIRRELVM vm, const SQChar* format, ...)
{
  va_list vl;
  char* message;
  int result;

  va_start(vl, format);
  result = vasprintf(&message, format, vl);
  va_end(vl);

  if (result < 0)
    return;

  log("%s", message);

  std::free(message);
}

void VM::onLogError(HSQUIRRELVM vm, const SQChar* format, ...)
{
  va_list vl;
  char* message;
  int result;

  va_start(vl, format);
  result = vasprintf(&message, format, vl);
  va_end(vl);

  if (result < 0)
    return;

  logError("%s", message);

  std::free(message);
}

void VM::onCompilerError(HSQUIRRELVM vm,
                         const SQChar* description,
                         const SQChar* source,
                         SQInteger line,
                         SQInteger column)
{
  logError("%s:%i:%i: %s", source, int(line), int(column), description);
}

SQInteger VM::onRuntimeError(HSQUIRRELVM vm)
{
  if (sq_gettop(vm) >= 1)
    logErrorCallStack(vm);

  return 0;
}

///////////////////////////////////////////////////////////////////////

Object::Object():
  m_vm(nullptr)
{
  sq_resetobject(&m_handle);
}

Object::Object(HSQUIRRELVM vm, SQInteger index):
  m_vm(vm)
{
  if (!m_vm)
    panic("VM handle cannot be NULL when constructing from stack");

  sq_resetobject(&m_handle);
  sq_getstackobj(m_vm, -1, &m_handle);
  sq_addref(m_vm, &m_handle);
}

Object::Object(const Object& source):
  m_vm(source.m_vm),
  m_handle(source.m_handle)
{
  sq_addref(m_vm, &m_handle);
}

Object::~Object()
{
  if (m_vm)
    sq_release(m_vm, &m_handle);
}

Object Object::clone() const
{
  if (!m_vm)
    return Object();

  sq_pushobject(m_vm, m_handle);
  sq_clone(m_vm, -1);
  Object clone(m_vm, -1);
  sq_pop(m_vm, 2);
  return clone;
}

Object& Object::operator = (const Object& source)
{
  HSQOBJECT next = source.m_handle;
  sq_addref(m_vm, &next);
  sq_release(m_vm, &m_handle);
  m_handle = next;
  m_vm = source.m_vm;
  return *this;
}

bool Object::isNull() const
{
  return sq_isnull(m_handle);
}

bool Object::isArray() const
{
  return sq_isarray(m_handle);
}

bool Object::isTable() const
{
  return sq_istable(m_handle);
}

bool Object::isClass() const
{
  return sq_isclass(m_handle);
}

bool Object::isInstance() const
{
  return sq_isinstance(m_handle);
}

String Object::asString() const
{
  if (!m_vm)
    return String();

  sq_pushobject(m_vm, m_handle);
  sq_tostring(m_vm, -1);

  String result = Value<String>::get(m_vm, -1);

  sq_pop(m_vm, 2);
  return result;
}

SQObjectType Object::type() const
{
  if (!m_vm)
    return OT_NULL;

  sq_pushobject(m_vm, m_handle);
  SQObjectType type = sq_gettype(m_vm, -1);
  sq_poptop(m_vm);
  return type;
}

HSQOBJECT Object::handle()
{
  return m_handle;
}

HSQUIRRELVM Object::VM() const
{
  return m_vm;
}

Object::Object(HSQUIRRELVM vm):
  m_vm(vm)
{
  sq_resetobject(&m_handle);
}

bool Object::removeSlot(const char* name)
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);
  sq_pushstring(m_vm, name, -1);

  const SQRESULT result = sq_deleteslot(m_vm, -2, false);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

bool Object::addFunction(const char* name,
                         void* pointer,
                         size_t pointerSize,
                         SQFUNCTION function,
                         bool staticMember)
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);
  sq_pushstring(m_vm, name, -1);

  std::memcpy(sq_newuserdata(m_vm, pointerSize), pointer, pointerSize);
  sq_newclosure(m_vm, function, 1);
  sq_setnativeclosurename(m_vm, -1, name);

  const SQRESULT result = sq_newslot(m_vm, -3, staticMember);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

bool Object::clear()
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);

  const SQRESULT result = sq_clear(m_vm, -1);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

bool Object::call(const char* name)
{
  sq_pushobject(m_vm, m_handle);
  sq_pushstring(m_vm, name, -1);
  if (SQ_FAILED(sq_get(m_vm, -2)))
  {
    sq_poptop(m_vm);
    return false;
  }

  sq_pushobject(m_vm, m_handle);

  const SQRESULT result = sq_call(m_vm, 1, false, true);

  sq_pop(m_vm, 2);
  return SQ_SUCCEEDED(result);
}

SQInteger Object::size() const
{
  if (isNull())
    return 0;

  sq_pushobject(m_vm, m_handle);
  SQInteger size = sq_getsize(m_vm, -1);
  sq_poptop(m_vm);
  return size;
}

///////////////////////////////////////////////////////////////////////

Array::Array()
{
}

Array::Array(HSQUIRRELVM vm):
  Object(vm)
{
  sq_newarray(m_vm, 0);
  sq_getstackobj(m_vm, -1, &m_handle);
  sq_addref(m_vm, &m_handle);
  sq_poptop(m_vm);
}

Array::Array(const Object& source):
  Object(source)
{
  if (!isNull() && !isArray())
    panic("Object is not an array");
}

Array::Array(HSQUIRRELVM vm, SQInteger index):
  Object(vm, index)
{
  if (!isArray())
    panic("Object is not an array");
}

bool Array::remove(SQInteger index)
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);

  const SQRESULT result = sq_arrayremove(m_vm, -1, index);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

bool Array::pop()
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);

  const SQRESULT result = sq_arraypop(m_vm, -1, false);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

bool Array::resize(SQInteger newSize)
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);

  const SQRESULT result = sq_arrayresize(m_vm, -1, newSize);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

bool Array::reverse()
{
  if (isNull())
    return false;

  sq_pushobject(m_vm, m_handle);

  const SQRESULT result = sq_arrayreverse(m_vm, -1);

  sq_poptop(m_vm);
  return SQ_SUCCEEDED(result);
}

Object Array::operator [] (SQInteger index) const
{
  if (isNull())
    panic("Cannot retrieve slot from null");

  sq_pushobject(m_vm, m_handle);
  sq_pushinteger(m_vm, index);

  if (SQ_FAILED(sq_get(m_vm, -2)))
  {
    sq_poptop(m_vm);
    panic("No array element at index");
  }

  Object result(m_vm, -1);
  sq_pop(m_vm, 2);
  return result;
}

///////////////////////////////////////////////////////////////////////

Table::Table()
{
}

Table::Table(HSQUIRRELVM vm):
  Object(vm)
{
  sq_newtable(m_vm);
  sq_getstackobj(m_vm, -1, &m_handle);
  sq_addref(m_vm, &m_handle);
  sq_poptop(m_vm);
}

Table::Table(const Object& source):
  Object(source)
{
  if (!isNull() && !isTable())
    panic("Object is not a table");
}

Table::Table(HSQUIRRELVM vm, SQInteger index):
  Object(vm, index)
{
  if (!isTable())
    panic("Object is not a table");
}

///////////////////////////////////////////////////////////////////////

Class::Class()
{
}

Class::Class(HSQUIRRELVM vm):
  Object(vm)
{
  sq_newtable(m_vm);
  sq_getstackobj(m_vm, -1, &m_handle);
  sq_addref(m_vm, &m_handle);
  sq_poptop(m_vm);
}

Class::Class(const Object& source):
  Object(source)
{
  if (!isNull() && !isClass())
    panic("Object is not a class");
}

Class::Class(HSQUIRRELVM vm, SQInteger index):
  Object(vm, index)
{
  if (!isClass())
    panic("Object is not a class");
}

Instance Class::createInstance() const
{
  if (isNull())
    panic("Cannot create instance of null");

  sq_pushobject(m_vm, m_handle);
  sq_createinstance(m_vm, -1);

  Instance result(m_vm, -1);

  sq_pop(m_vm, 2);
  return result;
}

Table Class::attributes()
{
  if (isNull())
    return Table();

  sq_pushobject(m_vm, m_handle);
  sq_pushnull(m_vm);
  sq_getattributes(m_vm, -2);

  Table result(m_vm, -1);

  sq_pop(m_vm, 2);
  return result;
}

Table Class::memberAttributes(const char* name)
{
  if (isNull())
    return Table();

  sq_pushobject(m_vm, m_handle);
  sq_pushstring(m_vm, name, -1);
  sq_getattributes(m_vm, -2);

  Table result(m_vm, -1);

  sq_pop(m_vm, 2);
  return result;
}

///////////////////////////////////////////////////////////////////////

Instance::Instance()
{
}

Instance::Instance(const Object& source):
  Object(source)
{
  if (!isNull() && !isInstance())
    panic("Object is not an instance");
}

Instance::Instance(HSQUIRRELVM vm, SQInteger index):
  Object(vm, index)
{
  if (!isInstance())
    panic("Object is not an instance");
}

Class Instance::class_() const
{
  sq_pushobject(m_vm, m_handle);
  sq_getclass(m_vm, -1);

  Class result(m_vm, -1);

  sq_pop(m_vm, 2);
  return result;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace sq*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
