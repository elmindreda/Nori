
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

int main(int argc, char** argv)
{
  GL::ContextMode mode;
  mode.set(640, 480, 32, 0, 0, GL::ContextMode::WINDOWED);

  if (!initializeSystem())
    return 1;

  if (GL::Context::create(mode))
  {
    GL::Context::get()->setTitle("Simple");

    while (GL::Context::get()->update())
    {
      glClearColor(1.f, 0.f, 0.f, 0.f);
      glClear(GL_COLOR_BUFFER_BIT);
    }

    GL::Context::destroy();
  }

  shutdownSystem();
  return 0;
}

