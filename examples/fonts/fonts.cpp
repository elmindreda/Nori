
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

namespace
{

class Demo : public Trackable
{
public:
  ~Demo(void);
  bool init(void);
private:
  bool render(void);
  Ptr<render::Font> font;
};

Demo::~Demo(void)
{
  font = NULL;

  GL::Renderer::destroy();
  GL::Context::destroy();
}

bool Demo::init(void)
{
  Image::addSearchPath(Path("../media"));
  Font::addSearchPath(Path("../media"));
  GL::VertexProgram::addSearchPath(Path("../media"));
  GL::FragmentProgram::addSearchPath(Path("../media"));
  GL::Program::addSearchPath(Path("../media"));

  GL::ContextMode mode;
  mode.set(640, 480, 32, 0, 0, 0, GL::ContextMode::WINDOWED);

  if (!GL::Context::create(mode))
  {
    Log::writeError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::get();
  context->setTitle("Fonts");
  context->getRenderSignal().connect(*this, &Demo::render);

  if (!GL::Renderer::create(*GL::Context::get()))
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
  GL::ScreenCanvas canvas;
  canvas.begin();
  canvas.clearColorBuffer();

  GL::Renderer::get()->begin2D(640.f, 480.f);

  String text = "In A.D. 2101, war was beginning\n"
		"What happen?\n"
		"Somebody set up us the bomb.\n"
		"We get signal.\n"
		"What?\n"
		"Main screen turn on.\n"
		"It's you.\n"
		"How are you gentlemen?\n"
                "All your base are belong to us.\n"
                "You are on the way to destruction.\n"
		"What you say?\n"
                "You have no chance to survive make your time.\n"
		"Ha ha ha ....";

  Vector2 pen(100.f, 400.f);

  font->setPenPosition(pen);
  font->setColor(ColorRGBA::WHITE);
  font->drawText(text);

  GL::Renderer::get()->end();

  canvas.end();

  return true;
}

}

int main(int argc, char** argv)
{
  if (!wendy::initialize())
    exit(1);

  Ptr<Demo> demo = new Demo();
  if (demo->init())
  {
    while (GL::Context::get()->update())
      ;
  }

  demo = NULL;

  wendy::shutdown();
  exit(0);
}

