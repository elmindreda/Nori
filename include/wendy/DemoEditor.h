///////////////////////////////////////////////////////////////////////
// Wendy demo system
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
#ifndef WENDY_DEMOEDITOR_H
#define WENDY_DEMOEDITOR_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class Editor : public Singleton<Editor>, public Trackable
{
public:
  void render(void);
  bool isVisible(void) const;
  void setVisible(bool newState);
  static bool create(void);
private:
  Editor(void);
  bool init(void);
  bool onRender(void);
  void onCreateEffect(UI::Button& button);
  void onDestroyEffect(UI::Button& button);
  void onZoomChanged(UI::Slider& slider);
  void onResized(unsigned int width, unsigned int height);
  void onKeyPressed(GL::Key key, bool pressed);
  void onKeyPressed(UI::Widget& widget, GL::Key key, bool pressed);
  void onTimeChanged(Timeline& timeline);
  void onParentChanged(Timeline& timeline);
  void onParentSelected(UI::Popup& popup, unsigned int index);
  bool visible;
  Ptr<Show> show;
  Ptr<UI::Book> book;
  UI::Canvas* canvas;
  UI::Widget* commandPanel;
  UI::List* effectType;
  UI::Popup* parentPopup;
  UI::Label* timeDisplay;
  Timeline* timeline;
  Timer timer;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEMOEDITOR_H*/
///////////////////////////////////////////////////////////////////////
