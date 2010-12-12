
#include <wendy/Wendy.h>

#include <cstdlib>

using namespace wendy;

class Demo : public Trackable
{
public:
  ~Demo(void);
  bool init(void);
  void run(void);
private:
  bool render(void);
  ResourceIndex index;
  input::MayaCamera controller;
  Ptr<deferred::Renderer> renderer;
  Ref<render::Camera> camera;
  scene::Graph graph;
  scene::Node* rootNode;
  scene::CameraNode* cameraNode;
  Timer timer;
  Time currentTime;
};

Demo::~Demo(void)
{
  graph.destroyRootNodes();

  camera = NULL;
  renderer = NULL;

  input::Context::destroy();
  render::GeometryPool::destroy();
  GL::Context::destroy();
}

bool Demo::init(void)
{
  index.addSearchPath(Path("../media"));
  index.addSearchPath(Path("media"));

  if (!GL::Context::createSingleton(index))
    return false;

  GL::Context* context = GL::Context::get();
  context->setTitle("Deferred Rendering");

  const unsigned int width = context->getScreenCanvas().getWidth();
  const unsigned int height = context->getScreenCanvas().getHeight();

  if (!render::GeometryPool::createSingleton(*context))
    return false;

  renderer = deferred::Renderer::create(*context, deferred::Config(width, height));
  if (!renderer)
    return false;

  if (!input::Context::createSingleton(*context))
    return false;

  Ref<render::Mesh> mesh = render::Mesh::read(*context, Path("deferred/cube.mesh"));
  if (!mesh)
  {
    Log::writeError("Failed to read mesh");
    return false;
  }

  rootNode = new scene::Node();
  graph.addRootNode(*rootNode);

  RandomRange angle(0.f, M_PI * 2.f);
  RandomVolume axis(Vec3(-1.f, -1.f, -1.f), Vec3(1.f, 1.f, 1.f));
  RandomVolume position(Vec3(-20.f, -2.f, -20.f), Vec3(20.f, 2.f, 20.f));

  for (size_t i = 0;  i < 200;  i++)
  {
    scene::MeshNode* meshNode = new scene::MeshNode();
    meshNode->setMesh(mesh);
    meshNode->getLocalTransform().position = position.generate();
    meshNode->getLocalTransform().rotation.setAxisRotation(axis.generate().normalized(),
                                                           angle.generate());
    rootNode->addChild(*meshNode);
  }

  Ref<GL::Texture> distAttTexture = GL::Texture::read(*context, Path("deferred/distatt.texture"));
  if (!distAttTexture)
    return false;

  camera = new render::Camera();
  camera->setDepthRange(0.5f, 500.f);
  camera->setFOV(60.f);
  camera->setAspectRatio((float) width / height);

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  graph.addRootNode(*cameraNode);

  scene::LightNode* lightNode;
  render::LightRef light;

  light = new render::Light();
  light->setType(render::Light::POINT);
  light->setColor(ColorRGB(1.f, 0.3f, 0.3f));
  light->setRadius(10.f);
  light->setDistAttTexture(distAttTexture);

  lightNode = new scene::LightNode();
  lightNode->getLocalTransform().position.set(-5.f, 4.f, 0.f);
  lightNode->setLight(light);
  graph.addRootNode(*lightNode);

  light = new render::Light();
  light->setType(render::Light::POINT);
  light->setColor(ColorRGB(0.7f, 0.2f, 0.8f));
  light->setRadius(10.f);
  light->setDistAttTexture(distAttTexture);

  lightNode = new scene::LightNode();
  lightNode->getLocalTransform().position.set(5.f, 4.f, 0.f);
  lightNode->setLight(light);
  graph.addRootNode(*lightNode);

  input::Context::get()->setFocus(&controller);

  timer.start();

  return true;
}

void Demo::run(void)
{
  GL::Context* context = GL::Context::get();

  render::Queue queue(*camera);

  do
  {
    currentTime = timer.getTime();

    rootNode->getLocalTransform().rotation.setAxisRotation(Vec3(0.f, 1.f, 0.f),
							   (float) currentTime);
    cameraNode->getLocalTransform() = controller.getTransform();

    graph.update();
    graph.enqueue(queue);

    context->clearDepthBuffer();
    context->clearColorBuffer(ColorRGBA::BLACK);

    renderer->render(queue);

    queue.removeOperations();
    queue.detachLights();
  }
  while (context->update());
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

