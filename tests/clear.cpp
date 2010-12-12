
#include <wendy/Wendy.h>

#include <cstdlib>

using namespace wendy;

namespace
{

ResourceIndex index;

bool init(void)
{
  if (!GL::Context::createSingleton(index))
  {
    Log::writeError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::get();
  context->setTitle("Clear screen");

  return true;
}

} /*namespace*/

int main(void)
{
  if (!wendy::initialize())
  {
    Log::writeError("Failed to initialize Wendy");
    std::exit(1);
  }

  if (!init())
  {
    wendy::shutdown();
    std::exit(1);
  }

  GL::Context* context = GL::Context::get();

  do
  {
    context->clearColorBuffer(ColorRGBA(0.2f, 0.2f, 0.2f, 1.f));
  }
  while (context->update());

  GL::Context::destroy();

  wendy::shutdown();
  std::exit(0);
}

