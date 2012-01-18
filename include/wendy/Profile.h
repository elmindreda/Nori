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
#ifndef WENDY_PROFILE_H
#define WENDY_PROFILE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class Profile;

///////////////////////////////////////////////////////////////////////

class ProfileNode
{
  friend class Profile;
public:
  typedef std::vector<ProfileNode> List;
  bool operator == (const char* string) const;
  Time getDuration() const;
  unsigned int getCallCount() const;
  const char* getName() const;
  const List& getChildren() const;
private:
  explicit ProfileNode(const char* name);
  ProfileNode* findChild(const char* name);
  String name;
  Time duration;
  List children;
  unsigned int calls;
};

///////////////////////////////////////////////////////////////////////

class ProfileNodeCall
{
public:
  ProfileNodeCall(const char* name);
  ~ProfileNodeCall();
private:
  Profile* profile;
};

///////////////////////////////////////////////////////////////////////

class Profile
{
public:
  void beginFrame();
  void endFrame();
  void beginNode(const char* name);
  void endNode();
  const ProfileNode& getRootNode() const;
  static Profile* getCurrent();
  static void setCurrent(Profile* newProfile);
private:
  void beginNode(ProfileNode& node);
  static void resetNode(ProfileNode& node);
  typedef std::vector<ProfileNode*> Stack;
  ProfileNode root;
  Stack stack;
  Timer timer;
  static Profile* current;
};

///////////////////////////////////////////////////////////////////////

inline ProfileNodeCall::ProfileNodeCall(const char* name):
  profile(Profile::getCurrent())
{
  if (profile)
    profile->beginNode(name);
}

inline ProfileNodeCall::~ProfileNodeCall()
{
  if (profile)
    profile->endNode();
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_PROFILE_H*/
///////////////////////////////////////////////////////////////////////
