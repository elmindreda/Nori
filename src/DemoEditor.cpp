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
#include <wendy/UIBook.h>
#include <wendy/UICanvas.h>
#include <wendy/UIEntry.h>
#include <wendy/UIScroller.h>
#include <wendy/UISlider.h>
#include <wendy/UILayout.h>
#include <wendy/UIButton.h>
#include <wendy/UILabel.h>
#include <wendy/UIItem.h>
#include <wendy/UIList.h>
#include <wendy/UIMenu.h>
#include <wendy/UIPopup.h>

#include <wendy/DemoProperty.h>
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

void Editor::run(void)
{
  togglePaused();

  do
  {
    const Time duration = show->getDuration();

    elapsed = timer.getTime();

    if (simple && elapsed >= duration)
      break;

    timeline->setTimeElapsed(elapsed);
    timeDisplay->setText("%u:%02u.%02u / %u:%02u.%02u",
			 (unsigned int) elapsed / 60,
			 (unsigned int) elapsed % 60,
			 (unsigned int) (elapsed * 100.0) % 100,
			 (unsigned int) duration / 60,
			 (unsigned int) duration % 60,
			 (unsigned int) (duration * 100.0) % 100);

    show->setTimeElapsed(elapsed);
    show->prepare();

    GL::Context* context = GL::Context::get();
    context->clearColorBuffer();

    if (book->isVisible())
      desktop->drawRootWidgets();
    else
      show->render();

    if (quitting)
      break;
  }
  while (GL::Context::get()->update());
}

bool Editor::isModified(void) const
{
  return modified;
}

bool Editor::isPaused(void) const
{
  return timer.isPaused();
}

bool Editor::isVisible(void) const
{
  return book->isVisible();
}

void Editor::setVisible(bool newState)
{
  if (newState)
    simple = false;

  book->setVisible(newState);
  updateTitle();
}

Time Editor::getTimeElapsed(void) const
{
  return elapsed;
}

void Editor::setTimeElapsed(Time newTime)
{
  timer.setTime(newTime);
  elapsed = newTime;
  timeChangedSignal.emit();
}

const Show& Editor::getShow(void) const
{
  return *show;
}

SignalProxy0<void> Editor::getMusicChangedSignal(void)
{
  return musicChangedSignal;
}

SignalProxy0<void> Editor::getTimeChangedSignal(void)
{
  return timeChangedSignal;
}

SignalProxy0<void> Editor::getPausedSignal(void)
{
  return pausedSignal;
}

SignalProxy0<void> Editor::getResumedSignal(void)
{
  return resumedSignal;
}

bool Editor::create(const String& showName)
{
  if (get())
    return true;

  Ptr<Editor> editor(new Editor());
  if (!editor->init(showName))
    return false;

  set(editor.detachObject());
  return true;
}

Editor::Editor(void):
  simple(true),
  modified(false),
  quitting(false),
  book(NULL),
  elapsed(0.0)
{
}

