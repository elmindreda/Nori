
#include <wendy/Wendy.h>

#include <cstdlib>

using namespace wendy;

int main()
{
  ResourceIndex index;

  if (!GL::Context::createSingleton(index, GL::WindowConfig("Clear screen")))
  {
    logError("Failed to create OpenGL context");
    std::exit(EXIT_FAILURE);
  }

  GL::Context* context = GL::Context::getSingleton();

  do
  {
    context->clearColorBuffer(vec4(0.2f, 0.2f, 0.2f, 1.f));
  }
  while (context->update());

  GL::Context::destroySingleton();

  std::exit(EXIT_SUCCESS);
}

