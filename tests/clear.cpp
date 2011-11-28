
#include <wendy/WendyCore.h>
#include <wendy/WendyGL.h>

#include <cstdlib>

using namespace wendy;

namespace
{

void onContextResized(unsigned int width, unsigned int height)
{
  GL::Context* context = GL::Context::getSingleton();
  context->setViewportArea(Recti(0, 0, width, height));
}

} /*namespace*/

int main()
{
  ResourceCache cache;

  if (!GL::Context::createSingleton(cache, GL::WindowConfig("Clear screen")))
  {
    logError("Failed to create OpenGL context");
    std::exit(EXIT_FAILURE);
  }

  GL::Context* context = GL::Context::getSingleton();
  context->getResizedSignal().connect(onContextResized);

  do
  {
    context->clearColorBuffer(vec4(0.2f, 0.2f, 0.2f, 1.f));
  }
  while (context->update());

  GL::Context::destroySingleton();

  std::exit(EXIT_SUCCESS);
}