bool Editor::init(const String& showName)
{
  if (showName.empty())
  {
    show = Show::createInstance();
    if (!show)
    {
      Log::writeError("Failed to create demo show");
      return false;
    }
  }
  else
  {
    show = Show::readInstance(showName);
    if (!show)
    {
      Log::writeError("Failed to load demo show \'%s\'", showName.c_str());
      return false;
    }
  }

  {
    input::Context* context = input::Context::get();
    context->getResizedSignal().connect(*this, &Editor::onResized);
    context->getKeyPressedSignal().connect(*this, &Editor::onKeyPressed);
  }

  GL::Context* context = GL::Context::get();
  context->getCloseRequestSignal().connect(*this, &Editor::onCloseRequest);

  const float em = UI::Renderer::get()->getDefaultEM();

  desktop = new UI::Desktop(*input::Context::get());

  book = new UI::Book();
  desktop->addRootWidget(*book);

  UI::Page* timelinePage = new UI::Page("Timeline Editor");
  book->addChild(*timelinePage);

  {
    UI::Layout* mainLayout = new UI::Layout(UI::VERTICAL);
    timelinePage->addChild(*mainLayout);

    UI::Layout* upperLayout = new UI::Layout(UI::HORIZONTAL, false);
    upperLayout->setBorderSize(3.f);
    mainLayout->addChild(*upperLayout, 0.f);

    canvas = new UI::Canvas();
    canvas->getKeyPressedSignal().connect(*this, &Editor::onKeyPressed);
    canvas->getDrawSignal().connect(*this, &Editor::onDrawShowCanvas);
    upperLayout->addChild(*canvas, 0.f);

    UI::Layout* controlLayout = new UI::Layout(UI::HORIZONTAL, false);
    controlLayout->setBorderSize(0.f);
    upperLayout->addChild(*controlLayout, 0.f);

    UI::Layout* commandLayout = new UI::Layout(UI::VERTICAL, false);
    commandLayout->setBorderSize(3.f);
    controlLayout->addChild(*commandLayout, 0.f);

    UI::Button* button;

    button = new UI::Button("Create Effect");
    button->getPushedSignal().connect(*this, &Editor::onCreateEffect);
    commandLayout->addChild(*button);

    button = new UI::Button("Destroy All Effects");
    button->getPushedSignal().connect(*this, &Editor::onDestroyAllEffects);
    commandLayout->addChild(*button);

    UI::Label* label;

    label = new UI::Label("Effect Name");
    commandLayout->addChild(*label);

    nameEntry = new UI::Entry();
    nameEntry->getKeyPressedSignal().connect(*this, &Editor::onKeyPressed);
    nameEntry->disable();
    commandLayout->addChild(*nameEntry);

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

    UI::Layout* playLayout = new UI::Layout(UI::HORIZONTAL, false);
    playLayout->setSize(Vec2(em * 2.f, em * 2.f));
    timelineLayout->addChild(*playLayout);

    UI::Slider* zoomSlider = new UI::Slider();
    zoomSlider->setSize(Vec2(em * 10.f, 0.f));
    zoomSlider->setValueRange(1.f, 10.f);
    zoomSlider->setValue(1.f);
    zoomSlider->getValueChangedSignal().connect(*this, &Editor::onZoomChanged);
    playLayout->addChild(*zoomSlider);

    timeDisplay = new UI::Label();
    timeDisplay->setTextAlignment(UI::CENTERED_ON_X);
    playLayout->addChild(*timeDisplay, 0.f);

    parentPopup = new UI::Popup();
    parentPopup->setSize(Vec2(em * 10.f, 0.f));
    parentPopup->getItemSelectedSignal().connect(*this, &Editor::onParentSelected);
    playLayout->addChild(*parentPopup);

    button = new UI::Button("Maali");
    button->setSize(Vec2(em * 6.f, 0.f));
    button->getPushedSignal().connect(*this, &Editor::onMaali);
    playLayout->addChild(*button);

    button = new UI::Button("||");
    button->setSize(Vec2(em * 4.f, 0.f));
    button->getPushedSignal().connect(*this, &Editor::onPauseResume);
    playLayout->addChild(*button);

    button = new UI::Button("|<");
    button->setSize(Vec2(em * 4.f, 0.f));
    button->getPushedSignal().connect(*this, &Editor::onRewind);
    playLayout->addChild(*button);

    timeline = new Timeline(*show);
    timeline->getTimeChangedSignal().connect(*this, &Editor::onTimeChanged);
    timeline->getParentChangedSignal().connect(*this, &Editor::onParentChanged);
    timeline->getEffectSelectedSignal().connect(*this, &Editor::onSelectionChanged);
    timelineLayout->addChild(*timeline, 0.f);
  }

  UI::Page* showPage = new UI::Page("Show Editor");
  book->addChild(*showPage);

  {
    UI::Layout* mainLayout = new UI::Layout(UI::VERTICAL);
    mainLayout->setBorderSize(3.f);
    showPage->addChild(*mainLayout);

    UI::Label* label;
    UI::Button* button;

    label = new UI::Label("Show Path: " + show->getSourcePath().asString());
    mainLayout->addChild(*label);

    button = new UI::Button("Load Show");
    button->getPushedSignal().connect(*this, &Editor::onLoadShow);
    mainLayout->addChild(*button);

    button = new UI::Button("Save Show");
    button->getPushedSignal().connect(*this, &Editor::onSaveShow);
    mainLayout->addChild(*button);

    label = new UI::Label("Show Title");
    mainLayout->addChild(*label);

    titleEntry = new UI::Entry();
    titleEntry->getKeyPressedSignal().connect(*this, &Editor::onKeyPressed);
    titleEntry->setText(show->getTitle());
    mainLayout->addChild(*titleEntry);

    label = new UI::Label("Music Path");
    mainLayout->addChild(*label);

    musicEntry = new UI::Entry();
    musicEntry->setText(show->getMusicPath().asString());
    mainLayout->addChild(*musicEntry);

    // Set aspect ratio(s)
    // Set default mode
  }

  UI::Page* aboutPage = new UI::Page("About Wendy");
  book->addChild(*aboutPage);

  {
    UI::Layout* mainLayout = new UI::Layout(UI::VERTICAL);
    aboutPage->addChild(*mainLayout);

    UI::Label* aboutLabel = new UI::Label();
    mainLayout->addChild(*aboutLabel, 0.f);

    aboutLabel->setText("The Wendy demo system, version " WENDY_VERSION "\n"
                        "Copyright (c) 2009 Camilla Berglund <elmindreda@elmindreda.org>\n"
			"\n"
			"expat - Copyright (c) 1998, 1999, 2000 Thai Open Source Software Center Ltd and Clark Cooper\n"
			"PCRE - Copyright (c) 1997-2008 University of Cambridge\n"
			"libpng - Copyright (c) 2004, 2006 Glenn Randers-Pehrson\n"
			"zlib - Copyright (c) 1995-1998 Jean-loup Gailly and Mark Adler\n"
			"GLFW - Copyright (c) 2006-2007 Camilla Berglund\n"
			"GLEW - Copyright (c) 2002-2006, Milan Ikits and Marcelo E. Magallon\n"
                        "\n"
			"Compiled " __TIME__ " on " __DATE__ "\n");
  }

  onResized(context->getScreenCanvas().getWidth(), context->getScreenCanvas().getHeight());
  onParentChanged(*timeline);

  updateTitle();

  timer.start();
  timer.pause();
  setTimeElapsed(0.0);

  canvas->activate();
  setVisible(false);

  return true;
}

