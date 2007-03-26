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

#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLLight.h>
#include <wendy/GLShader.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLPass.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLRender.h>

#include <wendy/RenderFont.h>
#include <wendy/RenderStyle.h>

#include <wendy/UIRender.h>
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

#include <wendy/DemoParameter.h>
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

TimelineRuler::TimelineRuler(Timeline& initTimeline):
  timeline(initTimeline)
{
  const float em = UI::Renderer::get()->getDefaultEM();

  getButtonClickedSignal().connect(*this, &TimelineRuler::onButtonClicked);
  getDragMovedSignal().connect(*this, &TimelineRuler::onDragMoved);

  setSize(Vector2(em * 2.f, em * 2.f));
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
  const Rectangle& area = getGlobalArea();

  UI::Renderer* renderer = UI::Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawFrame(area, getState());

    GL::Renderer::get()->setColor(ColorRGBA::BLACK);
    GL::Renderer::get()->setLineWidth(1.f / GL::Canvas::getCurrent()->getPhysicalHeight());

    const float em = renderer->getDefaultEM();
    const float width = timeline.getSecondWidth();
    const float start = timeline.getWindowStart();
    const float offset = (1.f - start + floorf(start)) * width;
    const unsigned int count = (unsigned int) ((area.size.x + width) / width);

    int index = (int) start + 1;

    Segment2 segment;

    for (unsigned int i = 0;  i < count;  i++)
    {
      float position = offset + i * width;

      segment.start = area.position + Vector2(position, area.size.y / 2.f);
      segment.end = area.position + Vector2(position, area.size.y);

      GL::Renderer::get()->drawLine(segment);

      if ((index + i) % 10 == 0)
      {
	Rectangle digitArea;
	digitArea.position = area.position + Vector2(position - em, 0.f);
	digitArea.size.set(2.f * em, area.size.y);

	String digits;
	Variant::convertToString(digits, index + (int) i);

	renderer->drawText(digitArea, digits);
      }
    }

    float position = (elapsed - timeline.getWindowStart()) * width;

    Triangle2 triangle;
    triangle.P[0] = Vector2(position + width / 2.f, area.size.y) + area.position;
    triangle.P[1] = Vector2(position - width / 2.f, area.size.y) + area.position;
    triangle.P[2] = Vector2(position, 0.f) + area.position;

    GL::Renderer::get()->setColor(ColorRGBA::BLACK);
    GL::Renderer::get()->fillTriangle(triangle);

    UI::Widget::draw();

    renderer->popClipArea();
  }
}

void TimelineRuler::onButtonClicked(Widget& widget,
                                    const Vector2& point,
				    unsigned int button,
				    bool clicked)
{
  if (!clicked)
    return;

  const Vector2 localPoint = transformToLocal(point);

  bool snapping = false;

  GL::Context* context = GL::Context::get();

  if (context->isKeyDown(GL::Key::LSHIFT) || context->isKeyDown(GL::Key::RSHIFT))
    snapping = true;

  elapsed = timeline.getWindowStart() + localPoint.x / timeline.getSecondWidth();

  if (snapping)
    elapsed = floorf(elapsed + 0.5);

  timeChangedSignal.emit(*this);
}

void TimelineRuler::onDragMoved(Widget& widget, const Vector2& point)
{
  const Vector2 localPoint = transformToLocal(point);

  bool snapping = false;

  GL::Context* context = GL::Context::get();

  if (context->isKeyDown(GL::Key::LSHIFT) || context->isKeyDown(GL::Key::RSHIFT))
    snapping = true;

  elapsed = timeline.getWindowStart() + localPoint.x / timeline.getSecondWidth();

  if (snapping)
    elapsed = floorf(elapsed + 0.5);

  timeChangedSignal.emit(*this);
}

///////////////////////////////////////////////////////////////////////

