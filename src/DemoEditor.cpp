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
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLPass.h>

#include <wendy/RenderFont.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>
#include <wendy/UIWindow.h>
#include <wendy/UICanvas.h>
#include <wendy/UISlider.h>
#include <wendy/UILayout.h>
#include <wendy/UILabel.h>

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
  if (!Show::get())
  {
    Log::writeError("Cannot create editor without a show");
    return false;
  }

  GL::Context* context = GL::Context::get();
  context->getRenderSignal().connect(*this, &Editor::onRender);

  window = new UI::Window();
  window->setArea(Rectangle(0, 0, context->getWidth(), context->getHeight()));

  canvas = new UI::Canvas();
  canvas->setArea(window->getArea());
  canvas->getKeyPressedSignal().connect(*this, &Editor::onKeyPressed);
  window->addChild(*canvas);

  return true;
}

bool Editor::onRender(void)
{
  GL::ScreenCanvas screen;

  screen.begin();
  UI::Widget::renderRoots();
  screen.end();

  canvas->getCanvas().begin();
  Show::get()->render();
  canvas->getCanvas().end();

  return true;
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
    }
  }
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
