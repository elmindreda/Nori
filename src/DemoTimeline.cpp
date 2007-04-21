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
	Variant::convertToString(digits, (index + (int) i) % 60);

	renderer->drawText(digitArea, digits);
      }
    }

    const Time timeOffset = timeline.getWindowStart() +
                            timeline.getParentEffect().getGlobalOffset();
    float position = (elapsed - timeOffset) * width;

    Triangle2 triangle;
    triangle.P[0] = Vector2(position + area.size.y / 2.f, area.size.y) + area.position;
    triangle.P[1] = Vector2(position - area.size.y / 2.f, area.size.y) + area.position;
    triangle.P[2] = Vector2(position, 0.f) + area.position;

    GL::Renderer::get()->setColor(ColorRGBA(0.8f, 0.1f, 0.1f, 1.f));
    GL::Renderer::get()->fillTriangle(triangle);
    GL::Renderer::get()->setColor(ColorRGBA::BLACK);
    GL::Renderer::get()->drawTriangle(triangle);

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

  const float position = transformToLocal(point).x;

  elapsed = timeline.getWindowStart() + position / timeline.getSecondWidth();
  elapsed = std::max(elapsed, 0.0);
  elapsed = timeline.getParentEffect().getGlobalOffset() +
            timeline.getSnappedTime(elapsed);

  timeChangedSignal.emit(*this);
}

void TimelineRuler::onDragMoved(Widget& widget, const Vector2& point)
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

  setSize(Vector2(em * 2.f, em * 2.f));
  setDraggable(true);
}

Effect& EffectTrack::getEffect(void) const
{
  return effect;
}

void EffectTrack::draw(void) const
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

void EffectTrack::onDragBegun(Widget& widget, const Vector2& point)
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

void EffectTrack::onDragMoved(Widget& widget, const Vector2& point)
{
  float position = transformToLocal(point).x;

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

void EffectTrack::onDragEnded(Widget& widget, const Vector2& point)
{
  mode = NOT_DRAGGING;
}

float EffectTrack::getHandleSize(void) const
{
  return effect.getDuration() * timeline.getSecondWidth();
}

float EffectTrack::getHandleOffset(void) const
{
  return (effect.getStartTime() - timeline.getWindowStart()) * timeline.getSecondWidth();
}

///////////////////////////////////////////////////////////////////////

Timeline::Timeline(Show& initShow):
  show(initShow),
  parent(NULL),
  selected(NULL),
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
  trackLayout->getButtonClickedSignal().connect(*this, &Timeline::onButtonClicked);
  mainLayout->addChild(*trackLayout, 0.f);

  timeScroller = new UI::Scroller(UI::HORIZONTAL);
  timeScroller->getValueChangedSignal().connect(*this, &Timeline::onValueChanged);
  mainLayout->addChild(*timeScroller);

  effectMenu = new UI::Menu();
  effectMenu->addItem(*new UI::Item("Enter", MENU_ENTER));
  effectMenu->addItem(*new UI::SeparatorItem());
  effectMenu->addItem(*new UI::Item("Rename", MENU_RENAME));
  effectMenu->addItem(*new UI::SeparatorItem());
  effectMenu->addItem(*new UI::Item("Move Up", MENU_MOVE_UP));
  effectMenu->addItem(*new UI::Item("Move Down", MENU_MOVE_DOWN));
  effectMenu->addItem(*new UI::SeparatorItem());
  effectMenu->addItem(*new UI::Item("Delete", MENU_DELETE));
  effectMenu->getItemSelectedSignal().connect(*this, &Timeline::onItemSelected);

  layoutMenu = new UI::Menu();
  layoutMenu->addItem(*new UI::Item("Exit", MENU_EXIT));
  layoutMenu->getItemSelectedSignal().connect(*this, &Timeline::onItemSelected);

  setParentEffect(show.getRootEffect());
}

bool Timeline::createEffect(EffectType& type)
{
  std::stringstream stream;
  stream << type.getName() << ' ' << effectIndex++;

  Effect* effect = type.createEffect(stream.str());
  if (!effect)
    return false;

  effect->setStartTime(start);
  effect->setDuration(10.0);
  parent->addChild(*effect);

  createTrack(*effect);
  return true;
}

void Timeline::destroyEffect(void)
{
  if (selected)
  {
    Effect* effect = &(selected->getEffect());

    delete selected;
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

Effect& Timeline::getParentEffect(void) const
{
  return *parent;
}

void Timeline::setParentEffect(Effect& newParent)
{
  if (parent == &newParent)
    return;

  //trackLayout->destroyChildren();
  selected = NULL;

  parent = &newParent;

  const Effect::List& effects = parent->getChildren();

  for (Effect::List::const_iterator i = effects.begin();  i != effects.end();  i++)
    createTrack(**i);

  updateScroller();

  parentChangedSignal.emit(*this);
}

Time Timeline::getSnappedTime(Time time) const
{
  GL::Context* context = GL::Context::get();

  if (context->isKeyDown(GL::Key::LSHIFT) || context->isKeyDown(GL::Key::RSHIFT))
    return floorf(time + 0.5);

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

void Timeline::draw(void) const
{
  const Rectangle& area = getGlobalArea();

  UI::Renderer* renderer = UI::Renderer::get();
  if (renderer->pushClipArea(area))
  {
    UI::Widget::draw();

    const Rectangle& layoutArea = trackLayout->getGlobalArea();

    const Time timeOffset = getWindowStart() + parent->getGlobalOffset();
    const float position = (getTimeElapsed() - timeOffset) * getSecondWidth();
    const float width = renderer->getDefaultEM() / 3.f;

    Rectangle markerArea;
    markerArea.position.set(layoutArea.position.x + position - width / 2.f,
                            layoutArea.position.y);
    markerArea.size.set(width, layoutArea.size.y);

    GL::Renderer::get()->setColor(ColorRGBA(0.3f, 0.3f, 0.3f, 0.5f));
    GL::Renderer::get()->fillRectangle(markerArea);

    renderer->popClipArea();
  }
}

void Timeline::updateScroller(void)
{
  Time visible = getVisibleDuration();
  Time duration = parent->getDuration();

  timeScroller->setPercentage(visible / (duration + visible) / zoom);
  timeScroller->setValueRange(0.f, duration + visible);
}

void Timeline::createTrack(Effect& effect)
{
  EffectTrack* track = new EffectTrack(*this, effect);
  track->getButtonClickedSignal().connect(*this, &Timeline::onButtonClicked);
  trackLayout->addChild(*track);
}

void Timeline::onButtonClicked(Widget& widget,
		               const Vector2& point,
		               unsigned int button,
		               bool clicked)
{
  if (&widget == trackLayout)
  {
    if (clicked && button == 1)
    {
      if (parent->getParent())
      {
	layoutMenu->setPosition(point);
	layoutMenu->display();
      }
    }
  }
  else if (EffectTrack* track = dynamic_cast<EffectTrack*>(&widget))
  {
    selected = track;

    if (clicked && button == 1)
    {
      effectMenu->setPosition(point);
      effectMenu->display();
    }
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

    case MENU_EXIT:
    {
      if (selected)
      {
	if (Effect* parent = selected->getEffect().getParent())
	  setParentEffect(*parent);
      }

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
