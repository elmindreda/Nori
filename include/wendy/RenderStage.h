///////////////////////////////////////////////////////////////////////
// Wendy default renderer
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_RENDERSTAGE_H
#define WENDY_RENDERSTAGE_H
///////////////////////////////////////////////////////////////////////

#include <list>
#include <vector>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Stage
{
public:
  virtual ~Stage(void);
  virtual void prepare(const Queue& queue);
  virtual void render(const Queue& queue);
};

///////////////////////////////////////////////////////////////////////

class StageStack
{
public:
  ~StageStack(void);
  void addStage(Stage& stage);
  void removeStage(Stage& stage);
  void destroyStages(void);
  void render(const Queue& queue);
private:
  typedef std::vector<Stage*> StageList;
  StageList stages;
};

///////////////////////////////////////////////////////////////////////

namespace stdlib
{

///////////////////////////////////////////////////////////////////////

class DefaultStage : public Stage
{
public:
  void render(const Queue& queue);
};

///////////////////////////////////////////////////////////////////////

class GlowStage : public Stage
{
public:
  void prepare(const Queue& queue);
  void render(const Queue& queue);
};

///////////////////////////////////////////////////////////////////////

class StencilShadowStage : public Stage
{
public:
  void render(const Queue& queue);
};

///////////////////////////////////////////////////////////////////////

} /*namespace stdlib*/

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERSTAGE_H*/
///////////////////////////////////////////////////////////////////////
