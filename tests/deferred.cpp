
#include <wendy/Wendy.h>

#include <cstdlib>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/constants.hpp>

using namespace wendy;

namespace
{

class Test : public Trackable
{
public:
  ~Test();
  bool init();
  void run();
private:
  bool render();
  ResourceCache cache;
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

Test::~Test()
{
  graph.destroyRootNodes();

  camera = NULL;
  renderer = NULL;
  pool = NULL;

  input::Context::destroySingleton();
  GL::Context::destroySingleton();
}

bool Test::init()
{
  const char* mediaPath = std::getenv("WENDY_MEDIA_DIR");
  if (!mediaPath)
    mediaPath = WENDY_MEDIA_DIR;

  if (!cache.addSearchPath(Path(mediaPath)))
    return false;

  GL::WindowConfig wc("Deferred Rendering Test");
  wc.resizable = false;

  if (!GL::Context::createSingleton(cache, wc))
    return false;

  GL::Context* context = GL::Context::getSingleton();

  const unsigned int width = context->getDefaultFramebuffer().getWidth();
  const unsigned int height = context->getDefaultFramebuffer().getHeight();

  pool = new render::GeometryPool(*context);

  renderer = deferred::Renderer::create(*pool, deferred::Config(width, height));
  if (!renderer)
    return false;

  if (!input::Context::createSingleton(*context))
    return false;

  Ref<render::Model> model = render::Model::read(*context, Path("cube.model"));
  if (!model)
  {
    logError("Failed to read model");
    return false;
  }

  rootNode = new scene::Node();
  graph.addRootNode(*rootNode);

  RandomRange angle(0.f, pi<float>() * 2.f);
  RandomVolume axis(vec3(-1.f), vec3(1.f));
  RandomVolume position(vec3(-20.f, -2.f, -20.f), vec3(20.f, 2.f, 20.f));

  for (size_t i = 0;  i < 200;  i++)
  {
    scene::ModelNode* modelNode = new scene::ModelNode();
    modelNode->setModel(model);
    modelNode->setLocalPosition(position());
    modelNode->setLocalRotation(angleAxis(degrees(angle()), normalize(axis())));
    rootNode->addChild(*modelNode);
  }

  camera = new render::Camera();
  camera->setNearZ(0.5f);
  camera->setFarZ(500.f);
  camera->setFOV(60.f);
  camera->setAspectRatio(float(width) / float(height));

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  graph.addRootNode(*cameraNode);

  scene::LightNode* lightNode;
  render::LightRef light;

  light = new render::Light();
  light->setType(render::Light::POINT);
  light->setColor(vec3(1.f, 0.3f, 0.3f));
  light->setRadius(10.f);

  lightNode = new scene::LightNode();
  lightNode->setLocalPosition(vec3(-5.f, 4.f, 0.f));
  lightNode->setLight(light);
  graph.addRootNode(*lightNode);

  light = new render::Light();
  light->setType(render::Light::POINT);
  light->setColor(vec3(0.7f, 0.2f, 0.8f));
  light->setRadius(10.f);

  lightNode = new scene::LightNode();
  lightNode->setLocalPosition(vec3(5.f, 4.f, 0.f));
  lightNode->setLight(light);
  graph.addRootNode(*lightNode);

  input::Context::getSingleton()->setTarget(&controller);

  timer.start();

  return true;
}

void Test::run()
{
  render::Scene scene(*pool, render::Technique::DEFERRED);
  GL::Context& context = pool->getContext();

  do
  {
    currentTime = timer.getTime();

    rootNode->setLocalRotation(angleAxis(degrees(float(currentTime)), vec3(0.f, 1.f, 0.f)));
    cameraNode->setLocalTransform(controller.getTransform());

    graph.update();
    graph.enqueue(scene, *camera);

    context.clearDepthBuffer();
    context.clearColorBuffer();

    renderer->render(scene, *camera);

    scene.removeOperations();
    scene.detachLights();
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

