
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

namespace
{

bool render(void)
{
  GL::ContextCanvas canvas;

  canvas.push();
  canvas.clearColor(ColorRGBA(1.f, 0.f, 0.f, 0.f));
  canvas.pop();

  return true;
}

}

int main(int argc, char** argv)
{
  GL::ContextMode mode;
  mode.set(640, 480, 32, 0, 0, GL::ContextMode::WINDOWED);

  if (!initializeSystem())
    return 1;

  if (GL::Context::create(mode))
  {
    GL::Context* context = GL::Context::get();
    context->setTitle("Simple");
    context->getRenderSignal().connect(render);

    while (context->update())
      ;

    GL::Context::destroy();
  }

  shutdownSystem();
  return 0;
}

