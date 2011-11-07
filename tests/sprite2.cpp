
#include <wendy/Wendy.h>

#include <cstdlib>

namespace
{

using namespace wendy;

class Test
{
public:
  ~Test();
  bool init();
  void run();
private:
  ResourceIndex index;
  Ref<render::SharedProgramState> state;
  Ptr<render::GeometryPool> pool;
  Ref<render::Material> material;
  render::Technique* technique;
};

Test::~Test()
{
  material = NULL;
  pool = NULL;

  GL::Context::destroySingleton();
}

bool Test::init()
{
  if (!index.addSearchPathAlt(Path("media"), Path("../media")))
    return false;

  if (!GL::Context::createSingleton(index, GL::WindowConfig("2D Sprite Test")))
  {
    logError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::getSingleton();

  state = new render::SharedProgramState();
  state->reserveSupported(*context);
  context->setCurrentSharedProgramState(state);

  pool = new render::GeometryPool(*context);

  Path path("sprite2.material");

  material = render::Material::read(*context, path);
  if (!material)
  {
    logError("Failed to load material \'%s\'", path.asString().c_str());
    return false;
  }

  technique = material->findBestTechnique(render::Technique::FORWARD);
  if (!technique)
  {
    logError("Material \'%s\' has no forward technique", path.asString().c_str());
    return false;
  }

  return true;
}

void Test::run()
{
  GL::Context& context = pool->getContext();

  const render::PassList& passes = technique->getPasses();

  Timer timer;
  timer.start();

  do
  {
    context.clearColorBuffer(vec4(0.2f, 0.2f, 0.2f, 1.f));
    context.clearDepthBuffer();

    render::Sprite2 sprite;
    sprite.position = vec2(4.f / 3.f / 2.f, 0.5f);
    sprite.size = vec2(0.5f, 0.5f);
    sprite.angle = (float) timer.getTime();

    state->setModelMatrix(mat4());
    state->setViewMatrix(mat4());
    state->setOrthoProjectionMatrix(4.f / 3.f, 1.f);

    for (render::PassList::const_iterator p = passes.begin();  p != passes.end();  p++)
    {
      p->apply();
      sprite.render(*pool);
    }
  }
  while (context.update());
}

} /*namespace*/

int main()
{
  Ptr<Test> test(new Test());
  if (!test->init())
  {
    logError("Failed to initialize test");
    std::exit(EXIT_FAILURE);
  }

  test->run();
  test = NULL;

  std::exit(EXIT_SUCCESS);
}

