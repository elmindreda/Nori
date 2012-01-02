
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
  Ref<render::GeometryPool> pool;
  Ref<deferred::Renderer> renderer;
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

  pool = render::GeometryPool::create(*context);

  renderer = deferred::Renderer::create(deferred::Config(width, height, *pool));
  if (!renderer)
    return false;

  if (!input::Context::createSingleton(*context))
    return false;

  Ref<render::Model> model = render::Model::read(*renderer, "cube.model");
  if (!model)
  {
    logError("Failed to read model");
    return false;
  }

  rootNode = new scene::Node();
  graph.addRootNode(*rootNode);

  RandomRange angle(0.f, pi<float>() * 2.f);
  RandomRange scale(0.5f, 2.f);
  RandomVolume axis(vec3(-1.f), vec3(1.f));
  RandomVolume position(vec3(-20.f, -2.f, -20.f), vec3(20.f, 2.f, 20.f));

  for (size_t i = 0;  i < 200;  i++)
  {
    scene::ModelNode* modelNode = new scene::ModelNode();
    modelNode->setModel(model);
    modelNode->setLocalPosition(position());
    modelNode->setLocalRotation(angleAxis(degrees(angle()), normalize(axis())));
    modelNode->setLocalScale(scale());
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
  Ref<render::Light> light;

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
  render::Scene scene(*pool);
  GL::Context& context = pool->getContext();

  do
  {
    currentTime = timer.getTime();

    rootNode->setLocalRotation(angleAxis(degrees(float(currentTime)), 0.f, 1.f, 0.f));
    cameraNode->setLocalTransform(controller.getTransform());

    graph.update();
    graph.enqueue(scene, *camera);

    context.clearBuffers();

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

