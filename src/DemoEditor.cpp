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
#include <wendy/UISlider.h>
#include <wendy/UILayout.h>
#include <wendy/UIButton.h>
#include <wendy/UILabel.h>
#include <wendy/UIItem.h>
#include <wendy/UIMenu.h>
#include <wendy/UIPopup.h>

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

TimelineEffect::TimelineEffect(Effect& initEffect):
  effect(initEffect)
{
}

Effect& TimelineEffect::getEffect(void) const
{
  return effect;
}

void TimelineEffect::render(void) const
{
  const Rectangle& area = getGlobalArea();

  UI::Renderer* renderer = UI::Renderer::get();
  if (renderer->pushClipArea(area))
  {
    renderer->drawFrame(area, getState());
    renderer->drawText(area, effect.getName());

    UI::Widget::render();

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

void Timeline::render(void) const
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
    }

    UI::Widget::render();

    renderer->popClipArea();
  }
}

///////////////////////////////////////////////////////////////////////

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

Editor::Editor(void)
{
}

bool Editor::init(void)
{
  show = Show::createInstance();
  if (!show)
    return false;

  GL::Context* context = GL::Context::get();
  context->getRenderSignal().connect(*this, &Editor::onRender);
  context->getResizedSignal().connect(*this, &Editor::onResized);

  window = new UI::Window();
  window->setArea(Rectangle(0, 0, context->getWidth(), context->getHeight()));

  UI::Layout* mainLayout = new UI::Layout(UI::VERTICAL);
  window->addChild(*mainLayout);

  // Upper half
  {
    UI::Widget* upperPanel = new UI::Widget();
    mainLayout->addChild(*upperPanel, 0.f);

    UI::Layout* upperLayout = new UI::Layout(UI::HORIZONTAL);
    upperLayout->setBorderSize(3.f);
    upperPanel->addChild(*upperLayout);

    canvas = new UI::Canvas();
    canvas->getKeyPressedSignal().connect(*this, &Editor::onKeyPressed);
    upperLayout->addChild(*canvas, 0.f);

    commandPanel = new UI::Widget();
    upperLayout->addChild(*commandPanel, 0.f);

    UI::Layout* commandLayout = new UI::Layout(UI::VERTICAL);
    commandLayout->setBorderSize(3.f);
    commandPanel->addChild(*commandLayout);

    UI::Button* button;
    
    button = new UI::Button("Maali");
    commandLayout->addChild(*button);

    button = new UI::Button("Create Effect");
    button->getPushedSignal().connect(*this, &Editor::onCreateEffect);
    commandLayout->addChild(*button);

    button = new UI::Button("Destroy Effect");
    button->getPushedSignal().connect(*this, &Editor::onDestroyEffect);
    commandLayout->addChild(*button);

    effectType = new UI::Popup();
    commandLayout->addChild(*effectType);

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
	effectType->addItem(*new UI::Item(instances[i]->getName()));
    }
  }

  // Lower half
  {
    UI::Widget* timelinePanel = new UI::Widget();
    mainLayout->addChild(*timelinePanel, 0.f);

    UI::Layout* timelineLayout = new UI::Layout(UI::VERTICAL);
    timelineLayout->setBorderSize(3.f);
    timelinePanel->addChild(*timelineLayout);

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

  return true;
}

bool Editor::onRender(void)
{
  Time currentTime = timer.getTime();

  timeSlider->setValue(currentTime);

  show->setTimeElapsed(currentTime);
  show->prepare();

  GL::ScreenCanvas screen;

  screen.begin();
  screen.clearColorBuffer();
  UI::Widget::renderRoots();
  screen.end();

  canvas->getCanvas().begin();
  canvas->getCanvas().clearColorBuffer(ColorRGBA::BLACK);
  show->render();
  canvas->getCanvas().end();

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

void Editor::onKeyPressed(UI::Widget& widget, GL::Key key, bool pressed)
{
  if (&widget == canvas)
  {
    if (!pressed)
      return;

    switch (key)
    {
      case GL::Key::ESCAPE:
      {
	// TODO: Toggle editor mode.
	break;
      }

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
