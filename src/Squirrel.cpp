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

VM::VM(ResourceCache& initCache):
  cache(initCache),
  vm(NULL)
{
  vm = sq_open(1024);

  sq_setforeignptr(vm, this);
  sq_setprintfunc(vm, onLogMessage, onLogError);
  sq_setcompilererrorhandler(vm, onCompilerError);

  sq_pushroottable(vm);
  sqstd_register_mathlib(vm);
  sqstd_register_stringlib(vm);
  sq_newclosure(vm, onRuntimeError, 0);
  sq_seterrorhandler(vm);
  sq_poptop(vm);
}

VM::~VM()
{
  if (vm)
    sq_close(vm);
}

bool VM::execute(const char* name)
{
  const Path path = cache.findFile(name);
  if (path.isEmpty())
  {
    logError("Failed to find script \'%s\'", name);
    return false;
  }

  std::ifstream stream(path.asString().c_str());
  if (stream.fail())
  {
    logError("Failed to open script \'%s\'", name);
    return NULL;
  }

  stream.seekg(0, std::ios::end);

  String text;
  text.resize((unsigned int) stream.tellg());

  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());
  stream.close();

  return execute(name, text.c_str());
}

bool VM::execute(const char* name, const char* text)
{
  if (SQ_FAILED(sq_compilebuffer(vm, text, std::strlen(text), name, true)))
    return false;

  sq_pushroottable(vm);

  const SQRESULT result = sq_call(vm, 1, false, true);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

VM::operator HSQUIRRELVM ()
{
  return vm;
}

void* VM::getForeignPointer() const
{
  return sq_getforeignptr(vm);
}

void VM::setForeignPointer(void* newValue)
{
  sq_setforeignptr(vm, newValue);
}

Table VM::getRootTable()
{
  sq_pushroottable(vm);
  Table table(vm, -1);
  sq_poptop(vm);

  return table;
}

Table VM::getConstTable()
{
  sq_pushconsttable(vm);
  Table table(vm, -1);
  sq_poptop(vm);

  return table;
}

Table VM::getRegistryTable()
{
  sq_pushregistrytable(vm);
  Table table(vm, -1);
  sq_poptop(vm);

  return table;
}

ResourceCache& VM::getCache() const
{
  return cache;
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
  vm(NULL)
{
  sq_resetobject(&handle);
}

Object::Object(HSQUIRRELVM initVM, SQInteger index):
  vm(initVM)
{
  if (!vm)
    panic("VM handle cannot be NULL when constructing from stack");

  sq_resetobject(&handle);
  sq_getstackobj(vm, -1, &handle);
  sq_addref(vm, &handle);
}

Object::Object(const Object& source):
  vm(source.vm),
  handle(source.handle)
{
  sq_addref(vm, &handle);
}

Object::~Object()
{
  if (vm)
    sq_release(vm, &handle);
}

Object Object::clone() const
{
  if (!vm)
    return Object();

  sq_pushobject(vm, handle);
  sq_clone(vm, -1);
  Object clone(vm, -1);
  sq_pop(vm, 2);
  return clone;
}

Object& Object::operator = (const Object& source)
{
  HSQOBJECT next = source.handle;
  sq_addref(vm, &next);
  sq_release(vm, &handle);
  handle = next;
  vm = source.vm;
  return *this;
}

bool Object::isNull() const
{
  return sq_isnull(handle);
}

bool Object::isArray() const
{
  return sq_isarray(handle);
}

bool Object::isTable() const
{
  return sq_istable(handle);
}

bool Object::isClass() const
{
  return sq_isclass(handle);
}

bool Object::isInstance() const
{
  return sq_isinstance(handle);
}

String Object::asString() const
{
  if (!vm)
    return String();

  sq_pushobject(vm, handle);
  sq_tostring(vm, -1);

  String result = Value<String>::get(vm, -1);

  sq_pop(vm, 2);
  return result;
}

SQObjectType Object::getType() const
{
  if (!vm)
    return OT_NULL;

  sq_pushobject(vm, handle);
  SQObjectType type = sq_gettype(vm, -1);
  sq_poptop(vm);
  return type;
}

HSQOBJECT Object::getHandle()
{
  return handle;
}

HSQUIRRELVM Object::getVM() const
{
  return vm;
}

Object::Object(HSQUIRRELVM initVM):
  vm(initVM)
{
  sq_resetobject(&handle);
}

bool Object::removeSlot(const char* name)
{
  if (isNull())
    return false;

  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);

  const SQRESULT result = sq_deleteslot(vm, -2, false);

  sq_poptop(vm);
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

  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);

  std::memcpy(sq_newuserdata(vm, pointerSize), pointer, pointerSize);
  sq_newclosure(vm, function, 1);
  sq_setnativeclosurename(vm, -1, name);

  const SQRESULT result = sq_newslot(vm, -3, staticMember);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

bool Object::clear()
{
  if (isNull())
    return false;

  sq_pushobject(vm, handle);

  const SQRESULT result = sq_clear(vm, -1);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

bool Object::call(const char* name)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  if (SQ_FAILED(sq_get(vm, -2)))
  {
    sq_poptop(vm);
    return false;
  }

  sq_pushobject(vm, handle);

  const SQRESULT result = sq_call(vm, 1, false, true);

  sq_pop(vm, 2);
  return SQ_SUCCEEDED(result);
}

SQInteger Object::getSize() const
{
  if (isNull())
    return 0;

  sq_pushobject(vm, handle);
  SQInteger size = sq_getsize(vm, -1);
  sq_poptop(vm);
  return size;
}

///////////////////////////////////////////////////////////////////////

Array::Array()
{
}

Array::Array(HSQUIRRELVM vm):
  Object(vm)
{
  sq_newarray(vm, 0);
  sq_getstackobj(vm, -1, &handle);
  sq_addref(vm, &handle);
  sq_poptop(vm);
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

  sq_pushobject(vm, handle);

  const SQRESULT result = sq_arrayremove(vm, -1, index);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

bool Array::pop()
{
  if (isNull())
    return false;

  sq_pushobject(vm, handle);

  const SQRESULT result = sq_arraypop(vm, -1, false);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

bool Array::resize(SQInteger newSize)
{
  if (isNull())
    return false;

  sq_pushobject(vm, handle);

  const SQRESULT result = sq_arrayresize(vm, -1, newSize);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

bool Array::reverse()
{
  if (isNull())
    return false;

  sq_pushobject(vm, handle);

  const SQRESULT result = sq_arrayreverse(vm, -1);

  sq_poptop(vm);
  return SQ_SUCCEEDED(result);
}

Object Array::operator [] (SQInteger index) const
{
  if (isNull())
    panic("Cannot retrieve slot from null");

  sq_pushobject(vm, handle);
  sq_pushinteger(vm, index);

  if (SQ_FAILED(sq_get(vm, -2)))
  {
    sq_poptop(vm);
    panic("No array element at index");
  }

  Object result(vm, -1);
  sq_pop(vm, 2);
  return result;
}

///////////////////////////////////////////////////////////////////////

Table::Table()
{
}

Table::Table(HSQUIRRELVM vm):
  Object(vm)
{
  sq_newtable(vm);
  sq_getstackobj(vm, -1, &handle);
  sq_addref(vm, &handle);
  sq_poptop(vm);
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
  sq_newtable(vm);
  sq_getstackobj(vm, -1, &handle);
  sq_addref(vm, &handle);
  sq_poptop(vm);
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

  sq_pushobject(vm, handle);
  sq_createinstance(vm, -1);

  Instance result(vm, -1);

  sq_pop(vm, 2);
  return result;
}

Table Class::getAttributes()
{
  if (isNull())
    return Table();

  sq_pushobject(vm, handle);
  sq_pushnull(vm);
  sq_getattributes(vm, -2);

  Table result(vm, -1);

  sq_pop(vm, 2);
  return result;
}

Table Class::getMemberAttributes(const char* name)
{
  if (isNull())
    return Table();

  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  sq_getattributes(vm, -2);

  Table result(vm, -1);

  sq_pop(vm, 2);
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

Class Instance::getClass() const
{
  sq_pushobject(vm, handle);
  sq_getclass(vm, -1);

  Class result(vm, -1);

  sq_pop(vm, 2);
  return result;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace sq*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
