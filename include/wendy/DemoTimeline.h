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
#ifndef WENDY_DEMOTIMELINE_H
#define WENDY_DEMOTIMELINE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {

///////////////////////////////////////////////////////////////////////

class Timeline;

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class TimelineRuler : public UI::Widget
{
public:
  TimelineRuler(UI::Desktop& desktop, Timeline& timeline);
  Time getTimeElapsed(void) const;
  void setTimeElapsed(Time newTime);
  SignalProxy1<void, TimelineRuler&> getTimeChangedSignal(void);
private:
  void draw(void) const;
  void onButtonClicked(Widget& widget,
                       const Vec2& point,
		       input::Button button,
		       bool clicked);
  void onDragMoved(Widget& widget, const Vec2& point);
  Signal1<void, TimelineRuler&> timeChangedSignal;
  Timeline& timeline;
  Time elapsed;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class EffectTrack : public UI::Widget
{
public:
  EffectTrack(UI::Desktop& desktop, Timeline& timeline, Effect& effect);
  Effect& getEffect(void) const;
private:
  enum DragMode
  {
    NOT_DRAGGING,
    DRAGGING_POSITION,
    DRAGGING_START,
    DRAGGING_DURATION,
  };
  void draw(void) const;
  void onDragBegun(Widget& widget, const Vec2& point);
  void onDragMoved(Widget& widget, const Vec2& point);
  void onDragEnded(Widget& widget, const Vec2& point);
  float getHandleSize(void) const;
  float getHandleOffset(void) const;
  Timeline& timeline;
  Effect& effect;
  DragMode mode;
  float reference;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Demo effect property key interface widget.
 *  @ingroup demo
 */
class PropertyTrack : public UI::Widget
{
public:
  PropertyTrack(UI::Desktop& desktop, Timeline& timeline, Property& property);
  Property& getParameter(void) const;
private:
  void draw(void) const;
  void onKeyPressed(Widget& widget, input::Key key, bool pressed);
  void onDragBegun(Widget& widget, const Vec2& point);
  void onDragMoved(Widget& widget, const Vec2& point);
  void onDragEnded(Widget& widget, const Vec2& point);
  Timeline& timeline;
  Property& property;
  PropertyKey* draggedKey;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup demo
 */
class Timeline : public UI::Widget
{
public:
  Timeline(UI::Desktop& desktop, Show& show);
  bool createEffect(EffectType& type);
  void destroyEffect(void);
  Time getWindowStart(void) const;
  void setWindowStart(Time newStart);
  float getZoom(void) const;
  void setZoom(float newZoom);
  Time getTimeElapsed(void) const;
  void setTimeElapsed(Time newTime);
  Time getVisibleDuration(void) const;
  float getSecondWidth(void) const;
  Effect* getSelectedEffect(void) const;
  Effect& getParentEffect(void) const;
  void setParentEffect(Effect& newEffect);
  Time getSnappedTime(Time time) const;
  SignalProxy1<void, Timeline&> getTimeChangedSignal(void);
  SignalProxy1<void, Timeline&> getParentChangedSignal(void);
  SignalProxy1<void, Timeline&> getEffectSelectedSignal(void);
  SignalProxy1<void, Timeline&> getPropertySelectedSignal(void);
  SignalProxy1<void, Timeline&> getPropertyKeySelectedSignal(void);
private:
  typedef std::vector<UI::Widget*> TrackList;
  enum MenuItem
  {
    MENU_ENTER = 1,
    MENU_EXIT_PARENT,
    MENU_EXIT_ALL,
    MENU_RENAME,
    MENU_MOVE_UP,
    MENU_MOVE_DOWN,
    MENU_DELETE,
  };
  void draw(void) const;
  void updateScroller(void);
  void createTrack(Property& property);
  void createTrack(Effect& effect);
  void onButtonClicked(Widget& widget,
                       const Vec2& point,
		       input::Button button,
		       bool clicked);
  void onValueChanged(UI::Scroller& scroller);
  void onTimeChanged(TimelineRuler& ruler);
  void onAreaChanged(UI::Widget& widget);
  void onItemSelected(UI::Menu& menu, unsigned int index);
  Signal1<void, Timeline&> timeChangedSignal;
  Signal1<void, Timeline&> parentChangedSignal;
  Signal1<void, Timeline&> effectSelectedSignal;
  Signal1<void, Timeline&> propertySelectedSignal;
  Signal1<void, Timeline&> propertyKeySelectedSignal;
  Show& show;
  Effect* parent;
  EffectTrack* selected;
  Ptr<UI::Menu> effectMenu;
  Ptr<UI::Menu> canvasMenu;
  TrackList tracks;
  Time start;
  float zoom;
  TimelineRuler* ruler;
  UI::Widget* trackPanel;
  UI::Scroller* scroller;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEMOTIMELINE_H*/
///////////////////////////////////////////////////////////////////////
