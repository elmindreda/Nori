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

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>

#include <wendy/RenderMaterial.h>
#include <wendy/RenderFont.h>

#include <wendy/Input.h>

#include <wendy/UIRender.h>
#include <wendy/UIDesktop.h>
#include <wendy/UIWidget.h>
#include <wendy/UIWindow.h>
#include <wendy/UIView.h>
#include <wendy/UICanvas.h>
#include <wendy/UIScroller.h>
#include <wendy/UISlider.h>
#include <wendy/UILayout.h>
#include <wendy/UIButton.h>
#include <wendy/UILabel.h>
#include <wendy/UIItem.h>
#include <wendy/UIList.h>
#include <wendy/UIMenu.h>

#include <wendy/DemoProperty.h>
#include <wendy/DemoEffect.h>
#include <wendy/DemoShow.h>
#include <wendy/DemoTimeline.h>

#include <sstream>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {

///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

namespace
{

class TrackPanel : public UI::Widget
{
public:
  TrackPanel(Timeline& timeline);
protected:
  void draw(void) const;
  void addedChild(Widget& child);
  void removedChild(Widget& child);
private:
  void updateTracks(void);
  void onAreaChanged(UI::Widget& widget);
  void onValueChanged(UI::Scroller& scroller);
  Timeline& timeline;
  UI::Scroller* scroller;
};

TrackPanel::TrackPanel(Timeline& initTimeline):
  timeline(initTimeline)
{
  getAreaChangedSignal().connect(*this, &TrackPanel::onAreaChanged);

  scroller = new UI::Scroller(UI::VERTICAL);
  scroller->setValueRange(0.f, 1.f);
  scroller->setPercentage(1.f);
  scroller->getValueChangedSignal().connect(*this, &TrackPanel::onValueChanged);
  addChild(*scroller);
}

void TrackPanel::draw(void) const
{
  const Rect& area = getGlobalArea();

  UI::Renderer* renderer = UI::Renderer::get();
  if (renderer->pushClipArea(area))
  {
    UI::Widget::draw();

    const Time timeOffset = timeline.getWindowStart() +
                            timeline.getParentEffect().getGlobalOffset();
    const float position = (float) (timeline.getTimeElapsed() - timeOffset) *
                           timeline.getSecondWidth();
    const float width = renderer->getDefaultEM() / 3.f;

    Rect markerArea;
    markerArea.set(area.position.x + position - width / 2.f, area.position.y,
                   width, area.size.y);

    renderer->fillRectangle(markerArea, ColorRGBA(0.3f, 0.3f, 0.3f, 0.5f));

    renderer->popClipArea();
  }
}

void TrackPanel::addedChild(Widget& child)
{
  if (&child == scroller)
    return;

  updateTracks();
}

void TrackPanel::removedChild(Widget& child)
{
  if (&child == scroller)
    return;

  updateTracks();
}

void TrackPanel::updateTracks(void)
{
  const UI::WidgetList& children = getChildren();

  const float em = UI::Renderer::get()->getDefaultEM();
  const float height = em * 2.f;
  const float visible = getArea().size.y;

  float width = getArea().size.x;
  float position = visible;

  const float extra = std::max((children.size() - 1) * height - visible, 0.f);
  if (extra == 0.f)
  {
    scroller->setValue(0.f);
    scroller->setVisible(false);
  }
  else
  {
    position += std::min(extra, scroller->getValue());
    width -= scroller->getArea().size.x + 2.f;

    scroller->setArea(Rect(width, 0.f, scroller->getArea().size.x, visible));
    scroller->setValueRange(0.f, extra);
    scroller->setPercentage(visible / (visible + extra));
    scroller->setVisible(true);
  }

  for (UI::WidgetList::const_iterator i = children.begin();  i != children.end();  i++)
  {
    if (*i == scroller)
      continue;

    position -= height;

    (*i)->setArea(Rect(0.f, position, width, height));
  }
}

void TrackPanel::onAreaChanged(UI::Widget& widget)
{
  updateTracks();
}

void TrackPanel::onValueChanged(UI::Scroller& scroller)
{
  updateTracks();
}

}

