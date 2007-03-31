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
#include <wendy/UIBook.h>
#include <wendy/UICanvas.h>
#include <wendy/UIScroller.h>
#include <wendy/UISlider.h>
#include <wendy/UILayout.h>
#include <wendy/UIButton.h>
#include <wendy/UILabel.h>
#include <wendy/UIItem.h>
#include <wendy/UIList.h>
#include <wendy/UIMenu.h>
#include <wendy/UIPopup.h>

#include <wendy/DemoParameter.h>
#include <wendy/DemoEffect.h>
#include <wendy/DemoShow.h>
#include <wendy/DemoTimeline.h>
#include <wendy/DemoEditor.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;


///////////////////////////////////////////////////////////////////////

void Editor::render(void)
{
  Time currentTime = timer.getTime();

  timeline->setTimeElapsed(currentTime);
  timeDisplay->setText("%u:%02u.%02u",
                       (unsigned int) currentTime / 60,
		       (unsigned int) currentTime % 60,
		       (unsigned int) (currentTime * 100.0) % 100);

  show->setTimeElapsed(currentTime);
  show->prepare();

  GL::ScreenCanvas screen;

  if (visible)
  {
    screen.begin();
    screen.clearColorBuffer();
    UI::Widget::drawRoots();
    screen.end();

    if (canvas->isVisible())
    {
      canvas->getCanvas().begin();
      show->render();
      canvas->getCanvas().end();
    }
  }
  else
  {
    screen.begin();
    show->render();
    screen.end();
  }
}

bool Editor::isVisible(void) const
{
  return visible;
}

void Editor::setVisible(bool newState)
{
  visible = newState;

  GL::Context* context = GL::Context::get();

  if (visible)
    context->setTitle(show->getTitle() + " - Wendy Editor");
  else
    context->setTitle(show->getTitle());

  book->setVisible(visible);
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
  context->getResizedSignal().connect(*this, &Editor::onResized);
  context->getKeyPressedSignal().connect(*this, &Editor::onKeyPressed);

  book = new UI::Book();

  UI::Page* timelinePage = new UI::Page("Timeline View");
  book->addChild(*timelinePage);

  {
    UI::Layout* mainLayout = new UI::Layout(UI::VERTICAL);
    timelinePage->addChild(*mainLayout);

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

    UI::Label* label;
    
    label = new UI::Label("Zoom");
    commandLayout->addChild(*label);

    UI::Slider* zoomSlider = new UI::Slider();
    zoomSlider->setValueRange(1.f, 4.f);
    zoomSlider->setValue(1.f);
    zoomSlider->getValueChangedSignal().connect(*this, &Editor::onZoomChanged);
    commandLayout->addChild(*zoomSlider);

    label = new UI::Label("Parent");
    commandLayout->addChild(*label);

    parentPopup = new UI::Popup();
    parentPopup->getItemSelectedSignal().connect(*this, &Editor::onParentSelected);
    commandLayout->addChild(*parentPopup);

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

    UI::Layout* timelineLayout = new UI::Layout(UI::VERTICAL, false);
    timelineLayout->setBorderSize(3.f);
    mainLayout->addChild(*timelineLayout, 0.f);

    timeDisplay = new UI::Label();
    timeDisplay->setTextAlignment(UI::CENTERED_ON_X);
    timelineLayout->addChild(*timeDisplay);

    timeline = new Timeline(*show);
    timeline->getTimeChangedSignal().connect(*this, &Editor::onTimeChanged);
    timeline->getParentChangedSignal().connect(*this, &Editor::onParentChanged);
    timelineLayout->addChild(*timeline, 0.f);
  }

  UI::Page* shaderPage = new UI::Page("Shader Editor");
  book->addChild(*shaderPage);

  {
    UI::Layout* mainLayout = new UI::Layout(UI::VERTICAL);
    shaderPage->addChild(*mainLayout);

    UI::Label* stupidLabel = new UI::Label("Stuff here");
    mainLayout->addChild(*stupidLabel, 0.f);
  }

  UI::Page* aboutPage = new UI::Page("About Wendy");
  book->addChild(*aboutPage);

  {
    UI::Layout* mainLayout = new UI::Layout(UI::VERTICAL);
    aboutPage->addChild(*mainLayout);

    UI::Label* aboutLabel = new UI::Label();
    mainLayout->addChild(*aboutLabel, 0.f);

    aboutLabel->setText("The Wendy demo system\n"
                        "Copyright (c) 2007 Camilla Berglund <elmindreda@users.sourceforge.net>\n"
                        "\n"
			"Compiled " __TIME__ " on " __DATE__ "\n");
  }

  timer.start();
  timer.pause();
  timer.setTime(0.0);

  onResized(context->getWidth(), context->getHeight());
  onParentChanged(*timeline);

  setVisible(false);
  return true;
}

void Editor::onCreateEffect(UI::Button& button)
{
  UI::Item* typeItem = effectType->getItem(effectType->getSelection());
  if (!typeItem)
    return;

  EffectType* type = EffectType::findInstance(typeItem->asString());
  if (!type)
    return;

  timeline->createEffect(*type);
}

void Editor::onZoomChanged(UI::Slider& slider)
{
  timeline->setZoom(slider.getValue());
}

void Editor::onResized(unsigned int width, unsigned int height)
{
  book->setSize(Vector2(width, height));
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

void Editor::onTimeChanged(Timeline& timeline)
{
  timer.setTime(timeline.getTimeElapsed());
}

void Editor::onParentChanged(Timeline& timeline)
{
  parentPopup->destroyItems();

  Effect* effect = &(timeline.getParentEffect());

  do
  {
    parentPopup->addItem(*new UI::Item(effect->getName()));
  }
  while (effect = effect->getParent());
}

void Editor::onParentSelected(UI::Popup& popup, unsigned int index)
{
  UI::Item* item = popup.getItem(index);
  Effect* effect = Effect::findInstance(item->asString());
  if (!effect)
    return;

  timeline->setParentEffect(*effect);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
