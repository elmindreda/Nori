
#include <wendy/Wendy.h>

#include <cstdlib>

namespace
{

using namespace wendy;

class Test
{
public:
  ~Test(void);
  bool init(void);
  void run(void);
private:
  ResourceIndex index;
  Ref<render::Material> material;
  render::Camera camera;
  Timer timer;
};

Test::~Test(void)
{
  material = NULL;

  render::GeometryPool::destroySingleton();
  GL::Context::destroySingleton();
}

bool Test::init(void)
{
  if (!index.addSearchPath(Path("../media")))
    return false;

  if (!GL::Context::createSingleton(index))
  {
    Log::writeError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::getSingleton();
  context->setTitle("Particles");

  if (!render::GeometryPool::createSingleton(*context))
  {
    Log::writeError("Failed to create OpenGL renderer");
    return false;
  }

  material = render::Material::read(*context, Path("sprite3.material"));
  if (!material)
  {
    Log::writeError("Failed to load material");
    return false;
  }

  timer.start();
  return true;
}

void Test::run(void)
{
  GL::Context* context = GL::Context::getSingleton();

  render::Sprite3 sprite; sprite.position.set(0.f, 0.f, -3.f);
  sprite.size.set(1.f, 1.f);
  sprite.material = material;
  sprite.type = render::STATIC_SPRITE;

  render::Queue queue(camera);

  do
  {
    context->clearColorBuffer(ColorRGBA(0.2f, 0.2f, 0.2f, 1.f));
    context->clearDepthBuffer();

    sprite.angle = timer.getTime();

    sprite.enqueue(queue, Transform3());
    queue.render();
    queue.removeOperations();
  }
  while (context->update());
}

} /*namespace*/

int main(void)
{
  if (!wendy::initialize())
  {
    Log::writeError("Failed to initialize Wendy");
    std::exit(EXIT_FAILURE);
  }

  Ptr<Test> test(new Test());
  if (!test->init())
  {
    Log::writeError("Failed to initialize test");
    std::exit(EXIT_FAILURE);
  }

  test->run();
  test = NULL;

  wendy::shutdown();
  std::exit(EXIT_SUCCESS);
}

