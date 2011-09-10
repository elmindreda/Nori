
#include <wendy/Wendy.h>

#include <cstdlib>

using namespace wendy;

class Demo : public Trackable, public input::Target
{
public:
  Demo(void);
  ~Demo(void);
  bool init(void);
  void run(void);
private:
  void onKeyPressed(input::Key key, bool pressed);
  void onButtonClicked(input::Button button, bool clicked);
  void onCursorMoved(const ivec2& position);
  ResourceIndex index;
  input::SpectatorController controller;
  Ptr<render::GeometryPool> pool;
  Ref<render::Camera> camera;
  Ptr<deferred::Renderer> renderer;
  scene::Graph graph;
  scene::CameraNode* cameraNode;
  scene::LightNode* lightNode;
  Timer timer;
  Time currentTime;
  ivec2 lastPosition;
  bool quitting;
};

Demo::Demo(void):
  quitting(false),
  currentTime(0.0),
  cameraNode(NULL)
{
}

Demo::~Demo(void)
{
  graph.destroyRootNodes();

  camera = NULL;
  renderer = NULL;
  pool = NULL;

  input::Context::destroySingleton();
  GL::Context::destroySingleton();
}

bool Demo::init(void)
{
  if (!index.addSearchPath(Path("../media")))
    return false;

  if (!index.addSearchPath(Path("../media/sponza")))
    return false;

  if (!GL::Context::createSingleton(index))
    return false;

  GL::Context* context = GL::Context::getSingleton();
  context->setTitle("Sponza Atrium");

  const unsigned int width = context->getDefaultFramebuffer().getWidth();
  const unsigned int height = context->getDefaultFramebuffer().getHeight();

  if (!input::Context::createSingleton(*context))
    return false;

  pool = new render::GeometryPool(*context);

  renderer = deferred::Renderer::create(*pool, deferred::Config(width, height));
  if (!renderer)
    return false;

  Ref<render::Model> model = render::Model::read(*context, Path("sponza.model"));
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

  render::LightRef light = new render::Light();
  light->setType(render::Light::POINT);
  light->setRadius(100.f);

  lightNode = new scene::LightNode();
  lightNode->setLight(light);
  graph.addRootNode(*lightNode);

  timer.start();

  {
    input::Context* context = input::Context::getSingleton();

    context->setTarget(this);
    context->captureCursor();

    lastPosition =  context->getCursorPosition();
  }

  return true;
}

void Demo::run(void)
{
  render::Scene scene(*pool, render::Technique::DEFERRED);
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

    context.clearDepthBuffer();
    context.clearColorBuffer();

    renderer->render(scene, *camera);

    scene.removeOperations();
    scene.detachLights();
  }
  while (!quitting && context.update());
}

void Demo::onKeyPressed(input::Key key, bool pressed)
{
  controller.inputKeyPress(key, pressed);

  if (pressed)
  {
    switch (key)
    {
      case input::KEY_ESCAPE:
        quitting = true;
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

int main()
{
  if (!wendy::initialize())
    std::exit(EXIT_FAILURE);

  Ptr<Demo> demo(new Demo());
  if (demo->init())
    demo->run();

  demo = NULL;

  wendy::shutdown();
  std::exit(EXIT_SUCCESS);
}

