
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

#include "editor.h"

namespace
{

bool setup(void)
{
  if (!GL::Renderer::create())
    return false;

  if (!UI::Renderer::create())
    return false;

  new demo::EffectTemplate<CubeEffect>("Shader cube");

  if (!demo::Editor::create())
    return false;

  return true;
}

}

CubeEffect::CubeEffect(demo::EffectType& type, const String& name):
  demo::Effect(type, name)
{
}

bool CubeEffect::init(void)
{
  cube = render::Mesh::readInstance("cube");
  if (!cube)
    return false;

  light = new GL::Light();
  light->setType(GL::Light::DIRECTIONAL);
  light->setIntensity(ColorRGB::WHITE);

  camera.setFOV(60.f);

  cameraNode = new render::CameraNode();
  cameraNode->setCameraName(camera.getName());
  cameraNode->getLocalTransform().position.z = cube->getBounds().radius * 1.5f;
  scene.addNode(*cameraNode);

  meshNode = new render::MeshNode();
  meshNode->setMesh(cube);
  scene.addNode(*meshNode);

  render::LightNode* lightNode = new render::LightNode();
  lightNode->setLight(light); scene.addNode(*lightNode);

  return true;
}

void CubeEffect::render(void) const
{
  render::Queue queue(camera);
  scene.enqueue(queue);
  queue.render();

  demo::Effect::render();
}

void CubeEffect::update(Time deltaTime)
{
  meshNode->getLocalTransform().rotation.setAxisRotation(Vector3(0.f, 1.f, 0.f), getTimeElapsed());

  scene.setTimeElapsed(getTimeElapsed());
}

int main(int argc, char** argv)
{
  if (!wendy::initialize())
    exit(1);

  GL::ContextMode mode(640, 480, 32, 32, 0, true);
  if (GL::Context::create(mode))
  {
    if (setup())
      demo::Editor::get()->run();

    demo::Editor::destroy();
    UI::Renderer::destroy();
    GL::Renderer::destroy();
    GL::Context::destroy();
  }
  else
    Log::writeError("Punt");

  wendy::shutdown();
  exit(0);
}