TimelineTrack::TimelineTrack(Timeline& initTimeline, Effect& initEffect):
  timeline(initTimeline),
  effect(initEffect),
  mode(NOT_DRAGGING)
{
  const float em = UI::Renderer::get()->getDefaultEM();

  getButtonClickedSignal().connect(*this, &TimelineTrack::onButtonClicked);
  getDragBegunSignal().connect(*this, &TimelineTrack::onDragBegun);
  getDragMovedSignal().connect(*this, &TimelineTrack::onDragMoved);
  getDragEndedSignal().connect(*this, &TimelineTrack::onDragEnded);

  menu = new UI::Menu();
  menu->addItem(*new UI::Item("Rename"));
  menu->addItem(*new UI::Item("Move Up"));
  menu->addItem(*new UI::Item("Move Down"));
  menu->addItem(*new UI::Item("Delete"));

  setSize(Vector2(em * 2.f, em * 2.f));
  setDraggable(true);
}

Effect& TimelineTrack::getEffect(void) const
{
  return effect;
}

void TimelineTrack::draw(void) const
{
  const Rectangle& area = getGlobalArea();

  UI::Renderer* renderer = UI::Renderer::get();
  if (renderer->pushClipArea(area))
  {
    const float size = getHandleSize();
    const float offset = getHandleOffset();

    Rectangle effectArea;
    effectArea.size.set(size, area.size.y);
    effectArea.position.set(area.position.x + offset, area.position.y);

    ColorRGBA color;

    if (effect.isActive())
      color.set(0.1f, 0.7f, 0.1f, 1.f);
    else
      color.set(renderer->getWidgetColor(), 1.f);

    GL::Renderer::get()->setColor(color);
    GL::Renderer::get()->fillRectangle(effectArea);

    GL::Renderer::get()->setColor(ColorRGBA::BLACK);
    GL::Renderer::get()->setLineWidth(1.f / GL::Canvas::getCurrent()->getPhysicalHeight());
    GL::Renderer::get()->drawRectangle(effectArea);

    renderer->drawText(effectArea, effect.getName());

    const float em = renderer->getDefaultEM();

    Rectangle handleArea;
    handleArea.size.set(em, effectArea.size.y);

    handleArea.position = effectArea.position - Vector2(em, 0.f);
    renderer->drawHandle(handleArea, getState());

    handleArea.position = effectArea.position + Vector2(effectArea.size.x, 0.f);
    renderer->drawHandle(handleArea, getState());

    UI::Widget::draw();

    renderer->popClipArea();
  }
}

void TimelineTrack::onButtonClicked(Widget& widget,
		                    const Vector2& point,
		                    unsigned int button,
		                    bool clicked)
{
  if (!clicked || button != 1)
    return;

  menu->setPosition(point);
  menu->display();
}

