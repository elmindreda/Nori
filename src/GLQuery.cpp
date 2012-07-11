///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
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

#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLQuery.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLHelper.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

OcclusionQuery::~OcclusionQuery()
{
  if (active)
    logError("Occlusion query destroyed while active");

  if (queryID)
    glDeleteQueries(1, &queryID);

#if WENDY_DEBUG
  checkGL("OpenGL error during occlusion query deletion");
#endif
}

void OcclusionQuery::begin()
{
  if (active)
  {
    logError("Cannot begin already active occlusion query");
    return;
  }

  glBeginQuery(GL_SAMPLES_PASSED, queryID);

  active = true;

#if WENDY_DEBUG
  checkGL("OpenGL error during occlusion query begin");
#endif
}

void OcclusionQuery::end()
{
  if (!active)
  {
    logError("Cannot end non-active occlusion query");
    return;
  }

  glEndQuery(GL_SAMPLES_PASSED);

  active = false;

#if WENDY_DEBUG
  checkGL("OpenGL error during occlusion query end");
#endif
}

bool OcclusionQuery::isActive() const
{
  return active;
}

bool OcclusionQuery::hasResultAvailable() const
{
  if (active)
    return false;

  int available;
  glGetQueryObjectiv(queryID, GL_QUERY_RESULT_AVAILABLE, &available);

#if WENDY_DEBUG
  if (!checkGL("OpenGL error during occlusion query result availability check"))
    return false;
#endif

  return available ? true : false;
}

uint OcclusionQuery::getResult() const
{
  if (active)
  {
    logError("Cannot retrieve result of active occlusion query");
    return 0;
  }

  uint result;
  glGetQueryObjectuiv(queryID, GL_QUERY_RESULT, &result);

#if WENDY_DEBUG
  if (!checkGL("OpenGL error during occlusion query result retrieval"))
    return 0;
#endif

  return result;
}

OcclusionQuery* OcclusionQuery::create(Context& context)
{
  Ptr<OcclusionQuery> query(new OcclusionQuery(context));
  if (!query->init())
    return NULL;

  return query.detachObject();
}

OcclusionQuery::OcclusionQuery(Context& initContext):
  context(initContext),
  queryID(0),
  active(false)
{
}

bool OcclusionQuery::init()
{
  glGenQueries(1, &queryID);

  if (!checkGL("OpenGL error during creation of occlusion query object"))
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
