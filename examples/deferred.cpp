
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
  Ptr<render::GeometryPool> pool;
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
  pool = NULL;

  input::Context::destroySingleton();
  GL::Context::destroySingleton();
}

bool Demo::init(void)
{
  index.addSearchPath(Path("../media"));
  index.addSearchPath(Path("media"));

  if (!GL::Context::createSingleton(index))
    return false;

  GL::Context* context = GL::Context::getSingleton();
  context->setTitle("Deferred Rendering");

  const unsigned int width = context->getScreenCanvas().getWidth();
  const unsigned int height = context->getScreenCanvas().getHeight();

  pool = new render::GeometryPool(*context);

  renderer = deferred::Renderer::create(*pool, deferred::Config(width, height));
  if (!renderer)
    return false;

  if (!input::Context::createSingleton(*context))
    return false;

  Ref<render::Model> model = render::Model::read(*context, Path("cube_deferred.mesh"));
  if (!model)
  {
    logError("Failed to read model");
    return false;
  }

  rootNode = new scene::Node();
  graph.addRootNode(*rootNode);

  RandomRange angle(0.f, M_PI * 2.f);
  RandomVolume axis(Vec3(-1.f, -1.f, -1.f), Vec3(1.f, 1.f, 1.f));
  RandomVolume position(Vec3(-20.f, -2.f, -20.f), Vec3(20.f, 2.f, 20.f));

  for (size_t i = 0;  i < 200;  i++)
  {
    scene::ModelNode* modelNode = new scene::ModelNode();
    modelNode->setModel(model);
    modelNode->getLocalTransform().position = position.generate();
    modelNode->getLocalTransform().rotation.setAxisRotation(axis.generate().normalized(),
                                                            angle.generate());
    rootNode->addChild(*modelNode);
  }

  Ref<GL::Texture> distAttTexture = GL::Texture::read(*context, Path("attenuation.texture"));
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

  input::Context::getSingleton()->setFocus(&controller);

  timer.start();

  return true;
}

void Demo::run(void)
{
  render::Queue queue(*pool, *camera);
  GL::Context& context = pool->getContext();

  do
  {
    currentTime = timer.getTime();

    rootNode->getLocalTransform().rotation.setAxisRotation(Vec3(0.f, 1.f, 0.f),
							   (float) currentTime);
    cameraNode->getLocalTransform() = controller.getTransform();

    graph.update();
    graph.enqueue(queue);

    context.clearDepthBuffer();
    context.clearColorBuffer(ColorRGBA::BLACK);

    renderer->render(queue);

    queue.removeOperations();
    queue.detachLights();
  }
  while (context.update());
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

