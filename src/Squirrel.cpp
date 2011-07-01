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

#include <sqstdaux.h>
#include <sqstdmath.h>
#include <sqstdstring.h>

#include <cstring>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace sq
  {

///////////////////////////////////////////////////////////////////////

VM::VM(ResourceIndex& initIndex):
  index(initIndex),
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
  sq_pop(vm, 1);
}

VM::~VM(void)
{
  if (vm)
    sq_close(vm);
}

bool VM::execute(const Path& path)
{
  std::ifstream stream;
  if (!index.openFile(stream, path))
    return false;

  stream.seekg(0, std::ios::end);

  String text;
  text.resize((unsigned int) stream.tellg());

  stream.seekg(0, std::ios::beg);
  stream.read(&text[0], text.size());
  stream.close();

  return execute(path.asString().c_str(), text.c_str());
}

bool VM::execute(const char* name, const char* text)
{
  if (SQ_FAILED(sq_compilebuffer(vm, text, std::strlen(text), name, true)))
    return false;

  sq_pushroottable(vm);
  if (SQ_FAILED(sq_call(vm, 1, false, true)))
    return false;
  sq_pop(vm, 1);

  return true;
}

VM::operator HSQUIRRELVM (void)
{
  return vm;
}

Object VM::getRootTable(void)
{
  sq_pushroottable(vm);
  Object object(vm, -1);
  sq_pop(vm, 1);

  return object;
}

ResourceIndex& VM::getIndex(void) const
{
  return index;
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
  logError("%s:%i:%i: %s", source, line, column, description);
}

SQInteger VM::onRuntimeError(HSQUIRRELVM vm)
{
  if (sq_gettop(vm) >= 1)
    sqstd_printcallstack(vm);

  return 0;
}

///////////////////////////////////////////////////////////////////////

Object::Object(void):
  vm(NULL)
{
  sq_resetobject(&handle);
}

Object::Object(HSQUIRRELVM initVM, SQInteger index):
  vm(initVM)
{
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

Object::~Object(void)
{
  sq_release(vm, &handle);
}

bool Object::removeSlot(const char* name)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);

  const SQRESULT result = sq_deleteslot(vm, -2, false);

  sq_pop(vm, 1);
  return SQ_SUCCEEDED(result);
}

Object& Object::operator = (const Object& source)
{
  sq_release(vm, &handle);
  handle = source.handle;
  vm = source.vm;
  sq_addref(vm, &handle);
  return *this;
}

bool Object::isNull(void) const
{
  return sq_isnull(handle);
}

bool Object::isTable(void) const
{
  return sq_istable(handle);
}

bool Object::isClass(void) const
{
  return sq_isclass(handle);
}

Object Object::getSlot(const char* name)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  if (SQ_FAILED(sq_get(vm, -2)))
  {
    sq_pop(vm, 1);
    return Object();
  }

  Object result(vm, -1);
  sq_pop(vm, 2);

  return result;
}

bool Object::setSlot(const char* name, const Object& value)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);
  sq_pushobject(vm, value.handle);

  const SQRESULT result =  sq_newslot(vm, -3, false);

  sq_pop(vm, 1);
  return SQ_SUCCEEDED(result);
}

HSQOBJECT Object::getHandle(void)
{
  return handle;
}

HSQUIRRELVM Object::getVM(void) const
{
  return vm;
}

void Object::setFunction(const char* name,
                         void* pointer,
                         size_t pointerSize,
                         SQFUNCTION function,
                         bool staticMember)
{
  sq_pushobject(vm, handle);
  sq_pushstring(vm, name, -1);

  std::memcpy(sq_newuserdata(vm, pointerSize), pointer, pointerSize);
  sq_newclosure(vm, function, 1);

  sq_newslot(vm, -3, staticMember);
  sq_pop(vm, 1);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace sq*/
} /*namespace wendy*/

