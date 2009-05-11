
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
  /*
  Ptr<GL::VertexBuffer> vertexBuffer;
  Ref<render::Style> style;
  render::Technique* technique;
  */
  Timer timer;
  Time currentTime;
};

Demo::~Demo(void)
{
  /*
  vertexBuffer = NULL;
  style = NULL;
  */

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
  context->getRenderSignal().connect(*this, &Demo::render);

  if (!GL::Renderer::create(*GL::Context::get()))
    return false;

  /*
  vertexBuffer = GL::VertexBuffer::createInstance(4, GL::Vertex2fv::format);
  if (!vertexBuffer)
    return false;

  GL::Vertex2fv* vertices = (GL::Vertex2fv*) vertexBuffer->lock();

  vertices[0].position.set(-1.f, -1.f);
  vertices[1].position.set(-1.f,  1.f);
  vertices[2].position.set( 1.f,  1.f);
  vertices[3].position.set( 1.f, -1.f);

  vertexBuffer->unlock();

  style = render::Style::readInstance("program");
  if (!style)
    return false;

  technique = style->getActiveTechnique();
  if (!technique)
    return false;
  */

  /*
  GL::Light* light = new GL::Light();
  light->setIntensity(ColorRGB(1.f, 1.f, 1.f));
  light->setAmbience(ColorRGB(0.5f, 0.5f, 0.5f));
  light->setType(GL::Light::POINT);
  light->setRadius(8.f);

  render::LightNode* lightNode = new render::LightNode();
  lightNode->setLight(light);
  lightNode->getLocalTransform().position.z = 3.f;
  scene.addNode(*lightNode);
  */

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
  while (GL::Context::get()->update())
    ;
}

bool Demo::render(void)
{
  currentTime = timer.getTime();

  meshNode->getLocalTransform().rotation.setAxisRotation(Vec3(0.f, 1.f, 0.f),
                                                         currentTime);

  graph.setTimeElapsed(currentTime);

  GL::Renderer* renderer = GL::Renderer::get();

  renderer->clearDepthBuffer();
  renderer->clearColorBuffer(ColorRGBA(0.2f, 0.2f, 0.2f, 1.f));

  render::Queue queue(camera);
  graph.enqueue(queue);
  queue.render();

  /*
  renderer->begin2D(Vector2(1.f, 1.f));

  for (unsigned int i = 0;  i < technique->getPassCount();  i++)
  {
    const GL::Pass& pass = technique->getPass(i);
    if (!pass.getName().empty())
      continue;

    pass.apply();

    GL::PrimitiveRange range(GL::TRIANGLE_FAN, *vertexBuffer);

    renderer->setCurrentPrimitiveRange(range);
    renderer->render();
  }

  renderer->end();
  */

  return true;
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

