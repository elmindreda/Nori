
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
  Ref<GL::ImageCanvas> canvas;
  Ref<GL::Texture> shadowmap;
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
  context->setTitle("Shadow Map");

  if (!GL::Renderer::create(*context))
    return false;

  Image data(PixelFormat::DEPTH24, 512, 512);

  shadowmap = GL::Texture::createInstance(*context, data, 0, "shadowmap");
  if (!shadowmap)
    return false;

  canvas = GL::ImageCanvas::createInstance(*context, shadowmap->getWidth(), shadowmap->getHeight());
  if (!canvas)
    return false;

  canvas->setDepthBuffer(&shadowmap->getImage());

  Ref<render::Mesh> mesh = render::Mesh::readInstance("hills");
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
  GL::Context* context = GL::Context::get();

  do
  {
    currentTime = timer.getTime();

    meshNode->getLocalTransform().rotation.setAxisRotation(Vec3(0.f, 1.f, 0.f),
							   (float) currentTime);

    graph.setTimeElapsed(currentTime);

    context->clearDepthBuffer();
    context->clearColorBuffer(ColorRGBA(0.2f, 0.2f, 0.2f, 1.f));

    render::Queue queue(camera);
    graph.enqueue(queue);
    queue.render();
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
