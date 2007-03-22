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

#include <wendy/DemoParameter.h>
#include <wendy/DemoEffect.h>
#include <wendy/DemoShow.h>
#include <wendy/DemoTimeline.h>

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

  setSize(Vector2(em * 2.f, em * 2.f));
}

void TimelineRuler::draw(void) const
{
  const Rectangle& area = getGlobalArea();

  UI::Renderer* renderer = UI::Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawFrame(area, getState());

    const unsigned int count = (unsigned int) timeline.getVisibleDuration();

    for (unsigned int i = 0;  i < count;  i++)
    {
    }

    UI::Widget::draw();

    renderer->popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

TimelineTrack::TimelineTrack(Timeline& initTimeline, Effect& initEffect):
  timeline(initTimeline),
  effect(initEffect),
  mode(NOT_DRAGGING)
{
  const float em = UI::Renderer::get()->getDefaultEM();

  getDragBegunSignal().connect(*this, &TimelineTrack::onDragBegun);
  getDragMovedSignal().connect(*this, &TimelineTrack::onDragMoved);
  getDragEndedSignal().connect(*this, &TimelineTrack::onDragEnded);

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
    renderer->drawFrame(area, UI::STATE_NORMAL);

    const float size = getHandleSize();
    const float offset = getHandleOffset();

    Rectangle effectArea;
    effectArea.size.set(size, area.size.y);
    effectArea.position.set(area.position.x + offset, area.position.y);

    renderer->drawFrame(effectArea, getState());
    renderer->drawText(effectArea, effect.getName());

    UI::Widget::draw();

    renderer->popClipArea();
  }
}

void TimelineTrack::onDragBegun(Widget& widget, const Vector2& point)
{
  Vector2 localPoint = transformToLocal(point);

  const float size = getHandleSize();
  const float offset = getHandleOffset();

  if (localPoint.x >= offset && localPoint.x < offset + size)
  {
    reference = localPoint.x - offset;
    mode = DRAGGING_POSITION;
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

  const float size = getHandleSize();

  if (mode == DRAGGING_POSITION)
  {
    Time start = timeline.getWindowStart() +
                 (position - reference) / timeline.getSecondWidth();

    if (snapping)
      start = floor(start);

    effect.setStartTime(start);
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
  elapsed(0.0),
  windowStart(0.0),
  zoom(1.f)
{
  UI::Layout* mainLayout = new UI::Layout(UI::VERTICAL);
  addChild(*mainLayout);

  ruler = new TimelineRuler(*this);
  mainLayout->addChild(*ruler);  

  trackLayout = new UI::Layout(UI::VERTICAL, false);
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

  effect->setDuration(10.0);
  show.addEffect(*effect);

  setParentEffect(parent);
  return true;
}

Time Timeline::getWindowStart(void) const
{
  return windowStart;
}

void Timeline::setWindowStart(Time newStart)
{
  windowStart = newStart;
  timeScroller->setValue(windowStart);
}

float Timeline::getZoom(void) const
{
  return zoom;
}

void Timeline::setZoom(float newZoom)
{
  zoom = newZoom;
}

Time Timeline::getTimeElapsed(void) const
{
  return elapsed;
}

void Timeline::setTimeElapsed(Time newTime)
{
  elapsed = newTime;
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

void Timeline::updateScroller(void)
{
  Time duration;

  if (parent)
    duration = parent->getDuration();
  else
    duration = show.getDuration();

  Time visible = getVisibleDuration();

  timeScroller->setPercentage(visible / (duration + visible));
  timeScroller->setValueRange(0.f, duration + visible);
}

void Timeline::onValueChanged(UI::Scroller& scroller)
{
  windowStart = scroller.getValue();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
