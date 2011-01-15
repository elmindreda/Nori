
#include <wendy/Wendy.h>

#include <cstdlib>

using namespace wendy;

class Demo : public Trackable
{
public:
  Demo(void);
  ~Demo(void);
  bool init(void);
  void run(void);
private:
  void onKeyPressed(input::Key key, bool pressed);
  void onButtonClicked(input::Button button, bool clicked);
  ResourceIndex index;
  input::SpectatorCamera controller;
  Ptr<render::GeometryPool> pool;
  Ref<render::Camera> camera;
  Ptr<deferred::Renderer> renderer;
  scene::Graph graph;
  scene::CameraNode* cameraNode;
  scene::LightNode* lightNode;
  Timer timer;
  Time currentTime;
  bool quitting;
  bool debugging;
};

Demo::Demo(void):
  quitting(false),
  debugging(false),
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

  const unsigned int width = context->getScreenCanvas().getWidth();
  const unsigned int height = context->getScreenCanvas().getHeight();

  if (!input::Context::createSingleton(*context))
    return false;

  input::Context::getSingleton()->getKeyPressedSignal().connect(*this, &Demo::onKeyPressed);
  input::Context::getSingleton()->getButtonClickedSignal().connect(*this, &Demo::onButtonClicked);

  pool = new render::GeometryPool(*context);

  renderer = deferred::Renderer::create(*pool, deferred::Config(width, height));
  if (!renderer)
    return false;

  Ref<GL::Texture> distAttTexture = GL::Texture::read(*context, Path("attenuation.texture"));
  if (!distAttTexture)
    return false;

  Ref<render::Model> model = render::Model::read(*context, Path("sponza.model"));
  if (!model)
    return false;

  scene::ModelNode* modelNode = new scene::ModelNode();
  modelNode->setModel(model);
  graph.addRootNode(*modelNode);

  camera = new render::Camera();
  camera->setFOV(60.f);
  camera->setDepthRange(0.9f, 500.f);
  camera->setAspectRatio((float) width / height);

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  graph.addRootNode(*cameraNode);

  render::LightRef light = new render::Light();
  light->setType(render::Light::POINT);
  light->setRadius(100.f);
  light->setDistAttTexture(distAttTexture);

  lightNode = new scene::LightNode();
  lightNode->setLight(light);
  graph.addRootNode(*lightNode);

  timer.start();

  input::Context::getSingleton()->setFocus(&controller);

  return true;
}

void Demo::run(void)
{
  render::Queue queue(*pool, *camera);
  GL::Context& context = pool->getContext();

  do
  {
    const Time deltaTime = timer.getTime() - currentTime;
    currentTime += deltaTime;

    lightNode->getLocalTransform().position.y = sinf(currentTime) * 40.f + 45.f;

    controller.update(deltaTime);
    cameraNode->getLocalTransform() = controller.getTransform();

    graph.update();
    graph.enqueue(queue);

    context.clearDepthBuffer();
    context.clearColorBuffer(ColorRGBA::BLACK);

    renderer->render(queue);
    renderer->renderAmbientLight(queue.getCamera(), ColorRGB(0.2f, 0.2f, 0.2f));

    queue.removeOperations();
    queue.detachLights();
  }
  while (!quitting && context.update());
}

void Demo::onKeyPressed(input::Key key, bool pressed)
{
  if (!pressed)
    return;

  switch (key)
  {
    case input::Key::TAB:
      debugging = !debugging;
      break;

    case input::Key::ESCAPE:
      quitting = true;
      break;
  }
}

void Demo::onButtonClicked(input::Button button, bool clicked)
{
  if (!clicked)
    return;

  if (button == input::Button::LEFT)
  {
    // TODO: Write screenshot to disk
  }
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