///////////////////////////////////////////////////////////////////////

TimelineRuler::TimelineRuler(Timeline& initTimeline):
  timeline(initTimeline)
{
  const float em = UI::Renderer::get()->getDefaultEM();

  getButtonClickedSignal().connect(*this, &TimelineRuler::onButtonClicked);
  getDragMovedSignal().connect(*this, &TimelineRuler::onDragMoved);

  setSize(Vec2(em * 2.f, em * 2.f));
  setDraggable(true);
}

Time TimelineRuler::getTimeElapsed(void) const
{
  return elapsed;
}

void TimelineRuler::setTimeElapsed(Time newTime)
{
  elapsed = newTime;
}

SignalProxy1<void, TimelineRuler&> TimelineRuler::getTimeChangedSignal(void)
{
  return timeChangedSignal;
}

void TimelineRuler::draw(void) const
{
  const Rect& area = getGlobalArea();

  UI::Renderer* renderer = UI::Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawFrame(area, getState());

    const float em = renderer->getDefaultEM();
    const float width = timeline.getSecondWidth();
    const float start = (float) timeline.getWindowStart();
    const float offset = (1.f - start + floorf(start)) * width;
    const unsigned int count = (unsigned int) ((area.size.x + width) / width);

    int index = (int) start + 1;

    Segment2 segment;

    for (unsigned int i = 0;  i < count;  i++)
    {
      float position = offset + i * width;

      segment.start = area.position + Vec2(position, area.size.y / 2.f);
      segment.end = area.position + Vec2(position, area.size.y);

      renderer->drawLine(segment, ColorRGBA::BLACK);

      if ((index + i) % 10 == 0)
      {
        Rect digitArea;
        digitArea.position = area.position + Vec2(position - em, 0.f);
        digitArea.size.set(2.f * em, area.size.y);

        String digits;
        Variant::convertToString(digits, (index + (int) i) % 60);

        renderer->drawText(digitArea, digits);
      }
    }

    const Time timeOffset = timeline.getWindowStart() +
                            timeline.getParentEffect().getGlobalOffset();
    float position = (float) (elapsed - timeOffset) * width;

    Triangle2 triangle;
    triangle.P[0] = Vec2(position + area.size.y / 2.f, area.size.y) + area.position;
    triangle.P[1] = Vec2(position - area.size.y / 2.f, area.size.y) + area.position;
    triangle.P[2] = Vec2(position, 0.f) + area.position;

    renderer->fillTriangle(triangle, ColorRGBA(0.8f, 0.1f, 0.1f, 1.f));
    renderer->drawTriangle(triangle, ColorRGBA::BLACK);

    UI::Widget::draw();

    renderer->popClipArea();
  }
}

void TimelineRuler::onButtonClicked(Widget& widget,
                                    const Vec2& point,
				    input::Button button,
				    bool clicked)
{
  if (!clicked)
    return;

  const float position = transformToLocal(point).x;

  elapsed = timeline.getWindowStart() + position / timeline.getSecondWidth();
  elapsed = std::max(elapsed, 0.0);
  elapsed = timeline.getParentEffect().getGlobalOffset() +
            timeline.getSnappedTime(elapsed);

  timeChangedSignal.emit(*this);
}

void TimelineRuler::onDragMoved(Widget& widget, const Vec2& point)
{
  const float position = transformToLocal(point).x;

  elapsed = timeline.getWindowStart() + position / timeline.getSecondWidth();
  elapsed = std::max(elapsed, 0.0);
  elapsed = timeline.getParentEffect().getGlobalOffset() +
            timeline.getSnappedTime(elapsed);

  timeChangedSignal.emit(*this);
}

///////////////////////////////////////////////////////////////////////

EffectTrack::EffectTrack(Timeline& initTimeline, Effect& initEffect):
  timeline(initTimeline),
  effect(initEffect),
  mode(NOT_DRAGGING)
{
  const float em = UI::Renderer::get()->getDefaultEM();

  getDragBegunSignal().connect(*this, &EffectTrack::onDragBegun);
  getDragMovedSignal().connect(*this, &EffectTrack::onDragMoved);
  getDragEndedSignal().connect(*this, &EffectTrack::onDragEnded);

  setSize(Vec2(em * 2.f, em * 2.f));
  setDraggable(true);
}

