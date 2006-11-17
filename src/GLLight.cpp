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

#include <moira/Moira.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLShader.h>
#include <wendy/GLLight.h>

#include <algorithm>
#include <sstream>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

namespace
{

class LightComparator
{
public:
  inline bool operator () (const Light* x, const Light* y)
  {
    return *x < *y;
  }
};

}

///////////////////////////////////////////////////////////////////////

Light::Light(const String& name):
  Managed<Light>(name),
  enabled(false)
{
  static bool initialized = false;

  if (!initialized)
  {
    Context::getDestroySignal().connect(onContextDestroy);
    initialized = true;
  }

  setDefaults();
}

Light::~Light(void)
{
  disable();
}

bool Light::operator < (const Light& other) const
{
  return type < other.type;
}

bool Light::isEnabled(void) const
{
  return enabled;
}

void Light::enable(void)
{
  if (enabled)
    return;

  current.push_back(this);
  enabled = true;
}

void Light::disable(void)
{
  if (!enabled)
    return;

  LightList::iterator i = std::find(current.begin(), current.end(), this);
  if (i != current.end())
    current.erase(i);

  enabled = false;
}

bool Light::isCastingShadows(void) const
{
  return shadows;
}

void Light::setShadowCasting(bool newState)
{
  shadows = newState;
}

Light::Type Light::getType(void) const
{
  return type;
}

void Light::setType(Type type)
{
  type = type;
}

const ColorRGB& Light::getAmbientIntensity(void) const
{
  return ambient;
}

void Light::setAmbientIntensity(const ColorRGB& newIntensity)
{
  ambient = newIntensity;
}

const ColorRGB& Light::getIntensity(void) const
{
  return intensity;
}

void Light::setIntensity(const ColorRGB& newIntensity)
{
  intensity = newIntensity;
}

const Vector3& Light::getPosition(void) const
{
  return position;
}

void Light::setPosition(const Vector3& newPosition)
{
  position = newPosition;
}

const Vector3& Light::getDirection(void) const
{
  return direction;
}

void Light::setDirection(const Vector3& newDirection)
{
  direction = newDirection;
}

float Light::getConstantAttenuation(void) const
{
  return constant;
}

void Light::setConstantAttenuation(float newValue)
{
  constant = newValue;
}

float Light::getLinearAttenuation(void) const
{
  return linear;
}

void Light::setLinearAttenuation(float newValue)
{
  linear = newValue;
}

float Light::getQuadraticAttenuation(void) const
{
  return quadratic;
}

void Light::setQuadraticAttenuation(float newValue)
{
  quadratic = newValue;
}

float Light::getCutoffAngle(void) const
{
  return cutoff;
}

void Light::setCutoffAngle(float newAngle)
{
  newAngle = std::max(newAngle, 0.f);
  if (newAngle > M_PI / 2.f)
    newAngle = M_PI;

  cutoff = newAngle;
}

void Light::setDefaults(void)
{
  shadows = false;
  type = DIRECTIONAL;
  ambient.set(0.f, 0.f, 0.f);
  intensity.set(1.f, 1.f, 1.f);
  position.set(0.f, 0.f, 0.f);
  direction.set(0.f, 0.f, 1.f);
  constant = 1.f;
  linear = 0.f;
  quadratic = 0.f;
  cutoff = M_PI;
}

void Light::applyFixedState(void)
{
  if (current.size() > getSlotCount())
    Log::writeWarning("Current OpenGL context has too few light slots to apply all currently enabled lights");

  // NOTE: It's up to the renderer to decide which lights to enable.  If it
  //       hasn't done its job then we don't care here.

  const unsigned int count = std::min((unsigned int) current.size(), getSlotCount());

  for (unsigned int i = 0;  i < count;  i++)
  {
    const Light& light = *current[i];

    // NOTE: Since these values will most often only be written once per frame,
    //       (unless we're doing stencil shadowing, and then most of the time
    //       will be spent redrawing anyway), and since at least some of them
    //       will likely change fairly often, I'm not taking the time to add
    //       state caching for them right now.

    glEnable(GL_LIGHT0 + i);

    ColorRGBA intensityGL;

    intensityGL.set(light.ambient, 1.f);
    glLightfv(GL_LIGHT0 + i, GL_AMBIENT, intensityGL);

    intensityGL.set(light.intensity, 1.f);
    glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, intensityGL);
    glLightfv(GL_LIGHT0 + i, GL_SPECULAR, intensityGL);

    Vector4 positionGL;

    if (light.type == DIRECTIONAL)
      positionGL.set(light.direction, 0.f);
    else
      positionGL.set(light.position, 1.f);

    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glLightfv(GL_LIGHT0 + i, GL_POSITION, positionGL);
    glPopMatrix();
    glPopAttrib();

    if (light.type != DIRECTIONAL)
    {
      glLightf(GL_LIGHT0 + i, GL_CONSTANT_ATTENUATION, light.constant);
      glLightf(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, light.linear);
      glLightf(GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, light.quadratic);

      if (light.type == SPOT)
      {
	const float degrees = light.cutoff * 180.f / M_PI;
	glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, degrees);

	glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, light.direction);
      }
    }

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
      Log::writeError("Error when applying light %s: %s",
                      light.getName().c_str(),
		      gluErrorString(error));
  }

  // Disable any unused light slots

  for (unsigned int i = count;  i < slotCount;  i++)
    glDisable(GL_LIGHT0 + i);
}

void Light::applyShaderState(void)
{
  for (unsigned int i = 0;  i < current.size();  i++)
  {
    // TODO: Write uniforms.
  }
}

Shader* Light::createShader(ShaderType type)
{
  std::sort(current.begin(), current.end(), LightComparator());

  String shaderName = "wendyLightShader:";

  for (unsigned int i = 0;  i < current.size();  i++)
    shaderName.append(1, current[i]->getTypeCharacter());

  switch (type)
  {
    case VERTEX_SHADER:
    {
      if (VertexShader* shader = VertexShader::findInstance(shaderName))
	return shader;

      // TODO: Create shader.

      break;
    }

    case FRAGMENT_SHADER:
    {
      if (FragmentShader* shader = FragmentShader::findInstance(shaderName))
	return shader;

      // TODO: Create shader.

      break;
    }
  }

  return NULL;
}

unsigned int Light::getSlotCount(void)
{
  if (!Context::get())
  {
    Log::writeError("Cannot query light slot before OpenGL context creation");
    return 0;
  }

  if (!slotCount)
    glGetIntegerv(GL_MAX_LIGHTS, (GLint*) &slotCount);

  return slotCount;
}

Light::Light(const Light& source):
  Managed<Light>(source)
{
  // NOTE: Not implemented.
}

Light& Light::operator = (const Light& source)
{
  // NOTE: Not implemented.

  return *this;
}

void Light::onContextDestroy(void)
{
  shaders.clear();

  slotCount = 0;
}

char Light::getTypeCharacter(void)
{
  switch (type)
  {
    case DIRECTIONAL:
      return 'D';
    case POINT:
      return 'P';
    case SPOT:
      return 'S';
    default:
      throw Exception("Invalid light type");
  }
}

Light::LightList Light::current;
Light::ShaderList Light::shaders;

unsigned int Light::slotCount = 0;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
