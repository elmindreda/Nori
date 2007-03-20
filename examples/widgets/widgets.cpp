
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
  void onButtonPushed(UI::Button& button);
  void onValueChanged(UI::Slider& slider);
  void onItemSelected(UI::Popup& menu, unsigned int index);
  Ptr<UI::Window> window;
  UI::Label* label;
};

Widgets::~Widgets(void)
{
  window = NULL;

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
  button->getPushedSignal().connect(*this, &Widgets::onButtonPushed);
  window->addChild(*button);

  button = new UI::Button("Shove");
  button->setPosition(Vector2(120, 10));
  button->getPushedSignal().connect(*this, &Widgets::onButtonPushed);
  window->addChild(*button);

  UI::Popup* popup = new UI::Popup();
  popup->setPosition(Vector2(220, 10));
  popup->getItemSelectedSignal().connect(*this, &Widgets::onItemSelected);
  popup->addItem(*new UI::Item("Space"));
  popup->addItem(*new UI::Item("Terrible secret"));
  popup->addItem(*new UI::Item("Stairs"));
  popup->addItem(*new UI::Item("PAK CHOOIE UNF"));
  window->addChild(*popup);

  label = new UI::Label("Amount of grandpa");
  label->setArea(Rectangle(10, 350, 400, 20));
  window->addChild(*label);

  UI::Entry* entry = new UI::Entry("Do you have stairs in your house?");
  entry->setArea(Rectangle(10, 300, 400, entry->getArea().size.y));
  window->addChild(*entry);

  UI::Slider* slider = new UI::Slider();
  slider->setArea(Rectangle(10, 400, 400, 20));
  slider->setValueRange(0.f, 100.f);
  slider->getValueChangedSignal().connect(*this, &Widgets::onValueChanged);
  window->addChild(*slider);

  UI::Scroller* scroller = new UI::Scroller();
  scroller->setArea(Rectangle(10, 430, 400, 20));
  scroller->setValueRange(0.f, 100.f);
  scroller->setPercentage(0.25f);
  window->addChild(*scroller);

  const char* names[] = { "Space", "Terrible secret", "Stairs", "House" };

  UI::List* list = new UI::List();
  list->setArea(Rectangle(300, 80, 150, 200));
  for (unsigned int i = 0;  i < 100;  i++)
    list->insertItem(new UI::Item(names[rand()&3]), 0);
  window->addChild(*list);

  window->activate();

  return true;
}

void Widgets::render(void)
{
  GL::ScreenCanvas canvas;
  canvas.begin();
  canvas.clearColorBuffer(ColorRGBA::WHITE);

  UI::Widget::drawRoots();

  canvas.end();
}

void Widgets::onButtonPushed(UI::Button& button)
{
  label->setText("Button: " + button.getText());
}

void Widgets::onValueChanged(UI::Slider& slider)
{
  Variant value;
  value.setFloatValue(slider.getValue());
  label->setText("Amount of grandpa: " + value.asString());
}

void Widgets::onItemSelected(UI::Popup& popup, unsigned int index)
{
  UI::Item* item = popup.getItem(index);
  label->setText("Menu: " + item->getValue());
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

