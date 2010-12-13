
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
  Ptr<render::GeometryPool> pool;
  Ref<render::Camera> camera;
  scene::Graph graph;
  scene::MeshNode* meshNode;
  scene::CameraNode* cameraNode;
  Timer timer;
  Time currentTime;
};

Demo::~Demo(void)
{
  graph.destroyRootNodes();

  pool = NULL;

  GL::Context::destroySingleton();
}

bool Demo::init(void)
{
  if (!index.addSearchPath(Path("../media")))
    return false;

  if (!GL::Context::createSingleton(index))
    return false;

  GL::Context* context = GL::Context::getSingleton();
  context->setTitle("Program");

  pool = new render::GeometryPool(*context);

  Ref<render::Mesh> mesh = render::Mesh::read(*context, Path("cube.mesh"));
  if (!mesh)
  {
    Log::writeError("Failed to load mesh");
    return false;
  }

  meshNode = new scene::MeshNode();
  meshNode->setMesh(mesh);
  graph.addRootNode(*meshNode);

  camera = new render::Camera();
  camera->setFOV(60.f);
  camera->setAspectRatio(0.f);

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  cameraNode->getLocalTransform().position.z = mesh->getBounds().radius * 3.f;
  graph.addRootNode(*cameraNode);

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

    meshNode->getLocalTransform().rotation.setAxisRotation(Vec3(0.f, 1.f, 0.f),
							   (float) currentTime);

    graph.update();

    context.clearDepthBuffer();
    context.clearColorBuffer(ColorRGBA(0.2f, 0.2f, 0.2f, 1.f));

    graph.enqueue(queue);
    queue.render();
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

