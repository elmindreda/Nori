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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLShader.h>
#include <wendy/GLState.h>

#include <wendy/RenderStyle.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

Pass::Pass(const String& initName):
  name(initName)
{
}

void Pass::apply(void) const
{
  RenderState::apply();
}

bool Pass::isCompatible(void) const
{
  // TODO: The code.

  return true;
}

const String& Pass::getName(void) const
{
  return name;
}

void Pass::setDefaults(void)
{
  RenderState::setDefaults();
}

///////////////////////////////////////////////////////////////////////

Technique::Technique(const String& initName):
  name(initName),
  quality(1.f)
{
}

Pass& Technique::createPass(const String& name)
{
  if (findPass(name))
    throw Exception("Duplicate render pass name");

  passes.push_back(Pass(name));
  return passes.back();
}

void Technique::destroyPass(Pass& pass)
{
  for (List::iterator i = passes.begin();  i != passes.end();  i++)
  {
    if (&(*i) == &pass)
    {
      passes.erase(i);
      break;
    }
  }
}

void Technique::destroyPasses(void)
{
  passes.clear();
}

void Technique::applyPass(unsigned int index) const
{
  getPass(index).apply();
}

Pass* Technique::findPass(const String& name)
{
  if (name.empty())
    return NULL;

  for (List::iterator i = passes.begin();  i != passes.end();  i++)
  {
    if ((*i).getName() == name)
      return &(*i);
  }

  return NULL;
}

const Pass* Technique::findPass(const String& name) const
{
  if (name.empty())
    return NULL;

  for (List::const_iterator i = passes.begin();  i != passes.end();  i++)
  {
    if ((*i).getName() == name)
      return &(*i);
  }

  return NULL;
}

bool Technique::operator < (const Technique& other) const
{
  // Styles with blending always go last
  if (isBlending() != other.isBlending())
  {
    if (isBlending())
      return false;
    else
      return true;
  }

  // TODO: Fix this!
 
  return getName() < other.getName();
}

bool Technique::isCompatible(void) const
{
  for (List::const_iterator i = passes.begin();  i != passes.end();  i++)
  {
    if (!(*i).isCompatible())
      return false;
  }

  return true;
}

bool Technique::isBlending(void) const
{
  for (List::const_iterator i = passes.begin();  i != passes.end();  i++)
  {
    if (!(*i).getName().empty())
      continue;

    if ((*i).isBlending())
      return true;
  }

  return false;
}

Pass& Technique::getPass(unsigned int index)
{
  List::iterator pass = passes.begin();
  std::advance(pass, index);
  return *pass;
}

const Pass& Technique::getPass(unsigned int index) const
{
  List::const_iterator pass = passes.begin();
  std::advance(pass, index);
  return *pass;
}

unsigned int Technique::getPassCount(void) const
{
  return (unsigned int) passes.size();
}

const String& Technique::getName(void) const
{
  return name;
}

float Technique::getQuality(void) const
{
  return quality;
}

void Technique::setQuality(float newQuality)
{
  quality = newQuality;
}

///////////////////////////////////////////////////////////////////////

Style::Style(const String& name):
  Resource<Style>(name),
  active(NULL)
{
}

Style::Style(const Style& source):
  Resource<Style>("")
{
  operator = (source);
}

Style::~Style(void)
{
  destroyTechniques();
}

Technique& Style::createTechnique(const String& name)
{
  if (!name.empty())
  {
    if (findTechnique(name))
      throw Exception("Duplicate technique name");
  }

  Technique* technique = new Technique(name);
  techniques.push_back(technique);
  return *technique;
}

void Style::destroyTechnique(Technique& technique)
{
  if (active == &technique)
    active = NULL;

  List::iterator i = std::find(techniques.begin(), techniques.end(), &technique);
  if (i != techniques.end())
    techniques.erase(i);
}

void Style::destroyTechniques(void)
{
  while (!techniques.empty())
  {
    delete techniques.back();
    techniques.pop_back();
  }
}

Technique* Style::findTechnique(const String& name)
{
  if (name.empty())
    return NULL;

  for (List::iterator i = techniques.begin();  i != techniques.end();  i++)
  {
    if ((*i)->getName() == name)
      return *i;
  }

  return NULL;
}

Style& Style::operator = (const Style& source)
{
  for (List::const_iterator i = source.techniques.begin();  i != source.techniques.end();  i++)
    techniques.push_back(new Technique(**i));

  return *this;
}

unsigned int Style::getTechniqueCount(void) const
{
  return (unsigned int) techniques.size();
}

Technique& Style::getTechnique(unsigned int index)
{
  return *techniques[index];
}

const Technique& Style::getTechnique(unsigned int index) const
{
  return *techniques[index];
}

Technique* Style::getActiveTechnique(void) const
{
  if (!active)
  {
    if (!validateTechniques())
      return NULL;
  }

  return active;
}

bool Style::validateTechniques(void) const
{
  List validated;

  for (List::const_iterator i = techniques.begin();  i != techniques.end();  i++)
  {
    if ((*i)->isCompatible())
      validated.push_back(*i);
  }

  float quality = 0.f;

  for (List::const_iterator i = techniques.begin();  i != techniques.end();  i++)
  {
    if ((*i)->getQuality() >= quality)
    {
      active = *i;
      quality = (*i)->getQuality();
    }
  }

  return active != NULL;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
