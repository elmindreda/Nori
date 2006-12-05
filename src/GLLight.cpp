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

Mapper<String, Light::Type> lightNameMap;

}

///////////////////////////////////////////////////////////////////////

Light::Light(const String& name):
  Managed<Light>(name)
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
}

bool Light::operator < (const Light& other) const
{
  return type < other.type;
}

bool Light::isBounded(void) const
{
  if (type == DIRECTIONAL)
    return false;

  return radius < std::numeric_limits<float>::infinity();
}

Light::Type Light::getType(void) const
{
  return type;
}

void Light::setType(Type newType)
{
  type = newType;
}

const ColorRGB& Light::getAmbience(void) const
{
  return ambience;
}

void Light::setAmbience(const ColorRGB& newAmbience)
{
  ambience = newAmbience;
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

float Light::getRadius(void) const
{
  return radius;
}

void Light::setRadius(float newRadius)
{
  radius = newRadius;
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

const Sphere& Light::getBounds(void) const
{
  const float infinity = std::numeric_limits<float>::infinity();

  if (type == DIRECTIONAL || radius == infinity)
    bounds.set(Vector3(0.f, 0.f, 0.f), infinity);
  else
    bounds.set(position, radius);

  return bounds;
}

char Light::getTypeCharacter(void) const
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

void Light::setDefaults(void)
{
  type = DIRECTIONAL;
  ambience.set(0.f, 0.f, 0.f);
  intensity.set(1.f, 1.f, 1.f);
  position.set(0.f, 0.f, 0.f);
  direction.set(0.f, 0.f, 1.f);
  radius = std::numeric_limits<float>::infinity();
  cutoff = M_PI;
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
  slotCount = 0;
}

unsigned int Light::slotCount = 0;

///////////////////////////////////////////////////////////////////////

LightState::LightState(void)
{
  static bool initialized = false;

  if (!initialized)
  {
    if (Context::get())
    {
      Context::getDestroySignal().connect(onContextDestroy);
      initialized = true;
    }
  }
}

void LightState::apply(void) const
{
  // NOTE: It's up to the renderer to decide which lights to enable.  If it
  //       hasn't done its job then we don't care here, except to yell.

  const unsigned int count = std::min((unsigned int) lights.size(), Light::getSlotCount());

  if (count < lights.size())
    Log::writeWarning("Current OpenGL context has too few light slots to apply all currently enabled lights");

  for (unsigned int i = 0;  i < count;  i++)
  {
    Light& light = *lights[i];

    // NOTE: Since these values will most often only be written once per frame,
    //       (unless we're doing stencil shadowing, and then most of the time
    //       will be spent redrawing anyway), and since at least some of them
    //       will likely change fairly often, I'm not taking the time to add
    //       state caching for them right now.

    glEnable(GL_LIGHT0 + i);

    ColorRGBA intensityGL;

    intensityGL.set(light.getAmbience(), 1.f);
    glLightfv(GL_LIGHT0 + i, GL_AMBIENT, intensityGL);

    intensityGL.set(light.getIntensity(), 1.f);
    glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, intensityGL);
    glLightfv(GL_LIGHT0 + i, GL_SPECULAR, intensityGL);

    Vector4 positionGL;

    if (light.getType() == Light::DIRECTIONAL)
      positionGL.set(light.getDirection(), 0.f);
    else
      positionGL.set(light.getPosition(), 1.f);

    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glLightfv(GL_LIGHT0 + i, GL_POSITION, positionGL);
    glPopMatrix();
    glPopAttrib();

    if (light.getType() != Light::DIRECTIONAL)
    {
      glLightf(GL_LIGHT0 + i, GL_CONSTANT_ATTENUATION, 1.f);
      glLightf(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, 0.f);

      if (light.isBounded())
      {
	const float epsilon = 0.01f;
	const float radius = light.getRadius();
	const float quadratic = 1.f / (radius * radius * epsilon);

	glLightf(GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, quadratic);
      }
      else
	glLightf(GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, 0.f);

      if (light.getType() == Light::SPOT)
      {
	const float degrees = light.getCutoffAngle() * 180.f / M_PI;
	glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, degrees);

	glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, light.getDirection());
      }
    }

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
      Log::writeError("Error when applying light %s: %s",
                      light.getName().c_str(),
		      gluErrorString(error));
  }

  // Disable any unused light slots
  for (unsigned int i = count;  i < Light::getSlotCount();  i++)
    glDisable(GL_LIGHT0 + i);

  current.lights = lights;
}

void LightState::attachLight(Light& light)
{
  List::const_iterator i = std::find(lights.begin(), lights.end(), &light);
  if (i != lights.end())
    return;

  lights.push_back(&light);
  std::sort(lights.begin(), lights.end(), LightComparator());
}

void LightState::detachLight(Light& light)
{
  List::iterator i = std::find(lights.begin(), lights.end(), &light);
  if (i != lights.end())
    lights.erase(i);
}

void LightState::detachLights(void)
{
  lights.clear();
}

unsigned int LightState::getLightCount(void) const
{
  return lights.size();
}

Light& LightState::getLight(unsigned int index) const
{
  return *lights[index];
}

void LightState::getPermutationName(String& name) const
{
  name.clear();
  name.reserve(lights.size());

  for (unsigned int i = 0;  i < lights.size();  i++)
    name.append(1, lights[i]->getTypeCharacter());
}

const String& LightState::getPermutationText(void) const
{
  String name;
  getPermutationName(name);

  String& source = permutations[name];
  if (!source.empty())
    return source;

  generatePermutation(source);
  return source;
}