Effect& EffectTrack::getEffect(void) const
{
  return effect;
}

void EffectTrack::draw(void) const
{
  const Rect& area = getGlobalArea();

  UI::Renderer* renderer = UI::Renderer::get();
  if (renderer->pushClipArea(area))
  {
    const float size = getHandleSize();
    const float offset = getHandleOffset();

    Rect effectArea;
    effectArea.size.set(size, area.size.y);
    effectArea.position.set(area.position.x + offset, area.position.y);

    ColorRGBA color;

    if (effect.isActive())
      color.set(0.1f, 0.7f, 0.1f, 1.f);
    else
      color.set(renderer->getWidgetColor(), 1.f);

    renderer->fillRectangle(effectArea, color);
    renderer->drawRectangle(effectArea, ColorRGBA::BLACK);

    renderer->drawText(effectArea, effect.getName());

    const float em = renderer->getDefaultEM();

    Rect handleArea;
    handleArea.size.set(em, effectArea.size.y);

    handleArea.position = effectArea.position - Vec2(em, 0.f);
    renderer->drawHandle(handleArea, getState());

    handleArea.position = effectArea.position + Vec2(effectArea.size.x, 0.f);
    renderer->drawHandle(handleArea, getState());

    UI::Widget::draw();

    renderer->popClipArea();
  }
}

void EffectTrack::onDragBegun(Widget& widget, const Vec2& point)
{
  const float position = transformToLocal(point).x;

  UI::Renderer* renderer = UI::Renderer::get();

  const float em = renderer->getDefaultEM();
  const float size = getHandleSize();
  const float offset = getHandleOffset();

  if (position >= offset - em && position < offset + size + em)
  {
    reference = position - offset;

    if (reference < 0.f)
      mode = DRAGGING_START;
    else if (reference < size)
      mode = DRAGGING_POSITION;
    else
    {
      mode = DRAGGING_DURATION;
      reference -= size;
    }
  }
  else
    cancelDragging();
}

void EffectTrack::onDragMoved(Widget& widget, const Vec2& point)
{
  const float position = transformToLocal(point).x;
  const float offset = getHandleOffset();

  if (mode == DRAGGING_POSITION)
  {
    Time start = timeline.getWindowStart() +
                 (position - reference) / timeline.getSecondWidth();

    start = timeline.getSnappedTime(start);

    if (start >= 0.0)
      effect.setStartTime(start);
  }
  else if (mode == DRAGGING_START)
  {
    Time start = timeline.getWindowStart() +
                 (position - reference) / timeline.getSecondWidth();

    start = timeline.getSnappedTime(start);

    Time duration = effect.getDuration() + effect.getStartTime() - start;
    if (start >= 0.0 && duration >= 0.0)
    {
      effect.setStartTime(start);
      effect.setDuration(duration);
    }
  }
  else
  {
    Time duration = (position - offset - reference) / timeline.getSecondWidth();

    duration = timeline.getSnappedTime(duration);

    if (duration >= 0.0)
      effect.setDuration(duration);
  }
}

void EffectTrack::onDragEnded(Widget& widget, const Vec2& point)
{
  mode = NOT_DRAGGING;
}

float EffectTrack::getHandleSize(void) const
{
  return (float) effect.getDuration() * timeline.getSecondWidth();
}

float EffectTrack::getHandleOffset(void) const
{
  return (float) (effect.getStartTime() - timeline.getWindowStart()) * timeline.getSecondWidth();
}

///////////////////////////////////////////////////////////////////////

