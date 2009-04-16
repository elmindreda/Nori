///////////////////////////////////////////////////////////////////////
// Wendy default renderer
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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLState.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderQueue.h>
#include <wendy/RenderStage.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Stage::~Stage(void)
{
}

void Stage::prepare(const Queue& queue)
{
}

void Stage::render(const Queue& queue)
{
}

///////////////////////////////////////////////////////////////////////

StageStack::~StageStack(void)
{
  destroyStages();
}

void StageStack::addStage(Stage& stage)
{
  stages.push_back(&stage);
}

void StageStack::removeStage(Stage& stage)
{
  StageList::iterator i = std::find(stages.begin(), stages.end(), &stage);
  if (i != stages.end())
    stages.erase(i);
}

void StageStack::destroyStages(void)
{
  while (!stages.empty())
  {
    delete stages.back();
    stages.pop_back();
  }
}

void StageStack::render(const Queue& queue)
{
  for (StageList::iterator i = stages.begin();  i != stages.end();  i++)
    (*i)->prepare(queue);

  for (StageList::iterator i = stages.begin();  i != stages.end();  i++)
    (*i)->render(queue);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
