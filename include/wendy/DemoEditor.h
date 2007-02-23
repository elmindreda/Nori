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

class Timeline;

///////////////////////////////////////////////////////////////////////

class TimelineRuler : public UI::Widget
{
};

///////////////////////////////////////////////////////////////////////

class TimelineEffect : public UI::Widget
{
public:
  TimelineEffect(Timeline& timeline, Effect& effect);
  Effect& getEffect(void) const;
private:
  enum Mode
  {
    NOT_DRAGGING,
    DRAGGING_POSITION,
    DRAGGING_START,
    DRAGGING_DURATION,
  };
  void render(void) const;
  void onDragBegun(Widget& widget, const Vector2& position);
  void onDragMoved(Widget& widget, const Vector2& position);
  void onDragEnded(Widget& widget, const Vector2& position);
  void onWindowChanged(Timeline& timeline);
  Timeline& timeline;
  Effect& effect;
  Vector2 reference;
  Mode mode;
};

///////////////////////////////////////////////////////////////////////

class Timeline : public UI::Widget
{
public:
  Timeline(Effect& root);
  Time getWindowStart(void) const;
  void setWindowStart(Time newStart);
  Time getWindowDuration(void) const;
  void setWindowDuration(Time newDuration);
  Time getTimeElapsed(void) const;
  void setTimeElapsed(Time newTime);
  void setRootEffect(Effect& newEffect);
  SignalProxy1<void, Timeline&> getWindowChangedSignal(void);
private:
  void onAreaChanged(Widget& widget);
  typedef std::vector<TimelineEffect*> EffectList;
  Signal1<void, Timeline&> windowChangedSignal;
  UI::View* view;
  EffectList effects;
  Effect* root;
  Time windowStart;
  Time windowDuration;
  Time elapsed;
};

///////////////////////////////////////////////////////////////////////

class Editor : public Singleton<Editor>, public Trackable
{
public:
  static bool create(void);
private:
  Editor(void);
  bool init(void);
  bool onRender(void);
  void onResized(unsigned int width, unsigned int height);
  void onKeyPressed(UI::Widget& widget, GL::Key key, bool pressed);
  Ptr<Show> show;
  Ptr<UI::Window> window;
  UI::Canvas* canvas;
  UI::Widget* commandPanel;
  Timeline* timeline;
  Timer timer;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEMOEDITOR_H*/
///////////////////////////////////////////////////////////////////////
