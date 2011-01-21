
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
    logError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::getSingleton();
  context->setTitle("Clear screen");

  return true;
}

} /*namespace*/

int main(void)
{
  if (!wendy::initialize())
  {
    logError("Failed to initialize Wendy");
    std::exit(EXIT_FAILURE);
  }

  if (!init())
  {
    wendy::shutdown();
    std::exit(EXIT_FAILURE);
  }

  GL::Context* context = GL::Context::getSingleton();

  do
  {
    context->clearColorBuffer(ColorRGBA(0.2f, 0.2f, 0.2f, 1.f));
  }
  while (context->update());

  GL::Context::destroySingleton();

  wendy::shutdown();
  std::exit(EXIT_SUCCESS);
}

