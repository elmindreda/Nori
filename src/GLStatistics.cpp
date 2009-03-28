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
  Frame& frame = frames.front();
  frame.passCount += count;
}

void Statistics::addPrimitives(RenderMode mode, unsigned int count)
{
  if (!count)
    return;

  Frame& frame = frames.front();
  frame.vertexCount += count;

  switch (mode)
  {
    case RENDER_POINTS:
      frame.pointCount += count;
      break;
    case RENDER_LINES:
      frame.lineCount += count / 2;
      break;
    case RENDER_LINE_STRIP:
      frame.lineCount += count - 1;
      break;
    case RENDER_TRIANGLES:
      frame.triangleCount += count / 3;
      break;
    case RENDER_TRIANGLE_STRIP:
      frame.triangleCount += count - 2;
      break;
    case RENDER_TRIANGLE_FAN:
      frame.triangleCount += count - 1;
      break;
    default:
      Log::writeError("Invalid render mode %u", mode);
  }
}

float Statistics::getFrameRate(void) const
{
  return frameRate;
}

unsigned int Statistics::getFrameCount(void) const
{
  return frameCount;
}

const Statistics::Frame& Statistics::getFrame(void) const
{
  return frames.front();
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
  frameRate(0.f)
{
  static bool initialized = false;

  if (!initialized)
  {
    Context::getDestroySignal().connect(&Statistics::onContextDestroy);
    initialized = true;
  }
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

  frames.push_back(Frame());

  timer.start();
  return true;
}

void Statistics::onFinish(void)
{
  frameCount++;

  frames.push_front(Frame());
  if (frames.size() > 60)
    frames.pop_back();

  frameRate = 0.f;

  for (unsigned int i = 0;  i < frames.size();  i++)
    frameRate += frames[i].duration / frames.size();
}

void Statistics::onContextDestroy(void)
{
  Log::writeWarning("Statistics tracker not explicitly destroyed before context destruction");

  destroy();
}

///////////////////////////////////////////////////////////////////////

Statistics::Frame::Frame(void):
  passCount(0),
  vertexCount(0),
  pointCount(0),
  lineCount(0),
  triangleCount(0),
  duration(0.0)
{
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