const LightState& LightState::getCurrent(void)
{
  return current;
}

void LightState::generatePermutation(String& text) const
{
  if (lightNameMap.isEmpty())
  {
    lightNameMap[Light::DIRECTIONAL] = "directional";
    lightNameMap[Light::POINT] = "point";
    lightNameMap[Light::SPOT] = "spot";
  }

  std::stringstream source;

  // Generate ambient(P)

  source << "vec3 ambient(in vec3 P)\n{\n";

  if (lights.empty())
    source << "  return vec3(0.0);\n}\n\n";
  else
  {
    source << "  vec3 Cl = vec3(0.0), L;\n";

    for (unsigned int i = 0;  i < lights.size();  i++)
    {
      const Light& light = *lights[i];

      source << "  // Slot " << i << " is a " << lightNameMap[light.getType()] << " light\n";

      if (light.isBounded())
      {
	source << "  L = gl_LightSource[" << i << "].position.xyz - P;\n";
	source << "  Cl += gl_LightSource[" << i << "].ambient.rgb / "
		  "(dot(L, L) * gl_LightSource[" << i << "].quadraticAttenuation);\n";
      }
      else
	source << "  Cl += gl_LightSource[" << i << "].ambient.rgb;\n";
    }

    source << "  return Cl;\n}\n\n";
  }

  // Generate diffuse(P, N)

  source << "vec3 diffuse(in vec3 P, in vec3 N)\n{\n";

  if (lights.empty())
    source << "  return vec3(0.0);\n}\n\n";
  else
  {
    source << "  vec3 Cl = vec3(0.0), L, NdotL;\n";

    for (unsigned int i = 0;  i < lights.size();  i++)
    {
      const Light& light = *lights[i];

      source << "  // Slot " << i << " is a " << lightNameMap[light.getType()] << " light\n";

      switch (light.getType())
      {
	case Light::DIRECTIONAL:
	{
	  source << "  Cl += gl_LightSource[" << i << "].diffuse.rgb * "
		    "max(dot(gl_LightSource[" << i << "].position.xyz, N), 0.0);\n";
	  break;
	}

	case Light::POINT:
	{
	  source << "  L = gl_LightSource[" << i << "].position.xyz - P;\n";

	  if (light.isBounded())
	    source << "  Cl += gl_LightSource[" << i << "].diffuse.rgb * "
		      "max(dot(normalize(L), N), 0.0) / "
		      "(dot(L, L) * gl_LightSource[" << i << "].quadraticAttenuation);\n";
	  else
	    source << "  Cl += gl_LightSource[" << i << "].diffuse.rgb * "
		      "max(dot(normalize(L), N), 0.0);\n";
	  break;
	}

	case Light::SPOT:
	{
	  source << "  L = gl_LightSource[" << i << "].position.xyz - P;\n";
	  source << "  NdotL = max(dot(normalize(L), N), 0.0);\n";
	  source << "  if (NdotL > 0.0)\n  {\n";
	  source << "    float e = dot(gl_LightSource[" << i << "].spotDirection, normalize(-L));\n";
	  source << "    if (e > gl_LightSource[" << i << "].spotCosCutoff)\n    {\n";
	  // TODO: The code code.
	  source << "    }\n  }\n";
	  break;
	}
      }
    }

    source << "  return Cl;\n}\n\n";
  }

  // Generate specular(P, N, s)

  source << "vec3 specular(in vec3 P, in vec3 N, float s)\n{\n";

  if (lights.empty())
    source << "  return vec3(0.0);\n}\n\n";
  else
  {
    source << "  vec3 Cl = vec3(0.0), L, R, NdotL;\n";

    for (unsigned int i = 0;  i < lights.size();  i++)
    {
      const Light& light = *lights[i];

      source << "  // Slot " << i << " is a " << lightNameMap[light.getType()] << " light\n";

      switch (light.getType())
      {
	case Light::DIRECTIONAL:
	{
	  source << "  R = reflect(-normalize(gl_LightSource[" << i << "].position.xyz), N);\n";
	  source << "  Cl += gl_LightSource[" << i << "].specular.rgb * "
		    "pow(max(dot(-normalize(P), R), 0.0), s);\n";
	  break;
	}

	case Light::POINT:
	{
	  source << "  L = P - gl_LightSource[" << i << "].position.xyz;\n";
	  source << "  R = reflect(normalize(L), N);\n";

	  if (light.isBounded())
	    source << "  Cl += gl_LightSource[" << i << "].specular.rgb * "
		      "pow(max(dot(-normalize(P), R), 0.0), s) / "
		      "(dot(L, L) * gl_LightSource[" << i << "].quadraticAttenuation);\n";
	  else
	    source << "  Cl += gl_LightSource[" << i << "].specular.rgb * "
		      "pow(max(dot(-normalize(P), R), 0.0), s);\n";
	  break;
	}

	case Light::SPOT:
	{
	  source << "  L = gl_LightSource[" << i << "].position.xyz - P;\n";
	  source << "  NdotL = max(dot(normalize(L), N), 0.0);\n";
	  source << "  if (NdotL > 0.0)\n  {\n";
	  // TODO: The code code.
	  source << "  }\n";
	  break;
	}
      }
    }

    source << "  return Cl;\n}\n\n";
  }

  Log::writeInformation("%s", source.str().c_str());

  text = source.str();
}

void LightState::onContextDestroy(void)
{
  current.detachLights();
}

LightState LightState::current;

LightState::PermutationMap LightState::permutations;

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
