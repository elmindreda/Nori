
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

int main(int argc, char** argv)
{
  if (!wendy::initialize())
    return 1;

  GL::ContextMode mode;
  mode.set(640, 480, 32, 0, 0, 0, GL::ContextMode::WINDOWED);

  if (GL::Context::create(mode))
  {
    GL::Context* context = GL::Context::get();
    context->setTitle("Simple");

    do
    {
      context->clearColorBuffer(ColorRGBA(1.f, 0.f, 0.f, 0.f));
    }
    while (context->update());

    GL::Context::destroy();
  }

  wendy::shutdown();
  return 0;
}

