
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

  material = render::Material::read(*context, Path("sprite2.material"));
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
  GL::Context& context = pool->getContext();

  do
  {
    context.clearColorBuffer(ColorRGBA(0.2f, 0.2f, 0.2f, 1.f));
    context.clearDepthBuffer();

    render::Sprite2 sprite;
    sprite.position.set(4.f / 3.f / 2.f, 0.5f);
    sprite.size.set(0.5f, 0.5f);
    sprite.angle = (float) timer.getTime();

    context.setOrthoProjectionMatrix(4.f / 3.f, 1.f);

    sprite.render(*pool, *material);
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

