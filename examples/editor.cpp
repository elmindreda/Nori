
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

#include "editor.h"

namespace
{

bool setup(void)
{
  Image::addSearchPath(Path("media"));
  Font::addSearchPath(Path("media"));
  Mesh::addSearchPath(Path("media"));
  GL::Texture::addSearchPath(Path("media"));
  GL::VertexProgram::addSearchPath(Path("media"));
  GL::FragmentProgram::addSearchPath(Path("media"));
  GL::Program::addSearchPath(Path("media"));
  render::Material::addSearchPath(Path("media"));
  demo::Show::addSearchPath(Path("."));

  if (!GL::Renderer::create(*GL::Context::get()))
  {
    Log::writeError("Unable to create OpenGL renderer");
    return false;
  }

  if (!input::Context::create(*GL::Context::get()))
  {
    Log::writeError("Unable to create input context");
    return false;
  }

  if (!UI::Renderer::create())
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

} /*namespace*/

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
  //light->setType(Light::DIRECTIONAL);
  light->setIntensity(ColorRGB::WHITE);

  lightNode = new scene::LightNode();
  lightNode->setLight(light);
  graph.addNode(*lightNode);

  light = new render::Light();
  //light->setType(Light::DIRECTIONAL);
  light->setIntensity(ColorRGB(0.1f, 0.1f, 1.f));

  lightNode = new scene::LightNode();
  lightNode->setLight(light);
  lightNode->getLocalTransform().rotation.setAxisRotation(Vec3::Y, (float) M_PI / 4.f);
  graph.addNode(*lightNode);

  camera = new render::Camera();
  camera->setFOV(60.f);
  camera->setAspectRatio(0.f);

  cameraNode = new scene::CameraNode();
  cameraNode->setCamera(camera);
  cameraNode->getLocalTransform().position.z = cube->getBounds().radius * 3.f;
  graph.addNode(*cameraNode);

  meshNode = new scene::MeshNode();
  meshNode->setMesh(cube);
  graph.addNode(*meshNode);

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

  graph.setTimeElapsed(getTimeElapsed());
}

int main(int argc, char** argv)
{
  if (!wendy::initialize())
    exit(1);

  GL::ContextMode mode(800, 600, 32, 16, 0, 0, GL::ContextMode::WINDOWED);
  if (GL::Context::create(mode))
  {
    if (setup())
      demo::Editor::get()->run();

    demo::Editor::destroy();
    UI::Renderer::destroy();
    input::Context::destroy();
    GL::Renderer::destroy();
    GL::Context::destroy();
  }
  else
  {
    Log::writeError("Punt");
    exit(1);
  }

  wendy::shutdown();
  exit(0);
}

