///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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

#include <moira/Config.h>
#include <moira/Core.h>
#include <moira/Signal.h>
#include <moira/Vector.h>
#include <moira/Color.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLDisplayList.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

DisplayList::~DisplayList(void)
{
  glDeleteLists(listID, 1);
}

void DisplayList::begin(bool immediate)
{
  if (immediate)
    glNewList(listID, GL_COMPILE_AND_EXECUTE);
  else
    glNewList(listID, GL_COMPILE);
}

void DisplayList::end(void)
{
  glEndList();
  
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeWarning("Error during display list recording: %s", gluErrorString(error));
}

void DisplayList::execute(void)
{
  glCallList(listID);
}

GLuint DisplayList::getGLID(void) const
{
  return listID;
}

DisplayList* DisplayList::createInstance(const std::string& name)
{
  Ptr<DisplayList> list = new DisplayList(name);
  if (!list->init())
    return NULL;

  return list.detachObject();
}

DisplayList::DisplayList(const std::string& name):
  Managed<DisplayList>(name),
  listID(0)
{
}

bool DisplayList::init(void)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create display list without OpenGL context");
    return false;
  }

  listID = glGenLists(1);
  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