void Editor::updateTitle(void)
{
  GL::Context* context = GL::Context::get();

  if (book->isVisible())
    context->setTitle(show->getTitle() + " - Wendy Editor");
  else
    context->setTitle(show->getTitle());
}

void Editor::togglePaused(void)
{
  if (timer.isPaused())
  {
    timer.resume();
    resumedSignal.emit();
  }
  else
  {
    timer.pause();
    pausedSignal.emit();
  }
}

bool Editor::onCloseRequest(void)
{
  quitting = true;
  return false;
}

void Editor::onMaali(UI::Button& button)
{
  setVisible(false);
}

void Editor::onLoadShow(UI::Button& button)
{
}

void Editor::onSaveShow(UI::Button& button)
{
  Path path = show->getSourcePath();
  if (path.asString().empty())
    path = Path("demo.show");

  Show::writeInstance(path, *show);
}

void Editor::onRewind(UI::Button& button)
{
  setTimeElapsed(0.0);
}

void Editor::onPauseResume(UI::Button& button)
{
  togglePaused();
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

void Editor::onDestroyAllEffects(UI::Button& button)
{
}

void Editor::onZoomChanged(UI::Slider& slider)
{
  timeline->setZoom(slider.getValue());
}

void Editor::onResized(unsigned int width, unsigned int height)
{
  book->setSize(Vec2((float) width, (float) height));
}

void Editor::onDrawShowCanvas(const UI::Canvas& canvas)
{
  GL::Context* context = GL::Context::get();
  GL::Renderer* renderer = GL::Renderer::get();

  Mat4 oldProjection = renderer->getProjectionMatrix();
  Rect oldViewport = context->getViewportArea();
  Rect oldScissor = context->getScissorArea();

  Vec2 scale(1.f / context->getCurrentCanvas().getWidth(),
             1.f / context->getCurrentCanvas().getHeight());

  Rect area = canvas.getGlobalArea() * scale;

  context->setScissorArea(area);
  context->setViewportArea(area);

  show->render();

  context->setScissorArea(oldScissor);
  context->setViewportArea(oldViewport);
  renderer->setProjectionMatrix(oldProjection);
}

void Editor::onKeyPressed(input::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case input::Key::TAB:
    {
      setVisible(!isVisible());
      break;
    }

    case input::Key::ESCAPE:
    {
      quitting = true;
      break;
    }
  }
}

void Editor::onKeyPressed(UI::Widget& widget, input::Key key, bool pressed)
{
  if (&widget == canvas)
  {
    if (!pressed)
      return;

    switch (key)
    {
      case input::Key::LEFT:
      {
	setTimeElapsed(elapsed - 1.0);
	break;
      }

      case input::Key::RIGHT:
      {
	setTimeElapsed(elapsed + 1.0);
	break;
      }

      case input::Key::HOME:
      {
	setTimeElapsed(0.0);
	break;
      }

      case input::Key::END:
      {
	setTimeElapsed(show->getDuration() - 0.01);
	break;
      }

      case input::Key::SPACE:
      {
	togglePaused();
	break;
      }
    }
  }
  else if (&widget == titleEntry)
  {
    if (!pressed || key != input::Key::ENTER)
      return;

    show->setTitle(titleEntry->getText());
    updateTitle();
  }
  else if (&widget == musicEntry)
  {
    if (!pressed || key != input::Key::ENTER)
      return;

    show->setMusicPath(Path(musicEntry->getText()));
    musicChangedSignal.emit();
  }
  else if (&widget == nameEntry)
  {
    if (!pressed || key != input::Key::ENTER)
      return;

    String effectName = nameEntry->getText();
    if (effectName.empty())
      return;

    Effect* selected = timeline->getSelectedEffect();
    if (!selected)
      return;

    selected->setName(effectName);
  }
}

void Editor::onTimeChanged(Timeline& timeline)
{
  setTimeElapsed(timeline.getTimeElapsed());
}

void Editor::onParentChanged(Timeline& timeline)
{
  parentPopup->destroyItems();

  Effect* effect = &(timeline.getParentEffect());

  do
  {
    parentPopup->addItem(effect->getName());
  }
  while ((effect = effect->getParent()) != NULL);
}

void Editor::onSelectionChanged(Timeline& timeline)
{
  Effect* selected = timeline.getSelectedEffect();
  if (selected)
  {
    nameEntry->setText(selected->getName());
    nameEntry->enable();
  }
  else
  {
    nameEntry->setText("");
    nameEntry->disable();
  }
}

void Editor::onParentSelected(UI::Popup& popup, unsigned int index)
{
  Effect* effect = Effect::findInstance(popup.getItemValue(index));
  if (!effect)
    return;

  timeline->setParentEffect(*effect);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