PropertyTrack::PropertyTrack(Timeline& initTimeline, Property& initProperty):
  timeline(initTimeline),
  property(initProperty),
  draggedKey(NULL)
{
  const float em = UI::Renderer::get()->getDefaultEM();

  getKeyPressedSignal().connect(*this, &PropertyTrack::onKeyPressed);
  getDragBegunSignal().connect(*this, &PropertyTrack::onDragBegun);
  getDragMovedSignal().connect(*this, &PropertyTrack::onDragMoved);
  getDragEndedSignal().connect(*this, &PropertyTrack::onDragEnded);

  setSize(Vec2(em * 2.f, em * 2.f));
  setDraggable(true);
}

Property& PropertyTrack::getParameter(void) const
{
  return property;
}

void PropertyTrack::draw(void) const
{
  const Rect& area = getGlobalArea();

  UI::Renderer* renderer = UI::Renderer::get();
  if (renderer->pushClipArea(area))
  {
    Rect wellArea;
    wellArea.position.set(area.position.x - (float) timeline.getWindowStart() * timeline.getSecondWidth(),
                          area.position.y);
    wellArea.size.set((float) property.getEffect().getDuration() * timeline.getSecondWidth(),
                      area.size.y);

    renderer->drawWell(wellArea, getState());
    renderer->drawText(wellArea, property.getName());

    const float em = renderer->getDefaultEM();

    const Property::KeyList& keys = property.getKeys();

    for (unsigned int i = 0;  i < keys.size();  i++)
    {
      const float offset = (float) (keys[i]->getMoment() - timeline.getWindowStart()) *
                           timeline.getSecondWidth();

      Rect handleArea;
      handleArea.position.set(area.position.x + offset - em / 2.f,
                              area.position.y);
      handleArea.size.set(em, area.size.y);

      renderer->drawHandle(handleArea, getState());
    }

    renderer->popClipArea();
  }
}

void PropertyTrack::onKeyPressed(Widget& widget, input::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case input::Key::DELETE:
    {
      break;
    }
  }
}

void PropertyTrack::onDragBegun(Widget& widget, const Vec2& point)
{
  float position = transformToLocal(point).x;

  UI::Renderer* renderer = UI::Renderer::get();

  const Property::KeyList& keys = property.getKeys();

  const float em = renderer->getDefaultEM();

  for (unsigned int i = 0;  i < keys.size();  i++)
  {
    const float offset = (float) (keys[i]->getMoment() - timeline.getWindowStart()) *
                         timeline.getSecondWidth();

    if (position >= offset - em / 2.f && position < offset + em / 2.f)
    {
      draggedKey = keys[i];
      break;
    }
  }

  if (!draggedKey)
  {
    const Time moment = position / timeline.getSecondWidth() -
                        timeline.getWindowStart();

    if (moment <= property.getEffect().getDuration())
      draggedKey = &(property.createKey(moment));
    else
      cancelDragging();
  }
}

void PropertyTrack::onDragMoved(Widget& widget, const Vec2& point)
{
  float position = transformToLocal(point).x;

  Time moment;

  moment = position / timeline.getSecondWidth() + timeline.getWindowStart();
  moment = std::min(moment, property.getEffect().getDuration());
  moment = timeline.getSnappedTime(moment);

  draggedKey->setMoment(moment);
}

void PropertyTrack::onDragEnded(Widget& widget, const Vec2& point)
{
  draggedKey = NULL;
}

///////////////////////////////////////////////////////////////////////

