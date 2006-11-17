///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
///////////////////////////////////////////////////////////////////////
#ifndef WENDY_GLLIGHT_H
#define WENDY_GLLIGHT_H
///////////////////////////////////////////////////////////////////////

#include <vector>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @brief Camera space light.
 *
 *  This works both with fixed pipeline and GLSL programs.
 */
class Light : public Managed<Light>
{
public:
  enum Type
  {
    DIRECTIONAL,
    POINT,
    SPOT,
  };
  enum ShaderType
  {
    VERTEX_SHADER,
    FRAGMENT_SHADER,
  };
  Light(const String& name = "");
  ~Light(void);
  bool operator < (const Light& other) const;
  bool isEnabled(void) const;
  void enable(void);
  void disable(void);
  bool isCastingShadows(void) const;
  void setShadowCasting(bool newState);
  Type getType(void) const;
  void setType(Type type);
  const ColorRGB& getAmbientIntensity(void) const;
  void setAmbientIntensity(const ColorRGB& newIntensity);
  const ColorRGB& getIntensity(void) const;
  void setIntensity(const ColorRGB& newIntensity);
  const Vector3& getPosition(void) const;
  void setPosition(const Vector3& newPosition);
  const Vector3& getDirection(void) const;
  void setDirection(const Vector3& newDirection);
  float getConstantAttenuation(void) const;
  void setConstantAttenuation(float newValue);
  float getLinearAttenuation(void) const;
  void setLinearAttenuation(float newValue);
  float getQuadraticAttenuation(void) const;
  void setQuadraticAttenuation(float newValue);
  float getCutoffAngle(void) const;
  void setCutoffAngle(float newAngle);
  void setDefaults(void);
  static void applyFixedState(void);
  static void applyShaderState(void);
  static void disableLights(void);
  static Shader* createShader(ShaderType type);
  static unsigned int getSlotCount(void);
private:
  Light(const Light& source);
  Light& operator = (const Light& source);
  static void onContextDestroy(void);
  char getTypeCharacter(void);
  typedef std::vector<Light*> LightList;
  typedef std::vector<ShaderRef> ShaderList;
  bool enabled;
  bool shadows;
  Type type;
  ColorRGB ambient;
  ColorRGB intensity;
  Vector3 position;
  Vector3 direction;
  float constant;
  float linear;
  float quadratic;
  float cutoff;
  static LightList current;
  static ShaderList shaders;
  static unsigned int slotCount;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLLIGHT_H*/
///////////////////////////////////////////////////////////////////////
