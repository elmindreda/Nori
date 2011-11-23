
#include <wendy/Wendy.h>

#include <cstdlib>

namespace
{

using namespace wendy;

class Test : public Trackable
{
public:
  ~Test();
  bool init();
  void run();
private:
  void onContextResized(unsigned int width, unsigned int height);
  ResourceCache cache;
  Ref<render::SharedProgramState> state;
  Ptr<render::GeometryPool> pool;
  Ref<render::Material> material;
  Ref<render::Camera> camera;
  Ptr<forward::Renderer> renderer;
};

Test::~Test()
{
  material = NULL;
  pool = NULL;

  GL::Context::destroySingleton();
}

bool Test::init()
{
  const char* mediaPath = std::getenv("WENDY_MEDIA_DIR");
  if (!mediaPath)
    mediaPath = WENDY_MEDIA_DIR;

  if (!cache.addSearchPath(Path(mediaPath)))
    return false;

  if (!GL::Context::createSingleton(cache, GL::WindowConfig("3D Sprite Test")))
  {
    logError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::getSingleton();
  context->getResizedSignal().connect(*this, &Test::onContextResized);

  state = new render::SharedProgramState();
  state->reserveSupported(*context);
  context->setCurrentSharedProgramState(state);

  pool = new render::GeometryPool(*context);

  renderer = forward::Renderer::create(*pool, forward::Config());
  if (!renderer)
  {
    logError("Failed to create forward renderer");
    return false;
  }

  Path path("sprite3.material");

  material = render::Material::read(*context, path);
  if (!material)
  {
    logError("Failed to load material \'%s\'",
             path.asString().c_str());
    return false;
  }

  GL::Framebuffer& framebuffer = context->getCurrentFramebuffer();

  camera = new render::Camera();
  camera->setAspectRatio((float) framebuffer.getWidth() / framebuffer.getHeight());

  return true;
}

void Test::run()
{
  render::Scene scene(*pool, render::Technique::FORWARD);
  GL::Context& context = pool->getContext();

  render::Sprite3 sprite;
  sprite.size = vec2(1.f);
  sprite.material = material;
  sprite.type = render::STATIC_SPRITE;

  Transform3 transform;
  transform.position.z = -1.f;

  Timer timer;
  timer.start();

  do
  {
    context.clearColorBuffer(vec4(0.2f, 0.2f, 0.2f, 1.f));
    context.clearDepthBuffer();

    sprite.angle = timer.getTime();

    sprite.enqueue(scene, *camera, transform);

    renderer->render(scene, *camera);

    scene.removeOperations();
    scene.detachLights();
  }
  while (context.update());
}

void Test::onContextResized(unsigned int width, unsigned int height)
{
  GL::Context* context = GL::Context::getSingleton();
  context->setViewportArea(Recti(0, 0, width, height));

  camera->setAspectRatio(float(width) / float(height));
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

