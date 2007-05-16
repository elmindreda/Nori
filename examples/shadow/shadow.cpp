
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
  Ptr<GL::RenderStyle> style;
  Ptr<GL::Mesh> mesh;
  GL::MeshNode* meshNode;
  GL::CameraNode* cameraNode;
  GL::LightNode* lightNode;
  Timer timer;
};

bool Demo::init(void)
{
  GL::ContextMode mode(640, 480, 32, 32, 0, 0, GL::ContextMode::WINDOWED);
  if (!GL::Context::create(mode))
    return false;

  GL::Context* context = GL::Context::get();
  context->setTitle("Shadow");
  context->getRenderSignal().connect(*this, &Demo::render);

  GL::Light* light = GL::Light::createInstance();
  light->setType(GL::Light::DIRECTIONAL);

  lightNode = new GL::LightNode();
  lightNode->setLightName(light->getName());
  scene.addRootNode(*lightNode);

  camera.setFOV(60.f);

  cameraNode = new GL::CameraNode();
  cameraNode->setCameraName(camera.getName());
  cameraNode->getLocalTransform().position.z = 5.f;
  scene.addRootNode(*cameraNode);

  style = new GL::RenderStyle();
  GL::RenderPass& pass = style->createPass();
  pass.setDiffuseColor(ColorRGBA(0.f, 0.f, 1.f, 1.f));
  pass.setLit(true);

  Mesh* meshData = Mesh::readInstance(Path("cube.mesh"));
  if (!meshData)
    return false;

  meshData->collapseGeometries(style->getName());
  meshData->calculateNormals(Mesh::SEPARATE_FACES);
  meshData->calculateEdges();

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
  GL::ScreenCanvas canvas;
  canvas.begin();
  canvas.clearDepthBuffer();
  canvas.clearColorBuffer();

  meshNode->getLocalTransform().rotation.setAxisRotation(Vector3(0.f, 1.f, 0.f), timer.getTime());

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

