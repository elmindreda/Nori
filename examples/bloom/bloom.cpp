
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
  GL::RenderState bloomPass;
  Ref<GL::Texture> texture;
  Ptr<GL::TextureCanvas> canvas;
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
  context->setTitle("Bloom");

  if (!GL::Renderer::create(*context))
    return false;

  texture = GL::Texture::createInstance(Image(ImageFormat::RGB888, 32, 32), 0);
  if (!texture)
    return false;

  canvas = GL::TextureCanvas::createInstance(*context, 32, 32);
  if (!canvas)
    return false;

  canvas->setColorBufferTexture(texture);

  Ref<GL::Program> program = GL::Program::readInstance("bloom");
  if (!program)
    return false;

  bloomPass.setProgram(program);
  bloomPass.getSamplerState("image").setTexture(texture);

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
							   currentTime);

    graph.setTimeElapsed(currentTime);

    render::Queue queue(camera);
    graph.enqueue(queue);

    GL::Renderer* renderer = GL::Renderer::get();

    renderer->setCurrentCanvas(*canvas);
    renderer->clearDepthBuffer();
    renderer->clearColorBuffer(ColorRGBA(0.4f, 0.4f, 0.4f, 1.f));

    queue.render();

    renderer->setScreenCanvasCurrent();
    renderer->clearDepthBuffer();
    renderer->clearColorBuffer(ColorRGBA(0.2f, 0.2f, 0.2f, 1.f));

    queue.render();

    renderer->setProjectionMatrix2D(4.f, 4.f);

    bloomPass.apply();

    render::Sprite2 sprite;
    sprite.position.set(0.5f, 0.5f);
    sprite.render();
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

