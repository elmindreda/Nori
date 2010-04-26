
class Demo : public Trackable
{
public:
  ~Demo(void);
  bool init(void);
private:
  Ptr<UI::Renderer> renderer;
};

class CubeEffect : public demo::Effect
{
public:
  CubeEffect(demo::EffectType& type, const String& name = "");
  bool init(void);
protected:
  void render(void) const;
  void update(Time deltaTime);
  Ptr<UI::Renderer> renderer;
  scene::Graph graph;
  Ref<render::Camera> camera;
  scene::CameraNode* cameraNode;
  scene::MeshNode* meshNode;
};