Timeline::Timeline(Show& initShow):
  show(initShow),
  parent(NULL),
  selected(NULL),
  ruler(NULL),
  start(0.0),
  zoom(1.f)
{
  getAreaChangedSignal().connect(*this, &Timeline::onAreaChanged);

  UI::Layout* mainLayout = new UI::Layout(UI::HORIZONTAL);
  addChild(*mainLayout);

  UI::Layout* vertLayout = new UI::Layout(UI::VERTICAL, false);
  mainLayout->addChild(*vertLayout, 0.f);

  ruler = new TimelineRuler(*this);
  ruler->getTimeChangedSignal().connect(*this, &Timeline::onTimeChanged);
  vertLayout->addChild(*ruler);

  trackPanel = new TrackPanel(*this);
  trackPanel->getButtonClickedSignal().connect(*this, &Timeline::onButtonClicked);
  vertLayout->addChild(*trackPanel, 0.f);

  scroller = new UI::Scroller(UI::HORIZONTAL);
  scroller->getValueChangedSignal().connect(*this, &Timeline::onValueChanged);
  vertLayout->addChild(*scroller);

  effectMenu = new UI::Menu();
  effectMenu->addItem(*new UI::Item("Delete", MENU_DELETE));
  effectMenu->addItem(*new UI::SeparatorItem());
  effectMenu->addItem(*new UI::Item("Rename", MENU_RENAME));
  effectMenu->addItem(*new UI::SeparatorItem());
  effectMenu->addItem(*new UI::Item("Move Up", MENU_MOVE_UP));
  effectMenu->addItem(*new UI::Item("Move Down", MENU_MOVE_DOWN));
  effectMenu->addItem(*new UI::SeparatorItem());
  effectMenu->addItem(*new UI::Item("Enter", MENU_ENTER));
  effectMenu->getItemSelectedSignal().connect(*this, &Timeline::onItemSelected);

  canvasMenu = new UI::Menu();
  canvasMenu->addItem(*new UI::Item("Exit All", MENU_EXIT_ALL));
  canvasMenu->addItem(*new UI::Item("Exit Parent", MENU_EXIT_PARENT));
  canvasMenu->getItemSelectedSignal().connect(*this, &Timeline::onItemSelected);

  setParentEffect(show.getRootEffect());
}

bool Timeline::createEffect(EffectType& type)
{
  unsigned int index = 1;
  std::stringstream stream;

  do
  {
    stream.str("");
    stream << type.getName() << ' ' << index++;
  }
  while (Effect::findInstance(stream.str()));

  try
  {
    Effect* effect = type.createEffect(stream.str());
    if (!effect)
      return false;

    effect->setStartTime(start);
    effect->setDuration(10.0);
    parent->addChild(*effect);

    createTrack(*effect);
  }
  catch (Exception& exception)
  {
    Log::writeError("Failed to create effect %s of type %s: %s",
                    stream.str().c_str(),
		    type.getName().c_str(),
		    exception.what());
    return false;
  }

  return true;
}

void Timeline::destroyEffect(void)
{
  if (selected)
  {
    Effect* effect = &(selected->getEffect());

    tracks.erase(std::find(tracks.begin(), tracks.end(), selected));

    delete selected;
    selected = NULL;
    effectSelectedSignal.emit(*this);

    delete effect;
  }
}

Time Timeline::getWindowStart(void) const
{
  return start;
}

void Timeline::setWindowStart(Time newStart)
{
  start = newStart;
  scroller->setValue((float) start);
}

float Timeline::getZoom(void) const
{
  return zoom;
}

void Timeline::setZoom(float newZoom)
{
  zoom = newZoom;
  updateScroller();
}

Time Timeline::getTimeElapsed(void) const
{
  return ruler->getTimeElapsed();
}

void Timeline::setTimeElapsed(Time newTime)
{
  ruler->setTimeElapsed(newTime);
}

Time Timeline::getVisibleDuration(void) const
{
  return getArea().size.x / getSecondWidth();
}

float Timeline::getSecondWidth(void) const
{
  const float em = UI::Renderer::get()->getDefaultEM();

  return em * zoom;
}

Effect* Timeline::getSelectedEffect(void) const
{
  if (selected)
    return &(selected->getEffect());

  return NULL;
}

Effect& Timeline::getParentEffect(void) const
{
  return *parent;
}

void Timeline::setParentEffect(Effect& newParent)
{
  if (parent == &newParent)
    return;

  selected = NULL;
  effectSelectedSignal.emit(*this);

  while (!tracks.empty())
  {
    delete tracks.back();
    tracks.pop_back();
  }

  parent = &newParent;

  const Effect::PropertyList& properties = parent->getProperties();

  for (Effect::PropertyList::const_iterator i = properties.begin();  i != properties.end();  i++)
    createTrack(**i);

  const Effect::List& effects = parent->getChildren();

  for (Effect::List::const_iterator i = effects.begin();  i != effects.end();  i++)
    createTrack(**i);

  updateScroller();

  parentChangedSignal.emit(*this);
}

