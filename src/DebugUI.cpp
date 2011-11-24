///////////////////////////////////////////////////////////////////////
// Wendy debug interface
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <wendy/Config.h>

#include <wendy/DebugUI.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace debug
  {

///////////////////////////////////////////////////////////////////////

Interface::Interface(input::Context& context, UI::Drawer& drawer):
  UI::Layer(context, drawer),
  root(NULL)
{
  root = new UI::Widget(*this);
  root->setArea(Rect(0.f, 0.f, 200.f, 400.f));
  addRootWidget(*root);

  UI::Layout* layout = new UI::Layout(*this, UI::VERTICAL, true);
  root->addChild(*layout);

  for (size_t i = 0;  i < LABEL_COUNT;  i++)
  {
    labels[i] = new UI::Label(*this);
    layout->addChild(*labels[i], 0.f);
  }
}

void Interface::update()
{
  GL::Stats* stats = getInputContext().getContext().getStats();

  if (stats)
  {
    labels[LABEL_FRAMERATE]->setText(format("%0.2f fps", stats->getFrameRate()).c_str());

    const GL::Stats::Frame& frame = stats->getFrame();

    labels[LABEL_PASSES]->setText(format("%u passes", frame.passCount).c_str());
    labels[LABEL_VERTICES]->setText(format("%u vertices", frame.vertexCount).c_str());
    labels[LABEL_POINTS]->setText(format("%u points", frame.pointCount).c_str());
    labels[LABEL_LINES]->setText(format("%u lines", frame.lineCount).c_str());
    labels[LABEL_TRIANGLES]->setText(format("%u triangles", frame.triangleCount).c_str());
  }
  else
  {
    for (size_t i = 0;  i < LABEL_COUNT;  i++)
      labels[i]->setText("No stats available");
  }
}

void Interface::draw()
{
  GL::Context& context = getInputContext().getContext();

  GL::Framebuffer& framebuffer = context.getCurrentFramebuffer();
  root->setSize(vec2(root->getWidth(), float(framebuffer.getHeight())));

  GL::Stats* stats = context.getStats();
  context.setStats(NULL);

  UI::Layer::draw();

  context.setStats(stats);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace debug*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
