///////////////////////////////////////////////////////////////////////
// Wendy demo system
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
#ifndef WENDY_DEMOSHOW_H
#define WENDY_DEMOSHOW_H
///////////////////////////////////////////////////////////////////////

#include <vector>
#include <stack>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Show : public Resource<Show>
{
public:
  void render(void) const;
  Effect& getRootEffect(void);
  const String& getTitle(void) const;
  void setTitle(const String& newTitle);
  Time getDuration(void) const;
  Time getTimeElapsed(void) const;
  void setTimeElapsed(Time newTime);
  static Show* createInstance(const String& name = "");
private:
  Show(const String& name);
  bool init(void);
  void updateEffect(Effect& effect, Time newTime);
  Ptr<Effect> root;
  String title;
};

///////////////////////////////////////////////////////////////////////

/*
class ShowCodecXML : public ResourceCodec<Show>, public XML::Codec
{
public:
  ShowCodecXML(void);
  Show* read(const Path& path, const String& name = "");
  Show* read(Stream& stream, const String& name = "");
  bool write(const Path& path, const Show& show);
  bool write(Stream& stream, const Show& show);
private:
  bool onBeginElement(const String& name);
  bool onEndElement(const String& name);
  Ptr<Show> show;
  std::stack<String> effectNameStack;
};
*/

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEMOSHOW_H*/
///////////////////////////////////////////////////////////////////////
