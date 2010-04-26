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

/*! @ingroup demo
 */
class Editor : public Singleton<Editor>, public Trackable
{
public:
  ~Editor(void);
  void run(void);
  bool isSimple(void) const;
  bool isModified(void) const;
  bool isPaused(void) const;
  bool isVisible(void) const;
  void setVisible(bool newState);
  Time getTimeElapsed(void) const;
  void setTimeElapsed(Time newTime);
  const Show& getShow(void) const;
  SignalProxy0<void> getMusicChangedSignal(void);
  SignalProxy0<void> getTimeChangedSignal(void);
  SignalProxy0<void> getPausedSignal(void);
  SignalProxy0<void> getResumedSignal(void);
  static bool create(const String& showName = "");
private:
  Editor(void);
  bool init(const String& showName);
  void updateTitle(void);
  void togglePaused(void);
  bool onCloseRequest(void);
  void onMaali(UI::Button& button);
  void onLoadShow(UI::Button& button);
  void onSaveShow(UI::Button& button);
  void onRewind(UI::Button& button);
  void onPauseResume(UI::Button& button);
  void onCreateEffect(UI::Button& button);
  void onDestroyAllEffects(UI::Button& button);
  void onZoomChanged(UI::Slider& slider);
  void onResized(unsigned int width, unsigned int height);
  void onDrawShowCanvas(const UI::Canvas& canvas);
  void onKeyPressed(input::Key key, bool pressed);
  void onKeyPressed(UI::Widget& widget, input::Key key, bool pressed);
  void onTimeChanged(Timeline& timeline);
  void onParentChanged(Timeline& timeline);
  void onSelectionChanged(Timeline& timeline);
  void onParentSelected(UI::Popup& popup, unsigned int index);
  Ptr<Show> show;
  bool simple;
  bool modified;
  bool quitting;
  Ptr<UI::Book> book;
  Ptr<UI::Renderer> renderer;
  Ptr<UI::Desktop> desktop;
  UI::Canvas* canvas;
  UI::List* effectType;
  UI::Popup* parentPopup;
  UI::Label* timeDisplay;
  UI::Entry* titleEntry;
  UI::Entry* musicEntry;
  UI::Entry* nameEntry;
  Timeline* timeline;
  Timer timer;
  Time elapsed;
  Signal0<void> musicChangedSignal;
  Signal0<void> timeChangedSignal;
  Signal0<void> pausedSignal;
  Signal0<void> resumedSignal;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEMOEDITOR_H*/
///////////////////////////////////////////////////////////////////////
