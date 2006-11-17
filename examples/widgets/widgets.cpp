
#include <wendy/Wendy.h>

using namespace moira;
using namespace wendy;

class Widgets : public Trackable
{
public:
  ~Widgets(void);
  bool init(void);
  void render(void);
private:
  void onButtonPush(UI::Button& button);
  void onValueChange(UI::Slider& slider, float newValue);
  Ptr<UI::Window> window;
  UI::Label* label;
};

Widgets::~Widgets(void)
{
  UI::Renderer::destroy();
  GL::Renderer::destroy();
}

bool Widgets::init(void)
{
  if (!GL::Renderer::create())
    return false;

  if (!UI::Renderer::create())
    return false;

  window = new UI::Window("Stairs");
  window->setArea(Rectangle(10, 10, 500, 500));

  UI::Button* button;

  button = new UI::Button("Push");
  button->setPosition(Vector2(10, 10));
  button->getPushedSignal().connect(*this, &Widgets::onButtonPush);
  window->addChild(*button);

  button = new UI::Button("Shove");
  button->setPosition(Vector2(120, 10));
  button->getPushedSignal().connect(*this, &Widgets::onButtonPush);
  window->addChild(*button);

  button = new UI::Button();
  button->setPosition(Vector2(220, 10));
  button->getPushedSignal().connect(*this, &Widgets::onButtonPush);
  window->addChild(*button);

  label = new UI::Label("Amount of grandpa");
  label->setArea(Rectangle(10, 350, 400, 20));
  window->addChild(*label);

  UI::Entry* entry = new UI::Entry("Do you have stairs in your house?");
  entry->setPosition(Vector2(10, 300));
  window->addChild(*entry);

  UI::Slider* slider = new UI::Slider();
  slider->setArea(Rectangle(10, 400, 400, 20));
  slider->setOrientation(UI::HORIZONTAL);
  slider->getChangeValueSignal().connect(*this, &Widgets::onValueChange);
  window->addChild(*slider);

  UI::List* list = new UI::List();
  list->setArea(Rectangle(400, 100, 150, 200));
  list->insertItem(new UI::Item("Bread"), 0);
  list->insertItem(new UI::Item("Terrible secret"), 0);
  list->insertItem(new UI::Item("Space"), 0);
  list->insertItem(new UI::Item("Blind people"), 0);
  window->addChild(*list);

  window->activate();

  return true;
}

void Widgets::render(void)
{
  GL::ScreenCanvas canvas;
  canvas.begin();
  canvas.clearColorBuffer(ColorRGBA::WHITE);

  UI::Widget::renderRoots();

  canvas.end();
}

void Widgets::onButtonPush(UI::Button& button)
{
  label->setText(button.getText());
}

void Widgets::onValueChange(UI::Slider& slider, float newValue)
{
  Variant value;
  value.setFloatValue(newValue);
  label->setText("Amount of grandpa: " + value.asString());
}

int main()
{
  GL::ContextMode mode;
  mode.set(640, 480, 32, 0, 0, GL::ContextMode::WINDOWED);

  if (!wendy::initialize())
    return 1;

  if (GL::Context::create(mode))
  {
    GL::Context::get()->setTitle("Widgets");

    Ptr<Widgets> widgets = new Widgets();
    if (widgets->init())
    { 
      while (GL::Context::get()->update())
	widgets->render();
    }

    widgets = NULL;

    GL::Context::destroy();
  }

  wendy::shutdown();
  return 0;
}

