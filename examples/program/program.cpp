
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

class Demo : public Trackable
{
public:
  ~Demo(void);
  bool init(void);
  void run(void);
private:
  bool render(void);
  render::Camera camera;
  scene::Graph graph;
  scene::MeshNode* meshNode;
  scene::CameraNode* cameraNode;
  Timer timer;
  Time currentTime;
};

Demo::~Demo(void)
{
  GL::Renderer::destroy();
  GL::Context::destroy();
}

bool Demo::init(void)
{
  Image::addSearchPath(Path("../media"));
  Mesh::addSearchPath(Path("../media"));
  GL::Texture::addSearchPath(Path("../media"));
  GL::VertexProgram::addSearchPath(Path("../media"));
  GL::FragmentProgram::addSearchPath(Path("../media"));
  GL::Program::addSearchPath(Path("../media"));
  render::Material::addSearchPath(Path("../media"));

  GL::ContextMode mode(640, 480, 32, 16, 0, 0, GL::ContextMode::WINDOWED);
  if (!GL::Context::create(mode))
    return false;

  GL::Context* context = GL::Context::get();
  context->setTitle("Program");

  if (!GL::Renderer::create(*GL::Context::get()))
    return false;

  Ref<render::Mesh> mesh = render::Mesh::readInstance("cube");
  if (!mesh)
  {
    Log::writeError("Failed to load mesh");
    return false;
  }

  meshNode = new scene::MeshNode();
  meshNode->setMesh(mesh);
  graph.addNode(*meshNode);

  camera.setFOV(60.f);
  camera.setAspectRatio(0.f);

  cameraNode = new scene::CameraNode();
  cameraNode->setCameraName(camera.getName());
  cameraNode->getLocalTransform().position.z = mesh->getBounds().radius * 3.f;
  graph.addNode(*cameraNode);

  timer.start();

  return true;
}

void Demo::run(void)
{
  do
  {
    currentTime = timer.getTime();

    meshNode->getLocalTransform().rotation.setAxisRotation(Vec3(0.f, 1.f, 0.f),
							   (float) currentTime);

    graph.setTimeElapsed(currentTime);

    GL::Renderer* renderer = GL::Renderer::get();

    renderer->clearDepthBuffer();
    renderer->clearColorBuffer(ColorRGBA(0.2f, 0.2f, 0.2f, 1.f));

    render::Queue queue(camera);
    graph.enqueue(queue);
    queue.render();
  }
  while (GL::Context::get()->update());
}

int main()
{
  if (!wendy::initialize())
    exit(1);

  Ptr<Demo> demo = new Demo();
  if (demo->init())
    demo->run();

  demo = NULL;

  wendy::shutdown();
  exit(0);
}

