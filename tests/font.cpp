
#include <wendy/Wendy.h>

#include <cstdlib>

using namespace wendy;

namespace
{

const char* lines[] =
{
  "In A.D. 2101, war was beginning",
  "What happen?",
  "Somebody set up us the bomb.",
  "We get signal.",
  "What?",
  "Main screen turn on.",
  "It's you.",
  "How are you gentlemen?",
  "All your base are belong to us.",
  "You are on the way to destruction.",
  "What you say?",
  "You have no chance to survive make your time.",
  "Ha ha ha ....",
  NULL
};

class Demo : public Trackable
{
public:
  ~Demo(void);
  bool init(void);
  bool render(void);
private:
  ResourceIndex index;
  Ptr<GL::SharedProgramState> state;
  Ptr<render::GeometryPool> pool;
  Ref<render::Font> font;
};

Demo::~Demo(void)
{
  font = NULL;
  pool = NULL;

  GL::Context::destroySingleton();
}

bool Demo::init(void)
{
  if (!index.addSearchPath(Path("../media")))
    return false;

  if (!GL::Context::createSingleton(index))
  {
    logError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::getSingleton();
  context->setTitle("Font Test");
  context->setRefreshMode(GL::Context::MANUAL_REFRESH);

  state = new GL::SharedProgramState();
  context->setSharedProgramState(state);

  pool = new render::GeometryPool(*context);

  font = render::Font::read(*pool, Path("wendy/default.font"));
  if (!font)
  {
    logError("Failed to load font");
    return false;
  }

  return true;
}

bool Demo::render(void)
{
  GL::Context* context = GL::Context::getSingleton();
  context->clearColorBuffer();

  state->setOrthoProjectionMatrix(640.f, 480.f);

  const float em = font->getHeight();

  Vec2 pen(100.f, 400.f);

  for (size_t i = 0;  lines[i];  i++)
  {
    font->drawText(pen, ColorRGBA::WHITE, lines[i]);
    pen.y -= em * 1.5f;
  }

  return true;
}

} /*namespace*/

int main(int argc, char** argv)
{
  if (!wendy::initialize())
    std::exit(EXIT_FAILURE);

  Ptr<Demo> demo(new Demo());
  if (demo->init())
  {
    do
    {
      demo->render();
    }
    while (GL::Context::getSingleton()->update());
  }

  demo = NULL;

  wendy::shutdown();
  std::exit(EXIT_SUCCESS);
}

