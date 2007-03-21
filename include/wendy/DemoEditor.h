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

class TimelineRuler : public UI::Widget
{
};

///////////////////////////////////////////////////////////////////////

class TimelineTrack : public UI::Widget
{
public:
  TimelineTrack(Effect& effect);
  Effect& getEffect(void) const;
private:
  void draw(void) const;
  Effect& effect;
};

///////////////////////////////////////////////////////////////////////

class Timeline : public UI::Widget
{
public:
  Timeline(Show& show);
  Time getWindowStart(void) const;
  void setWindowStart(Time newStart);
  float getScale(void) const;
  void setScale(float newScale);
  Time getTimeElapsed(void) const;
  void setTimeElapsed(Time newTime);
  Effect* getParentEffect(void) const;
  void setParentEffect(Effect* newEffect);
  SignalProxy1<void, Timeline&> getWindowChangedSignal(void);
private:
  enum DragMode
  {
    NOT_DRAGGING,
    DRAGGING_POSITION,
    DRAGGING_START,
    DRAGGING_DURATION,
  };
  void update(void);
  void draw(void) const;
  void onAreaChanged(Widget& widget);
  typedef std::vector<TimelineTrack*> EffectList;
  Signal1<void, Timeline&> windowChangedSignal;
  Show& show;
  EffectList tracks;
  Effect* parent;
  Time windowStart;
  float scale;
  Time elapsed;
  DragMode dragMode;
};

///////////////////////////////////////////////////////////////////////

class Editor : public Singleton<Editor>, public Trackable
{
public:
  bool isVisible(void) const;
  void setVisible(bool newState);
  static bool create(void);
private:
  Editor(void);
  bool init(void);
  bool onRender(void);
  void onCreateEffect(UI::Button& button);
  void onDestroyEffect(UI::Button& button);
  void onResized(unsigned int width, unsigned int height);
  void onKeyPressed(GL::Key key, bool pressed);
  void onKeyPressed(UI::Widget& widget, GL::Key key, bool pressed);
  void onTimeSlider(UI::Slider& slider);
  bool visible;
  Ptr<Show> show;
  Ptr<UI::Window> window;
  UI::Canvas* canvas;
  UI::Widget* commandPanel;
  UI::List* effectType;
  UI::Slider* timeSlider;
  Timeline* timeline;
  Timer timer;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEMOEDITOR_H*/
///////////////////////////////////////////////////////////////////////
