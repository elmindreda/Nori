///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2012 Camilla Berglund <elmindreda@elmindreda.org>
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
#include <wendy/Timer.h>
#include <wendy/Profile.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

bool ProfileNode::operator == (const char* string) const
{
  return name == string;
}

Time ProfileNode::getDuration() const
{
  return duration;
}

unsigned int ProfileNode::getCallCount() const
{
  return calls;
}

const char* ProfileNode::getName() const
{
  return name.c_str();
}

const ProfileNode::List& ProfileNode::getChildren() const
{
  return children;
}

ProfileNode::ProfileNode(const char* initName):
  name(initName),
  duration(0.0),
  calls(0)
{
}

ProfileNode* ProfileNode::findChild(const char* name)
{
  auto n = std::find(children.begin(), children.end(), name);
  if (n == children.end())
    return NULL;

  return &(*n);
}

///////////////////////////////////////////////////////////////////////

void Profile::beginFrame()
{
  resetNode(root);
  beginNode(root);
  timer.start();
}

void Profile::endFrame()
{
  endNode();
  timer.stop();
}

void Profile::beginNode(const char* name)
{
  ProfileNode* parent = stack.back();

  ProfileNode* node = parent->findChild(name);
  if (!node)
  {
    parent->children.push_back(ProfileNode(name));
    node = &(parent->children.back());
  }

  beginNode(*node);
}

void Profile::endNode()
{
  ProfileNode* node = stack.back();
  node->duration = timer.getTime() - node->duration;

  stack.pop_back();
}

const ProfileNode& Profile::getRootNode() const
{
  return root;
}

Profile* Profile::getCurrent()
{
  return current;
}

void Profile::setCurrent(Profile* newProfile)
{
  current = newProfile;
}

void Profile::beginNode(ProfileNode& node)
{
  node.calls++;
  node.duration = timer.getTime() - node.duration;

  stack.push_back(&node);
}

void Profile::resetNode(ProfileNode& node)
{
  node.calls = 0;
  node.duration = 0.0;

  std::for_each(node.children.begin(), node.children.end(), resetNode);
}

Profile* Profile::current = NULL;

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
