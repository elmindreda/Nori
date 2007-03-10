///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2007 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <moira/Moira.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLStatistics.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

void Statistics::addPasses(unsigned int count)
{
  passCount += count;
}

void Statistics::addPrimitives(GLenum mode, unsigned int count)
{
  if (!count)
    return;

  vertexCount += count;

  switch (mode)
  {
    case GL_POINTS:
      pointCount += count;
      break;
    case GL_LINES:
      lineCount += count / 2;
      break;
    case GL_LINE_LOOP:
      lineCount += count;
      break;
    case GL_LINE_STRIP:
      lineCount += count - 1;
      break;
    case GL_TRIANGLES:
      triangleCount += count / 3;
      break;
    case GL_TRIANGLE_STRIP:
      triangleCount += count - 2;
      break;
    case GL_TRIANGLE_FAN:
      triangleCount += count - 1;
      break;
    case GL_QUADS:
      triangleCount += count / 2;
      break;
    case GL_QUAD_STRIP:
      triangleCount += count - 2;
      break;
    case GL_POLYGON:
      triangleCount += count - 2;
      break;
    default:
      Log::writeError("Invalid render mode %u", mode);
  }
}

float Statistics::getFrameRate(void) const
{
  return 0.f;
}

unsigned int Statistics::getFrameCount(void) const
{
  return frameCount;
}

unsigned int Statistics::getPassCount(void) const
{
  return passCount;
}

unsigned int Statistics::getVertexCount(void) const
{
  return vertexCount;
}

unsigned int Statistics::getPointCount(void) const
{
  return pointCount;
}

unsigned int Statistics::getLineCount(void) const
{
  return lineCount;
}

unsigned int Statistics::getTriangleCount(void) const
{
  return triangleCount;
}

bool Statistics::create(void)
{
  if (get())
    return true;

  Ptr<Statistics> statistics = new Statistics();
  if (!statistics->init())
    return false;

  set(statistics.detachObject());
  return true;
}

Statistics::Statistics(void):
  frameCount(0),
  passCount(0),
  vertexCount(0),
  pointCount(0),
  lineCount(0),
  triangleCount(0)
{
}

bool Statistics::init(void)
{
  Context* context = Context::get();
  if (!context)
  {
    Log::writeError("Cannot create statistics tracker without OpenGL context");
    return false;
  }

  context->getFinishSignal().connect(*this, &Statistics::onFinish);
  context->getDestroySignal().connect(*this, &Statistics::onContextDestroy);

  return true;
}

void Statistics::onFinish(void)
{
  frameCount++;

  passCount = 0;
  vertexCount = 0;
  pointCount = 0;
  lineCount = 0;
  triangleCount = 0;
}

void Statistics::onContextDestroy(void)
{
  Log::writeWarning("Statistics tracker not explicitly destroyed before context destruction");
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