Time Timeline::getSnappedTime(Time time) const
{
  input::Context* context = input::Context::get();

  if (context->isKeyDown(input::Key::LSHIFT) || context->isKeyDown(input::Key::RSHIFT))
    return floor(time + 0.5);

  return time;
}

SignalProxy1<void, Timeline&> Timeline::getTimeChangedSignal(void)
{
  return timeChangedSignal;
}

SignalProxy1<void, Timeline&> Timeline::getParentChangedSignal(void)
{
  return parentChangedSignal;
}

SignalProxy1<void, Timeline&> Timeline::getEffectSelectedSignal(void)
{
  return effectSelectedSignal;
}

SignalProxy1<void, Timeline&> Timeline::getPropertySelectedSignal(void)
{
  return propertySelectedSignal;
}

SignalProxy1<void, Timeline&> Timeline::getPropertyKeySelectedSignal(void)
{
  return propertyKeySelectedSignal;
}

void Timeline::draw(void) const
{
  const Rect& area = getGlobalArea();

  UI::Renderer* renderer = UI::Renderer::get();
  if (renderer->pushClipArea(area))
  {
    UI::Widget::draw();

    renderer->popClipArea();
  }
}

void Timeline::updateScroller(void)
{
  const Time visible = getVisibleDuration();
  const Time duration = parent->getDuration();

  scroller->setPercentage((float) std::max(visible / (duration + 10.0), 0.0));
  scroller->setValueRange(0.f, (float) duration + 10.f);
}

void Timeline::createTrack(Property& property)
{
  PropertyTrack* track = new PropertyTrack(*this, property);
  track->getButtonClickedSignal().connect(*this, &Timeline::onButtonClicked);
  trackPanel->addChild(*track);
  tracks.push_back(track);
}

void Timeline::createTrack(Effect& effect)
{
  EffectTrack* track = new EffectTrack(*this, effect);
  track->getButtonClickedSignal().connect(*this, &Timeline::onButtonClicked);
  trackPanel->addChild(*track);
  tracks.push_back(track);
}

void Timeline::onButtonClicked(Widget& widget,
		               const Vec2& point,
		               input::Button button,
		               bool clicked)
{
  if (&widget == trackPanel)
  {
    if (clicked && button == input::Button::RIGHT)
    {
      if (parent->getParent())
      {
	canvasMenu->setPosition(point);
	canvasMenu->display(*getDesktop());
      }
    }
  }
  else if (EffectTrack* track = dynamic_cast<EffectTrack*>(&widget))
  {
    selected = track;
    effectSelectedSignal.emit(*this);

    if (clicked && button == input::Button::RIGHT)
    {
      effectMenu->setPosition(point);
      effectMenu->display(*getDesktop());
    }
  }
  else if (PropertyTrack* track = dynamic_cast<PropertyTrack*>(&widget))
  {
    // TODO: The code.
  }
}

void Timeline::onValueChanged(UI::Scroller& scroller)
{
  start = scroller.getValue();
}

void Timeline::onTimeChanged(TimelineRuler& ruler)
{
  timeChangedSignal.emit(*this);
}

void Timeline::onAreaChanged(UI::Widget& widget)
{
  updateScroller();
}

void Timeline::onItemSelected(UI::Menu& menu, unsigned int index)
{
  UI::Item* item = menu.getItem(index);

  switch (item->getID())
  {
    case MENU_ENTER:
    {
      if (selected)
	setParentEffect(selected->getEffect());
      break;
    }

    case MENU_EXIT_PARENT:
    {
      if (parent->getParent())
	setParentEffect(*parent->getParent());

      break;
    }

    case MENU_EXIT_ALL:
    {
      Effect* root = parent;
      while (root->getParent())
	root = root->getParent();

      if (parent != root)
	setParentEffect(*root);

      break;
    }

    case MENU_RENAME:
    {
      break;
    }

    case MENU_MOVE_UP:
    {
      break;
    }

    case MENU_MOVE_DOWN:
    {
      break;
    }

    case MENU_DELETE:
    {
      destroyEffect();
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
