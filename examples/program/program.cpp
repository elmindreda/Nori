
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

class Demo : public Trackable
{
public:
  bool init(void);
private:
  bool render(void);
  GL::Camera camera;
  GL::Scene scene;
  Ptr<GL::Mesh> mesh;
  GL::MeshNode* meshNode;
  GL::CameraNode* cameraNode;
  Timer timer;
};

bool Demo::init(void)
{
  GL::ContextMode mode(640, 480, 32, 32, 0, GL::ContextMode::WINDOWED);
  if (!GL::Context::create(mode))
    return false;

  GL::Context* context = GL::Context::get();
  context->setTitle("Program");
  context->getRenderSignal().connect(*this, &Demo::render);

  if (!GL::Renderer::create())
    return false;

  camera.setFOV(60.f);

  cameraNode = new GL::CameraNode();
  cameraNode->setCameraName(camera.getName());
  cameraNode->getLocalTransform().position.z = 5.f;
  scene.addRootNode(*cameraNode);

  Mesh* meshData = Mesh::readInstance("cube");
  if (!meshData)
    return false;

  meshData->calculateNormals(Mesh::SEPARATE_FACES);

  mesh = GL::Mesh::createInstance(*meshData);
  if (!mesh)
    return false;

  meshNode = new GL::MeshNode();
  meshNode->setMeshName(mesh->getName());
  scene.addRootNode(*meshNode);

  timer.start();

  return true;
}

bool Demo::render(void)
{
  const Time time = timer.getTime();

  if (GL::ShaderProgram* program = GL::ShaderProgram::findInstance("program"))
  {
    if (GL::ShaderUniform* uniform = program->getUniform("time"))
      uniform->setValue((float) time);
  }

  GL::ScreenCanvas canvas;
  canvas.begin();
  canvas.clearDepthBuffer();
  canvas.clearColorBuffer();

  meshNode->getLocalTransform().rotation.setAxisRotation(Vector3(0.f, 1.f, 0.f), time);

  scene.updateTree();
  scene.renderTree(camera);

  canvas.end();
  return true;
}

int main()
{
  if (!wendy::initialize())
    exit(1);

  Ptr<Demo> demo = new Demo();
  if (demo->init())
  {
    while (GL::Context::get()->update())
      ;
  }
  else
  {
    Log::writeError("Punt");
    exit(1);
  }

  demo = NULL;

  wendy::shutdown();
  exit(0);
}

