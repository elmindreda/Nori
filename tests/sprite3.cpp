
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
  Ptr<render::GeometryPool> pool;
  Ref<render::Material> material;
  render::Camera camera;
  Timer timer;
};

Test::~Test(void)
{
  material = NULL;
  pool = NULL;

  GL::Context::destroySingleton();
}

bool Test::init(void)
{
  if (!index.addSearchPath(Path("../media")))
    return false;

  if (!GL::Context::createSingleton(index))
  {
    logError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::getSingleton();
  context->setTitle("Particles");

  pool = new render::GeometryPool(*context);

  material = render::Material::read(*context, Path("sprite3.material"));
  if (!material)
  {
    logError("Failed to load material");
    return false;
  }

  timer.start();
  return true;
}

void Test::run(void)
{
  render::Queue queue(*pool, camera);
  GL::Context& context = pool->getContext();

  render::Sprite3 sprite; sprite.position.set(0.f, 0.f, -3.f);
  sprite.size.set(1.f, 1.f);
  sprite.material = material;
  sprite.type = render::STATIC_SPRITE;

  do
  {
    context.clearColorBuffer(ColorRGBA(0.2f, 0.2f, 0.2f, 1.f));
    context.clearDepthBuffer();

    sprite.angle = timer.getTime();

    sprite.enqueue(queue, Transform3());
    queue.render();
    queue.removeOperations();
  }
  while (context.update());
}

} /*namespace*/

int main(void)
{
  if (!wendy::initialize())
  {
    logError("Failed to initialize Wendy");
    std::exit(EXIT_FAILURE);
  }

  Ptr<Test> test(new Test());
  if (!test->init())
  {
    logError("Failed to initialize test");
    std::exit(EXIT_FAILURE);
  }

  test->run();
  test = NULL;

  wendy::shutdown();
  std::exit(EXIT_SUCCESS);
}

