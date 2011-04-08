
#include <wendy/Wendy.h>

#include <cstdlib>

#include <glm/gtx/quaternion.hpp>

using namespace wendy;

class Test : public Trackable
{
public:
  ~Test(void);
  bool init(void);
  void run(void);
private:
  bool render(void);
  ResourceIndex index;
  input::MayaCamera controller;
  Ptr<render::GeometryPool> pool;
  Ref<render::Camera> camera;
  Ptr<forward::Renderer> renderer;
  scene::Graph graph;
  scene::CameraNode* cameraNode;
};

Test::~Test(void)
{
  graph.destroyRootNodes();

  pool = NULL;

  input::Context::destroySingleton();
  GL::Context::destroySingleton();
}

bool Test::init(void)
{
  if (!index.addSearchPath(Path("../media")))
    return false;

  if (!GL::Context::createSingleton(index))
    return false;

  GL::Context* context = GL::Context::getSingleton();
  context->setTitle("Cube Map");

  if (!input::Context::createSingleton(*context))
    return false;

  input::Context::getSingleton()->setFocus(&controller);

  pool = new render::GeometryPool(*context);

  renderer = forward::Renderer::create(*pool, forward::Config());
  if (!renderer)
  {
    logError("Failed to create forward renderer");
    return false;
  }

  Path path("cube_cubemapped.model");

  Ref<render::Model> model = render::Model::read(*context, path);
  if (!model)
  {
    logError("Failed to load model \'%s\'", path.asString().c_str());
    return false;
  }

  RandomRange angle(0.f, float(PI) * 2.f);
  RandomVolume axis(vec3(-1.f), vec3(1.f));
  RandomVolume position(vec3(-2.f), vec3(2.f));

  for (size_t i = 0;  i < 20;  i++)
  {
    scene::ModelNode* modelNode = new scene::ModelNode();
    modelNode->setModel(model);
    modelNode->getLocalTransform().position = position();
    modelNode->getLocalTransform().rotation = angleAxis(degrees(angle()),
                                                        normalize(axis()));
    graph.addRootNode(*modelNode);
  }

  camera = new render::Camera();
  camera->setFOV(60.f);

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  cameraNode->getLocalTransform().position.z = model->getBounds().radius * 3.f;
  graph.addRootNode(*cameraNode);

  return true;
}

void Test::run(void)
{
  render::Scene scene(*pool, render::Technique::FORWARD);
  GL::Context& context = pool->getContext();

  do
  {
    cameraNode->getLocalTransform() = controller.getTransform();
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

int main()
{
  if (!wendy::initialize())
    std::exit(EXIT_FAILURE);

  Ptr<Test> test(new Test());
  if (test->init())
    test->run();

  test = NULL;

  wendy::shutdown();
  std::exit(EXIT_SUCCESS);
}

