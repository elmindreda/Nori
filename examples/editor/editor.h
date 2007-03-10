
class CubeEffect : public demo::Effect
{
public:
  CubeEffect(demo::EffectType& type, const String& name = "");
  bool init(void);
protected:
  void render(void) const;
  void update(Time deltaTime);
  render::Scene scene;
  render::Camera camera;
  render::CameraNode* cameraNode;
  render::MeshNode* meshNode;
  Ref<render::Mesh> cube;
};


