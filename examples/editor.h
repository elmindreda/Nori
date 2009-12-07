
class CubeEffect : public demo::Effect
{
public:
  CubeEffect(demo::EffectType& type, const String& name = "");
  bool init(void);
protected:
  void render(void) const;
  void update(Time deltaTime);
  scene::Graph graph;
  render::Camera camera;
  scene::CameraNode* cameraNode;
  scene::MeshNode* meshNode;
};


