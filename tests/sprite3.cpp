
#include <wendy/Wendy.h>

namespace
{

using namespace moira;
using namespace wendy;

class Test
{
public:
  ~Test(void);
  bool init(void);
  void run(void);
private:
  Ref<render::Material> material;
  render::Camera camera;
  Timer timer;
};

Test::~Test(void)
{
  material = NULL;

  GL::Renderer::destroy();

  GL::Context::destroy();
}

bool Test::init(void)
{
  if (!GL::Context::create(GL::ContextMode()))
  {
    Log::writeError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::get();
  context->setTitle("Sprite 3D");

  if (!GL::Renderer::create(*context))
  {
    Log::writeError("Failed to create OpenGL renderer");
    return false;
  }

  GL::VertexProgram::addSearchPath(Path("media"));
  GL::FragmentProgram::addSearchPath(Path("media"));
  GL::Program::addSearchPath(Path("media"));
  render::Material::addSearchPath(Path("media"));

  material = render::Material::readInstance("red3");
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
  GL::Context* context = GL::Context::get();

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
    queue.destroyOperations();
  }
  while (context->update());
}

} /*namespace*/

int main(void)
{
  if (!wendy::initialize())
  {
    Log::writeError("Failed to initialize Wendy");
    std::exit(1);
  }

  Ptr<Test> test(new Test());
  if (!test->init())
  {
    Log::writeError("Failed to initialize test");
    std::exit(1);
  }

  test->run();
  test = NULL;

  wendy::shutdown();
  std::exit(0);
}

