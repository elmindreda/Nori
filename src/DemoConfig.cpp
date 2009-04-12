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
#include <wendy/GLVertex.h>
#include <wendy/GLShader.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLState.h>
#include <wendy/GLPass.h>

#include <wendy/RenderFont.h>

#include <wendy/Input.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>
#include <wendy/UIWindow.h>
#include <wendy/UIScroller.h>
#include <wendy/UILayout.h>
#include <wendy/UIButton.h>
#include <wendy/UILabel.h>
#include <wendy/UIItem.h>
#include <wendy/UIMenu.h>
#include <wendy/UIPopup.h>

#include <wendy/DemoConfig.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace demo
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

ConfigDialog::ConfigDialog(void)
{
  UI::Layout* layout = new UI::Layout(UI::VERTICAL);
  addChild(*layout);

  UI::Button* button;

  button = new UI::Button("Demo");
  layout->addChild(*button, 0.f);

  button = new UI::Button("Die");
  layout->addChild(*button, 0.f);

  setVisible(false);
}

void ConfigDialog::request(Config& config)
{
  setVisible(true);

  while (GL::Context::get()->update())
  {
  }

  setVisible(false);
}

SignalProxy0<void> ConfigDialog::getRenderSignal(void)
{
  return renderSignal;
}

void ConfigDialog::onDemo(UI::Button& button)
{
}

void ConfigDialog::onDie(UI::Button& button)
{
}

///////////////////////////////////////////////////////////////////////

  } /*namespace demo*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
