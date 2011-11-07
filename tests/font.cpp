
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

class Test : public Trackable
{
public:
  ~Test();
  bool init();
  void run();
private:
  ResourceIndex index;
  Ref<render::SharedProgramState> state;
  Ptr<render::GeometryPool> pool;
  Ref<render::Font> font;
};

Test::~Test()
{
  font = NULL;
  pool = NULL;

  GL::Context::destroySingleton();
}

bool Test::init()
{
  if (!index.addSearchPathAlt(Path("media"), Path("../media")))
    return false;

  if (!GL::Context::createSingleton(index, GL::WindowConfig("Text Rendering Test")))
  {
    logError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::getSingleton();
  context->setRefreshMode(GL::Context::MANUAL_REFRESH);

  state = new render::SharedProgramState();
  state->reserveSupported(*context);
  context->setCurrentSharedProgramState(state);

  pool = new render::GeometryPool(*context, 2048);

  font = render::Font::read(*pool, Path("wendy/UIDefault.font"));
  if (!font)
  {
    logError("Failed to load font");
    return false;
  }

  return true;
}

void Test::run()
{
  GL::Context* context = GL::Context::getSingleton();

  do
  {
    context->clearColorBuffer();

    state->setOrthoProjectionMatrix(640.f, 480.f);

    const float em = font->getHeight();

    vec2 pen(100.f, 400.f);

    for (size_t i = 0;  lines[i];  i++)
    {
      font->drawText(pen, vec4(1.f), lines[i]);
      pen.y -= em * 1.5f;
    }
  }
  while (context->update());
}

} /*namespace*/

int main()
{
  Ptr<Test> test(new Test());
  if (!test->init())
  {
    logError("Failed to initialize test");
    std::exit(EXIT_FAILURE);
  }

  test->run();
  test = NULL;

  std::exit(EXIT_SUCCESS);
}

