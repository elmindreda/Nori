
#include <wendy/Wendy.h>

using namespace wendy;

class Demo : public Trackable
{
public:
  ~Demo(void);
  bool init(void);
  void run(void);
private:
  bool render(void);
  input::MayaCamera controller;
  Ref<render::Camera> camera;
  scene::Graph graph;
  scene::CameraNode* cameraNode;
};

Demo::~Demo(void)
{
  input::Context::destroy();
  render::GeometryPool::destroy();
  GL::Context::destroy();
}

bool Demo::init(void)
{
  Image::addSearchPath(Path("media"));
  Mesh::addSearchPath(Path("media"));
  GL::Texture::addSearchPath(Path("media"));
  GL::VertexProgram::addSearchPath(Path("media"));
  GL::FragmentProgram::addSearchPath(Path("media"));
  GL::Program::addSearchPath(Path("media"));
  render::Material::addSearchPath(Path("media"));

  if (!GL::Context::create(GL::ContextMode()))
    return false;

  GL::Context* context = GL::Context::get();
  context->setTitle("Cube Map");

  if (!render::GeometryPool::create(*context))
    return false;

  if (!input::Context::create(*context))
    return false;

  input::Context::get()->setFocus(&controller);

  Ref<render::Mesh> mesh = render::Mesh::readInstance("cube");
  if (!mesh)
  {
    Log::writeError("Failed to load mesh");
    return false;
  }

  RandomRange angle(0.f, M_PI * 2.f);
  RandomVolume axis(Vec3(-1.f, -1.f, -1.f), Vec3(1.f, 1.f, 1.f));
  RandomVolume position(Vec3(-2.f, -2.f, -2.f), Vec3(2.f, 2.f, 2.f));

  for (size_t i = 0;  i < 20;  i++)
  {
    scene::MeshNode* meshNode = new scene::MeshNode();
    meshNode->setMesh(mesh);
    meshNode->getLocalTransform().position = position.generate();
    meshNode->getLocalTransform().rotation.setAxisRotation(axis.generate().normalized(),
                                                           angle.generate());
    graph.addRootNode(*meshNode);
  }

  camera = new render::Camera();
  camera->setFOV(60.f);

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  cameraNode->getLocalTransform().position.z = mesh->getBounds().radius * 3.f;
  graph.addRootNode(*cameraNode);

  return true;
}

void Demo::run(void)
{
  GL::Context* context = GL::Context::get();

  render::Queue queue(*camera);

  do
  {
    cameraNode->getLocalTransform() = controller.getTransform();
    graph.update();

    context->clearDepthBuffer();
    context->clearColorBuffer(ColorRGBA::BLACK);

    graph.enqueue(queue);
    queue.render();
    queue.removeOperations();
  }
  while (context->update());
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