void TimelineTrack::onDragBegun(Widget& widget, const Vector2& point)
{
  float position = transformToLocal(point).x;

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

void TimelineTrack::onDragMoved(Widget& widget, const Vector2& point)
{
  float position = transformToLocal(point).x;

  bool snapping = false;

  GL::Context* context = GL::Context::get();

  if (context->isKeyDown(GL::Key::LSHIFT) || context->isKeyDown(GL::Key::RSHIFT))
    snapping = true;

  const float offset = getHandleOffset();

  if (mode == DRAGGING_POSITION)
  {
    Time start = timeline.getWindowStart() +
                 (position - reference) / timeline.getSecondWidth();

    if (snapping)
      start = floor(start + 0.5);

    if (start >= 0.0)
      effect.setStartTime(start);
  }
  else if (mode == DRAGGING_START)
  {
    Time start = timeline.getWindowStart() +
                 (position - reference) / timeline.getSecondWidth();

    if (snapping)
      start = floor(start + 0.5);

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

    if (snapping)
      duration = floorf(duration + 0.5);

    if (duration >= 0.0)
      effect.setDuration(duration);
  }
}

void TimelineTrack::onDragEnded(Widget& widget, const Vector2& point)
{
  mode = NOT_DRAGGING;
}

float TimelineTrack::getHandleSize(void) const
{
  return effect.getDuration() * timeline.getSecondWidth();
}

float TimelineTrack::getHandleOffset(void) const
{
  return (effect.getStartTime() - timeline.getWindowStart()) * timeline.getSecondWidth();
}

///////////////////////////////////////////////////////////////////////

Timeline::Timeline(Show& initShow):
  show(initShow),
  parent(NULL),
  start(0.0),
  zoom(1.f),
  effectIndex(1)
{
  getAreaChangedSignal().connect(*this, &Timeline::onAreaChanged);

  UI::Layout* mainLayout = new UI::Layout(UI::VERTICAL);
  addChild(*mainLayout);

  ruler = new TimelineRuler(*this);
  ruler->getTimeChangedSignal().connect(*this, &Timeline::onTimeChanged);
  mainLayout->addChild(*ruler);  

  trackLayout = new UI::Layout(UI::VERTICAL, false);
  trackLayout->setBorderSize(0.f);
  mainLayout->addChild(*trackLayout, 0.f);

  timeScroller = new UI::Scroller(UI::HORIZONTAL);
  timeScroller->getValueChangedSignal().connect(*this, &Timeline::onValueChanged);
  mainLayout->addChild(*timeScroller);

  setParentEffect(NULL);
}

bool Timeline::createEffect(EffectType& type)
{
  Effect* effect = type.createEffect();
  if (!effect)
    return false;

  std::stringstream stream;
  stream << "Effect " << effectIndex++;
  effect->setName(stream.str());

  effect->setStartTime(start);
  effect->setDuration(10.0);
  show.addEffect(*effect);

  setParentEffect(parent);
  return true;
}

void Timeline::destroyEffect(void)
{
  TimelineTrack* track = dynamic_cast<TimelineTrack*>(getActive());
  if (track)
  {
    updateScroller();
  }
}

Time Timeline::getWindowStart(void) const
{
  return start;
}

void Timeline::setWindowStart(Time newStart)
{
  start = newStart;
  timeScroller->setValue(start);
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

  return em * 2.f * zoom;
}

Effect* Timeline::getParentEffect(void) const
{
  return parent;
}

void Timeline::setParentEffect(Effect* newEffect)
{
  while (!tracks.empty())
  {
    delete tracks.back();
    tracks.pop_back();
  }

  parent = newEffect;

  if (parent)
  {
    const Effect::List& effects = parent->getChildren();

    for (unsigned int i = 0;  i < effects.size();  i++)
    {
      TimelineTrack* track = new TimelineTrack(*this, *effects[i]);
      trackLayout->addChild(*track);
      tracks.push_back(track);
    }
  }
  else
  {
    const Show::EffectList& effects = show.getEffects();

    for (unsigned int i = 0;  i < effects.size();  i++)
    {
      TimelineTrack* track = new TimelineTrack(*this, *effects[i]);
      trackLayout->addChild(*track);
      tracks.push_back(track);
    }
  }

  updateScroller();
}

SignalProxy1<void, Timeline&> Timeline::getTimeChangedSignal(void)
{
  return timeChangedSignal;
}

void Timeline::draw(void) const
{
  const Rectangle& area = getGlobalArea();

  UI::Renderer* renderer = UI::Renderer::get();
  if (renderer->pushClipArea(area))
  {
    UI::Widget::draw();

    const Rectangle& layoutArea = trackLayout->getGlobalArea();

    const float width = renderer->getDefaultEM() / 3.f;
    const float position = (getTimeElapsed() - getWindowStart()) *
			   getSecondWidth();

    Rectangle markerArea;
    markerArea.position.set(layoutArea.position.x + position - width / 2.f,
                            layoutArea.position.y);
    markerArea.size.set(width, layoutArea.size.y);

    GL::Renderer::get()->setColor(ColorRGBA(0.3f, 0.3f, 0.3f, 0.7f));
    GL::Renderer::get()->fillRectangle(markerArea);

    renderer->popClipArea();
  }
}

void Timeline::updateScroller(void)
{
  Time duration;

  if (parent)
    duration = parent->getDuration();
  else
    duration = show.getDuration();

  Time visible = getVisibleDuration();

  timeScroller->setPercentage(visible / (duration + visible) / zoom);
  timeScroller->setValueRange(0.f, duration + visible);
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

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
