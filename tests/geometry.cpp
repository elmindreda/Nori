
#include <wendy/Wendy.h>

#include <cstdlib>

#include <glm/gtx/quaternion.hpp>

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
  ResourceIndex index;
  input::MayaCamera controller;
  Ptr<render::GeometryPool> pool;
  Ref<render::Camera> camera;
  Ptr<forward::Renderer> renderer;
  scene::Graph graph;
  scene::CameraNode* cameraNode;
};

Test::~Test()
{
  graph.destroyRootNodes();

  pool = NULL;

  input::Context::destroySingleton();
  GL::Context::destroySingleton();
}

bool Test::init()
{
  if (!index.addSearchPath(Path("../media")))
    return false;

  GL::ContextConfig cc;
  cc.glMajor = 3;
  cc.glMinor = 3;

  if (!GL::Context::createSingleton(index, GL::WindowConfig("Geometry Shader Test"), cc))
    return false;

  GL::Context* context = GL::Context::getSingleton();

  if (context->getGLVersionMajor() != cc.glMajor) {
    logError("OpenGL 3 is not supported");
    return false;
  }

  if (!input::Context::createSingleton(*context))
    return false;

  input::Context::getSingleton()->setTarget(&controller);

  pool = new render::GeometryPool(*context);

  renderer = forward::Renderer::create(*pool, forward::Config());
  if (!renderer)
  {
    logError("Failed to create forward renderer");
    return false;
  }

  Path path("cube_geometry.model");

  Ref<render::Model> model = render::Model::read(*context, path);
  if (!model)
  {
    logError("Failed to load model \'%s\'", path.asString().c_str());
    return false;
  }

  RandomRange angle(0.f, float(PI) * 2.f);
  RandomVolume axis(vec3(-1.f), vec3(1.f));

  scene::ModelNode* modelNode = new scene::ModelNode();
  modelNode->setModel(model);
  modelNode->setLocalPosition(vec3(0,0,0));
  modelNode->setLocalRotation(angleAxis(degrees(angle()), normalize(axis())));
  graph.addRootNode(*modelNode);

  camera = new render::Camera();
  camera->setFOV(60.f);

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  cameraNode->setLocalPosition(vec3(0.f, 0.f, model->getBounds().radius * 3.f));
  graph.addRootNode(*cameraNode);

  return true;
}

void Test::run()
{
  render::Scene scene(*pool, render::Technique::FORWARD);
  GL::Context& context = pool->getContext();

  do
  {
    cameraNode->setLocalTransform(controller.getTransform());
    graph.update();

    context.clearDepthBuffer();
    context.clearColorBuffer();

    graph.enqueue(scene, *camera);
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

