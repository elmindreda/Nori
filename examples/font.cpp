
#include <wendy/Wendy.h>

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
  Ptr<render::Font> font;
};

Demo::~Demo(void)
{
  font = NULL;

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

  if (!GL::Context::create(GL::ContextMode()))
  {
    Log::writeError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::get();
  context->setTitle("Fonts");

  if (!render::GeometryPool::create(*GL::Context::get()))
    return false;

  font = render::Font::readInstance("default");
  if (!font)
  {
    Log::writeError("Failed to load font");
    return false;
  }

  return true;
}

bool Demo::render(void)
{
  GL::Context* context = GL::Context::get();
  context->clearColorBuffer();
  context->setProjectionMatrix2D(640.f, 480.f);

  const float em = font->getHeight();

  Vec2 pen(100.f, 400.f);

  for (size_t i = 0;  lines[i];  i++)
  {
    font->drawText(pen, ColorRGBA::WHITE, lines[i]);
    pen.y -= em * 1.5;
  }

  return true;
}

}

int main(int argc, char** argv)
{
  if (!wendy::initialize())
    exit(1);

  Ptr<Demo> demo(new Demo());
  if (demo->init())
  {
    do
    {
      demo->render();
    }
    while (GL::Context::get()->update());
  }

  demo = NULL;

  wendy::shutdown();
  exit(0);
}

