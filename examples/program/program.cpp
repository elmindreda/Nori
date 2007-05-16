
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

class Demo : public Trackable
{
public:
  bool init(void);
  void run(void);
private:
  void applied(GL::ShaderPermutation& permutation);
  bool render(void);
  render::Camera camera;
  render::Scene scene;
  Ref<render::Mesh> mesh;
  render::MeshNode* meshNode;
  render::CameraNode* cameraNode;
  Timer timer;
  Time currentTime;
};

bool Demo::init(void)
{
  GL::ContextMode mode(640, 480, 32, 32, 0, 0, GL::ContextMode::WINDOWED);
  if (!GL::Context::create(mode))
    return false;

  GL::Context* context = GL::Context::get();
  context->setTitle("Program");
  context->getRenderSignal().connect(*this, &Demo::render);

  if (!GL::Renderer::create())
    return false;

  camera.setFOV(60.f);

  cameraNode = new render::CameraNode();
  cameraNode->setCameraName(camera.getName());
  cameraNode->getLocalTransform().position.z = 3.f;
  scene.addNode(*cameraNode);

  GL::Light* light = new GL::Light();
  light->setIntensity(ColorRGB(1.f, 1.f, 1.f));
  light->setAmbience(ColorRGB(0.5f, 0.5f, 0.5f));
  light->setType(GL::Light::POINT);
  //light->setRadius(8.f);

  render::LightNode* lightNode = new render::LightNode();
  lightNode->setLight(light);
  lightNode->getLocalTransform().position.z = 3.f;
  scene.addNode(*lightNode);

  mesh = render::Mesh::readInstance("cube");
  if (!mesh)
    return false;

  meshNode = new render::MeshNode();
  meshNode->setMesh(mesh);
  scene.addNode(*meshNode);

  if (GL::ShaderProgram* program = GL::ShaderProgram::findInstance("program"))
    program->getPermutationAppliedSignal().connect(*this, &Demo::applied);

  timer.start();

  return true;
}

void Demo::run(void)
{
  while (GL::Context::get()->update())
    ;
}

void Demo::applied(GL::ShaderPermutation& permutation)
{
  if (GL::ShaderUniform* uniform = permutation.getUniform("time"))
    uniform->setValue((float) currentTime);
}

bool Demo::render(void)
{
  currentTime = timer.getTime();

  meshNode->getLocalTransform().rotation.setAxisRotation(Vector3(0.f, 1.f, 0.f), currentTime);

  scene.setTimeElapsed(currentTime);

  GL::ScreenCanvas canvas;
  canvas.begin();
  canvas.clearDepthBuffer();
  canvas.clearColorBuffer();

  render::Queue queue(camera);
  scene.enqueue(queue);
  queue.render();

  canvas.end();
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

