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
#include <wendy/DemoEditor.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

TimelineTrack::TimelineTrack(Effect& initEffect):
  effect(initEffect)
{
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
    renderer->drawFrame(area, getState());
    renderer->drawText(area, effect.getName());

    UI::Widget::draw();

    renderer->popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

Timeline::Timeline(Show& initShow):
  show(initShow),
  parent(NULL),
  elapsed(0.0),
  windowStart(0.0),
  scale(1.f)
{
}

Time Timeline::getWindowStart(void) const
{
  return windowStart;
}

void Timeline::setWindowStart(Time newStart)
{
  windowStart = newStart;
  windowChangedSignal.emit(*this);
}

float Timeline::getScale(void) const
{
  return scale;
}

void Timeline::setScale(float newScale)
{
  scale = newScale;
  windowChangedSignal.emit(*this);
}

Time Timeline::getTimeElapsed(void) const
{
  return elapsed;
}

void Timeline::setTimeElapsed(Time newTime)
{
  elapsed = newTime;
}

Effect* Timeline::getParentEffect(void) const
{
  return parent;
}

void Timeline::setParentEffect(Effect* newEffect)
{
  parent = newEffect;

  while (!tracks.empty())
  {
    delete tracks.back();
    tracks.pop_back();
  }
}

void Timeline::update(void)
{
}

void Timeline::draw(void) const
{
  const Show::EffectList& effects = show.getEffects();

  const Rectangle& area = getGlobalArea();
      
  UI::Renderer* renderer = UI::Renderer::get();
  if (renderer->pushClipArea(area))
  {
    render::Font* font = renderer->getDefaultFont();

    const Show::EffectList& effects = show.getEffects();

    const float height = font->getHeight() * 2.f;

    for (unsigned int i = 0;  i < effects.size();  i++)
    {
      Rectangle effectArea;
      effectArea.position.set((effects[i]->getStartTime() - windowStart) * font->getWidth() * scale,
                              area.size.y - (i + 1) * height);
      effectArea.size.set(effects[i]->getDuration() * font->getWidth() * scale,
                          height);
      transformToGlobal(effectArea.position);

      renderer->drawFrame(effectArea, getState());
      renderer->drawText(effectArea, effects[i]->getName());
    }

    UI::Widget::draw();

    renderer->popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

bool Editor::isVisible(void) const
{
  return visible;
}

void Editor::setVisible(bool newState)
{
  visible = newState;

  GL::Context* context = GL::Context::get();

  if (visible)
  {
    window->show();
    context->setTitle(show->getTitle() + " - Wendy Editor");
  }
  else
  {
    window->hide();
    context->setTitle(show->getTitle());
  }
}

bool Editor::create(void)
{
  if (get())
    return true;

  Ptr<Editor> editor = new Editor();
  if (!editor->init())
    return false;

  set(editor.detachObject());
  return true;
}

Editor::Editor(void):
  visible(false)
{
}

bool Editor::init(void)
{
  show = Show::createInstance();
  if (!show)
    return false;

  show->setTitle("Demo");

  GL::Context* context = GL::Context::get();
  context->getRenderSignal().connect(*this, &Editor::onRender);
  context->getResizedSignal().connect(*this, &Editor::onResized);
  context->getKeyPressedSignal().connect(*this, &Editor::onKeyPressed);

  window = new UI::Window();
  window->setArea(Rectangle(0, 0, context->getWidth(), context->getHeight()));

  UI::Layout* mainLayout = new UI::Layout(UI::VERTICAL);
  window->addChild(*mainLayout);

  // Upper half
  {
    UI::Layout* upperLayout = new UI::Layout(UI::HORIZONTAL, false);
    upperLayout->setBorderSize(3.f);
    mainLayout->addChild(*upperLayout, 0.f);

    canvas = new UI::Canvas();
    canvas->getKeyPressedSignal().connect(*this, &Editor::onKeyPressed);
    upperLayout->addChild(*canvas, 0.f);

    UI::Layout* controlLayout = new UI::Layout(UI::HORIZONTAL, false);
    controlLayout->setBorderSize(0.f);
    upperLayout->addChild(*controlLayout, 0.f);

    UI::Layout* commandLayout = new UI::Layout(UI::VERTICAL, false);
    commandLayout->setBorderSize(3.f);
    controlLayout->addChild(*commandLayout, 0.f);

    UI::Button* button;
    
    button = new UI::Button("Maali");
    commandLayout->addChild(*button);

    button = new UI::Button("Create Effect");
    button->getPushedSignal().connect(*this, &Editor::onCreateEffect);
    commandLayout->addChild(*button);

    button = new UI::Button("Destroy Effect");
    button->getPushedSignal().connect(*this, &Editor::onDestroyEffect);
    commandLayout->addChild(*button);

    effectType = new UI::List();
    controlLayout->addChild(*effectType, 0.f);

    // Build effect type list
    {
      EffectType::InstanceList instances;
      EffectType::getInstances(instances);

      if (instances.empty())
      {
	Log::writeError("No point in running editor without effect types");
	return false;
      }

      for (unsigned int i = 0;  i < instances.size();  i++)
      {
	UI::Item* item = new UI::Item(instances[i]->getName());
	effectType->addItem(*item);
      }
    }
  }

  // Lower half
  {
    UI::Layout* timelineLayout = new UI::Layout(UI::VERTICAL, false);
    timelineLayout->setBorderSize(3.f);
    mainLayout->addChild(*timelineLayout, 0.f);

    timeSlider = new UI::Slider(UI::HORIZONTAL);
    timeSlider->getValueChangedSignal().connect(*this, &Editor::onTimeSlider);
    timelineLayout->addChild(*timeSlider);

    UI::Label* timeDisplay = new UI::Label();
    timeDisplay->setTextAlignment(UI::RIGHT_ALIGNED);
    timelineLayout->addChild(*timeDisplay);

    Timeline* timeline = new Timeline(*show);
    timelineLayout->addChild(*timeline, 0.f);
  }

  timer.start();
  timer.pause();
  timer.setTime(0.0);

  setVisible(false);
  return true;
}

bool Editor::onRender(void)
{
  Time currentTime = timer.getTime();

  timeSlider->setValue(currentTime);

  show->setTimeElapsed(currentTime);
  show->prepare();

  GL::ScreenCanvas screen;

  if (visible)
  {
    screen.begin();
    screen.clearColorBuffer();
    UI::Widget::drawRoots();
    screen.end();

    canvas->getCanvas().begin();
    show->render();
    canvas->getCanvas().end();
  }
  else
  {
    screen.begin();
    show->render();
    screen.end();
  }

  return true;
}

void Editor::onCreateEffect(UI::Button& button)
{
  UI::Item* typeItem = effectType->getItem(effectType->getSelection());
  if (!typeItem)
    return;

  EffectType* type = EffectType::findInstance(typeItem->getValue());
  if (!type)
    return;

  Effect* effect = type->createEffect();
  if (!effect)
    return;

  effect->setDuration(10.0);
  show->addEffect(*effect);

  timeSlider->setValueRange(0.f, show->getDuration());
}

void Editor::onDestroyEffect(UI::Button& button)
{
  const Show::EffectList& effects = show->getEffects();

  if (!effects.empty())
  {  
    Effect* effect = effects.back();
    show->removeEffect(*effect);
    delete effect;
  }
}

void Editor::onResized(unsigned int width, unsigned int height)
{
  window->setSize(Vector2(width, height));
}

void Editor::onKeyPressed(GL::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case GL::Key::TAB:
    {
      setVisible(!visible);
      break;
    }
  }
}

void Editor::onKeyPressed(UI::Widget& widget, GL::Key key, bool pressed)
{
  if (&widget == canvas)
  {
    if (!pressed)
      return;

    switch (key)
    {
      /*
      case GL::Key::TAB:
      {
	setVisible(false);
	break;
      }
      */

      case GL::Key::LEFT:
      {
	timer.setTime(timer.getTime() - 1.0);
	break;
      }

      case GL::Key::RIGHT:
      {
	timer.setTime(timer.getTime() + 1.0);
	break;
      }

      case GL::Key::HOME:
      {
	timer.setTime(0.0);
	break;
      }

      case GL::Key::END:
      {
	timer.setTime(show->getDuration() - 0.01);
	break;
      }

      case GL::Key::SPACE:
      {
	if (timer.isPaused())
	  timer.resume();
	else
	  timer.pause();
	break;
      }
    }
  }
}

void Editor::onTimeSlider(UI::Slider& slider)
{
  timer.setTime(slider.getValue());
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
