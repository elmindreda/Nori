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

#include <wendy/Config.hpp>

#include <wendy/Core.hpp>
#include <wendy/Timer.hpp>
#include <wendy/Profile.hpp>

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

bool ProfileNode::operator == (const char* string) const
{
  return m_name == string;
}

ProfileNode::ProfileNode(const char* name):
  m_name(name),
  m_duration(0.0),
  m_calls(0)
{
}

ProfileNode* ProfileNode::findChild(const char* name)
{
  auto n = std::find(m_children.begin(), m_children.end(), name);
  if (n == m_children.end())
    return nullptr;

  return &(*n);
}

///////////////////////////////////////////////////////////////////////

void Profile::beginFrame()
{
  resetNode(m_root);
  beginNode(m_root);
  m_timer.start();
}

void Profile::endFrame()
{
  endNode();
  m_timer.stop();
}

void Profile::beginNode(const char* name)
{
  ProfileNode* parent = m_stack.back();

  ProfileNode* node = parent->findChild(name);
  if (!node)
  {
    parent->m_children.push_back(ProfileNode(name));
    node = &(parent->m_children.back());
  }

  beginNode(*node);
}

void Profile::endNode()
{
  ProfileNode* node = m_stack.back();
  node->m_duration = m_timer.time() - node->m_duration;

  m_stack.pop_back();
}

void Profile::beginNode(ProfileNode& node)
{
  node.m_calls++;
  node.m_duration = m_timer.time() - node.m_duration;

  m_stack.push_back(&node);
}

void Profile::resetNode(ProfileNode& node)
{
  node.m_calls = 0;
  node.m_duration = 0.0;

  for (auto& c : node.m_children)
    resetNode(c);
}

Profile* Profile::m_current = nullptr;

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
