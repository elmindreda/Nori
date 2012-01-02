
#include <wendy/Wendy.h>

#include <cstdlib>
#include <sstream>

using namespace wendy;

namespace
{

class Demo : public Trackable, public input::Target
{
public:
  Demo();
  ~Demo();
  bool init();
  void run();
private:
  void onKeyPressed(input::Key key, bool pressed);
  void onButtonClicked(input::Button button, bool clicked);
  void onCursorMoved(const ivec2& position);
  ResourceCache cache;
  GL::Stats stats;
  input::SpectatorController controller;
  Ref<render::GeometryPool> pool;
  Ref<render::Camera> camera;
  Ref<deferred::Renderer> renderer;
  Ref<UI::Drawer> drawer;
  Ref<debug::Interface> interface;
  scene::Graph graph;
  scene::CameraNode* cameraNode;
  scene::LightNode* lightNode;
  Timer timer;
  Time currentTime;
  ivec2 lastPosition;
  bool quitting;
  bool drawdebug;
};

Demo::Demo():
  quitting(false),
  drawdebug(false),
  currentTime(0.0),
  cameraNode(NULL)
{
}

Demo::~Demo()
{
  graph.destroyRootNodes();

  interface = NULL;
  drawer = NULL;
  camera = NULL;
  renderer = NULL;
  pool = NULL;

  input::Context::destroySingleton();
  GL::Context::destroySingleton();
}

bool Demo::init()
{
  const char* mediaPath = std::getenv("WENDY_MEDIA_DIR");
  if (!mediaPath)
    mediaPath = WENDY_MEDIA_DIR;

  if (!cache.addSearchPath(Path(mediaPath)))
    return false;

  if (!cache.addSearchPath(Path(mediaPath) + "sponza"))
    return false;

  GL::WindowConfig wc;
  wc.title = "Sponza Atrium";
  wc.resizable = false;

  if (!GL::Context::createSingleton(cache, wc))
    return false;

  GL::Context* context = GL::Context::getSingleton();
  context->setStats(&stats);

  const unsigned int width = context->getDefaultFramebuffer().getWidth();
  const unsigned int height = context->getDefaultFramebuffer().getHeight();

  if (!input::Context::createSingleton(*context))
    return false;

  pool = render::GeometryPool::create(*context);

  renderer = deferred::Renderer::create(deferred::Config(width, height, *pool));
  if (!renderer)
    return false;

  Ref<render::Model> model = render::Model::read(*renderer, "sponza.model");
  if (!model)
    return false;

  scene::ModelNode* modelNode = new scene::ModelNode();
  modelNode->setModel(model);
  graph.addRootNode(*modelNode);

  camera = new render::Camera();
  camera->setFOV(60.f);
  camera->setNearZ(0.9f);
  camera->setFarZ(500.f);
  camera->setAspectRatio((float) width / height);

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  graph.addRootNode(*cameraNode);

  Ref<render::Light> light = new render::Light();
  light->setType(render::Light::POINT);
  light->setRadius(100.f);

  lightNode = new scene::LightNode();
  lightNode->setLight(light);
  graph.addRootNode(*lightNode);

  drawer = UI::Drawer::create(*pool);
  if (!drawer)
    return false;

  timer.start();

  {
    input::Context* context = input::Context::getSingleton();

    interface = new debug::Interface(*context, *drawer);

    context->setTarget(this);
    context->captureCursor();

    lastPosition =  context->getCursorPosition();
  }

  controller.setSpeed(25.f);
  controller.setPosition(vec3(0.f, 10.f, 0.f));

  return true;
}

void Demo::run()
{
  render::Scene scene(*pool);
  scene.setAmbientIntensity(vec3(0.2f, 0.2f, 0.2f));

  GL::Context& context = pool->getContext();

  do
  {
    const Time deltaTime = timer.getTime() - currentTime;
    currentTime += deltaTime;

    lightNode->setLocalPosition(vec3(0.f, sinf((float) currentTime) * 40.f + 45.f, 0.f));

    controller.update(deltaTime);
    cameraNode->setLocalTransform(controller.getTransform());

    graph.update();
    graph.enqueue(scene, *camera);

    context.clearBuffers();

    renderer->render(scene, *camera);

    scene.removeOperations();
    scene.detachLights();

    interface->update();
    if (drawdebug)
      interface->draw();
  }
  while (!quitting && context.update());
}

void Demo::onKeyPressed(input::Key key, bool pressed)
{
  controller.inputKeyPress(key, pressed);

  GL::Context& context = pool->getContext();

  if (pressed)
  {
    switch (key)
    {
      case input::KEY_ESCAPE:
        quitting = true;
        return;
      case input::KEY_F1:
        drawdebug = !drawdebug;
        context.setSwapInterval(drawdebug ? 0 : 1);
        return;
    }
  }
}

void Demo::onButtonClicked(input::Button button, bool clicked)
{
  controller.inputButtonClick(button, clicked);

  if (clicked)
  {
    if (button == input::BUTTON_LEFT)
    {
      // TODO: Write screenshot to disk
    }
  }
}

void Demo::onCursorMoved(const ivec2& position)
{
  controller.inputCursorOffset(position - lastPosition);
  lastPosition = position;
}

} /*namespace*/

int main()
{
  Ptr<Demo> demo(new Demo());
  if (!demo->init())
  {
    logError("Failed to initialize demo");
    std::exit(EXIT_FAILURE);
  }

  demo->run();
  demo = NULL;

  std::exit(EXIT_SUCCESS);
}

