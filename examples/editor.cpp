
#include <wendy/Wendy.h>

using namespace wendy;

#include "editor.h"

Demo::~Demo(void)
{
  demo::Editor::destroy();

  renderer = NULL;

  input::Context::destroy();
  render::GeometryPool::destroy();
  GL::Context::destroy();
}

bool Demo::init(void)
{
  GL::VertexProgram::addSearchPath(Path("../media"));
  GL::FragmentProgram::addSearchPath(Path("../media"));
  GL::Program::addSearchPath(Path("../media"));

  Image::addSearchPath(Path("media"));
  Font::addSearchPath(Path("media"));
  Mesh::addSearchPath(Path("media"));
  GL::Texture::addSearchPath(Path("media"));
  GL::VertexProgram::addSearchPath(Path("media"));
  GL::FragmentProgram::addSearchPath(Path("media"));
  GL::Program::addSearchPath(Path("media"));
  render::Material::addSearchPath(Path("media"));
  demo::Show::addSearchPath(Path("."));

  if (!GL::Context::create(GL::ContextMode()))
    return false;

  GL::Context* context = GL::Context::get();

  if (!render::GeometryPool::create(*context))
  {
    Log::writeError("Unable to create OpenGL renderer");
    return false;
  }

  if (!input::Context::create(*context))
  {
    Log::writeError("Unable to create input context");
    return false;
  }

  renderer = UI::Renderer::createInstance(*render::GeometryPool::get());
  if (!renderer)
  {
    Log::writeError("Unable to create UI renderer");
    return false;
  }

  new demo::EffectTemplate<CubeEffect>("Shader cube");

  if (!demo::Editor::create("editor"))
  {
    Log::writeError("Unable to create demo editor");
    return false;
  }

  demo::Editor::get()->setVisible(true);
  return true;
}

CubeEffect::CubeEffect(demo::EffectType& type, const String& name):
  demo::Effect(type, name)
{
}

bool CubeEffect::init(void)
{
  render::Mesh* cube = render::Mesh::readInstance("cube");
  if (!cube)
    return false;

  render::Light* light;
  scene::LightNode* lightNode;

  light = new render::Light();
  light->setType(render::Light::DIRECTIONAL);
  light->setColor(ColorRGB::WHITE);

  lightNode = new scene::LightNode();
  lightNode->setLight(light);
  graph.addRootNode(*lightNode);

  light = new render::Light();
  light->setType(render::Light::DIRECTIONAL);
  light->setColor(ColorRGB(0.1f, 0.1f, 1.f));

  lightNode = new scene::LightNode();
  lightNode->setLight(light);
  lightNode->getLocalTransform().rotation.setAxisRotation(Vec3::Y, (float) M_PI / 4.f);
  graph.addRootNode(*lightNode);

  camera = new render::Camera();
  camera->setFOV(60.f);
  camera->setAspectRatio(0.f);

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  cameraNode->getLocalTransform().position.z = cube->getBounds().radius * 3.f;
  graph.addRootNode(*cameraNode);

  meshNode = new scene::MeshNode();
  meshNode->setMesh(cube);
  graph.addRootNode(*meshNode);

  return true;
}

void CubeEffect::render(void) const
{
  render::Queue queue(*camera);
  graph.enqueue(queue);
  queue.render();

  demo::Effect::render();
}

void CubeEffect::update(Time deltaTime)
{
  meshNode->getLocalTransform().rotation.setAxisRotation(Vec3(0.f, 1.f, 0.f), (float) getTimeElapsed());

  graph.update();
}

int main(int argc, char** argv)
{
  if (!wendy::initialize())
    exit(1);

  Ptr<Demo> demo(new Demo());
  if (demo->init())
  {
    demo::Editor::get()->setVisible(true);
    demo::Editor::get()->run();
  }

  demo = NULL;

  wendy::shutdown();
  exit(0);
}

