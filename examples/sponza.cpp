
#include <wendy/Wendy.h>

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
  input::SpectatorCamera controller;
  Ptr<deferred::Renderer> renderer;
  Ref<render::Camera> camera;
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

  input::Context::destroy();
  GL::Renderer::destroy();
  GL::Context::destroy();
}

bool Demo::init(void)
{
  Image::addSearchPath(Path("media/sponza"));
  Mesh::addSearchPath(Path("media/sponza"));
  GL::Texture::addSearchPath(Path("media/sponza"));
  render::Material::addSearchPath(Path("media/sponza"));

  Image::addSearchPath(Path("media/deferred"));
  GL::Texture::addSearchPath(Path("media/deferred"));
  GL::VertexProgram::addSearchPath(Path("media/deferred"));
  GL::FragmentProgram::addSearchPath(Path("media/deferred"));
  GL::Program::addSearchPath(Path("media/deferred"));

  if (!GL::Context::create(GL::ContextMode()))
    return false;

  GL::Context* context = GL::Context::get();
  context->setTitle("Sponza Atrium");

  const unsigned int width = context->getScreenCanvas().getWidth();
  const unsigned int height = context->getScreenCanvas().getHeight();

  if (!input::Context::create(*context))
    return false;

  input::Context::get()->getKeyPressedSignal().connect(*this, &Demo::onKeyPressed);
  input::Context::get()->getButtonClickedSignal().connect(*this, &Demo::onButtonClicked);

  if (!GL::Renderer::create(*context))
    return false;

  renderer = deferred::Renderer::create(*context, deferred::Config(width, height));
  if (!renderer)
    return false;

  GL::TextureRef distAttTexture = GL::Texture::readInstance("distatt");
  if (!distAttTexture)
    return false;

  Ref<render::Mesh> mesh = render::Mesh::readInstance("sponza");
  if (!mesh)
    return false;

  scene::MeshNode* meshNode = new scene::MeshNode();
  meshNode->setMesh(mesh);
  graph.addRootNode(*meshNode);

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

  input::Context::get()->setFocus(&controller);

  return true;
}

void Demo::run(void)
{
  GL::Context* context = GL::Context::get();

  render::Queue queue(*camera);

  do
  {
    const Time deltaTime = timer.getTime() - currentTime;
    currentTime += deltaTime;

    lightNode->getLocalTransform().position.y = sinf(currentTime) * 40.f + 45.f;

    controller.update(deltaTime);
    cameraNode->getLocalTransform() = controller.getTransform();

    graph.update();
    graph.enqueue(queue);

    context->clearDepthBuffer();
    context->clearColorBuffer(ColorRGBA::BLACK);

    renderer->render(queue);

    queue.destroyOperations();
    queue.detachLights();
  }
  while (!quitting && context->update());
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
    exit(1);

  Ptr<Demo> demo(new Demo());
  if (demo->init())
    demo->run();

  demo = NULL;

  wendy::shutdown();
  exit(0);
}

