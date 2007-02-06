
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

namespace
{

bool onRender(void)
{
  GL::ScreenCanvas canvas;

  canvas.begin();
  canvas.clearColorBuffer(ColorRGBA(1.f, 0.f, 0.f, 0.f));
  canvas.end();

  return true;
}

}

int main(int argc, char** argv)
{
  GL::ContextMode mode;
  mode.set(640, 480, 32, 0, 0, GL::ContextMode::WINDOWED);

  if (!wendy::initialize())
    return 1;

  if (GL::Context::create(mode))
  {
    GL::Context* context = GL::Context::get();
    context->setTitle("Simple");
    context->getRenderSignal().connect(onRender);

    while (context->update())
      ;

    GL::Context::destroy();
  }

  wendy::shutdown();
  return 0;
}

