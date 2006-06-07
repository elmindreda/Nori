///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2005 Camilla Berglund <elmindreda@home.se>
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

#include <moira/Config.h>
#include <moira/Portability.h>
#include <moira/Core.h>
#include <moira/Log.h>
#include <moira/Point.h>
#include <moira/Vector.h>
#include <moira/Color.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLLight.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

Light::~Light(void)
{
  setEnabled(false);
}

GLenum Light::getGLID(void) const
{
  if (data.enabled)
    return GL_LIGHT0 + index;

  return 0;
}

bool Light::isEnabled(void) const
{
  return data.enabled;
}

bool Light::setEnabled(bool enabled)
{
  if (enabled != data.enabled)
  {
    if (!enabled)
    {
      glDisable(getGLID());
      caches[index].enabled = data.enabled = false;
      return true;
    }

    if (!caches[index].enabled)
      caches[index].enabled = data.enabled = true;
    else
    {
      for (unsigned int i = 0;  i < caches.size();  i++)
      {
        if (!caches[i].enabled)
        {
          index = i;
          caches[index].enabled = data.enabled = true;
          break;
        }
      }
    }

    if (!data.enabled)
      return false;

    glEnable(getGLID());
    apply();
  }

  return true;
}

Light::Type Light::getType(void) const
{
  return data.type;
}

void Light::setType(Type type)
{
  data.type = type;
  data.dirty = true;

  if (data.enabled)
    apply();
}

const ColorRGB& Light::getAmbientIntensity(void) const
{
  return data.ambient;
}

void Light::setAmbientIntensity(const ColorRGB& color)
{
  data.ambient = color;
  data.dirty = true;

  if (data.enabled)
    apply();
}

const ColorRGB& Light::getDiffuseIntensity(void) const
{
  return data.diffuse;
}

void Light::setDiffuseIntensity(const ColorRGB& color)
{
  data.diffuse = color;
  data.dirty = true;

  if (data.enabled)
    apply();
}

const ColorRGB& Light::getSpecularIntensity(void) const
{
  return data.specular;
}

void Light::setSpecularIntensity(const ColorRGB& color)
{
  data.specular = color;
  data.dirty = true;

  if (data.enabled)
    apply();
}

const Vector3& Light::getPosition(void) const
{
  return data.position;
}

void Light::setPosition(const Vector3& position)
{
  data.position = position;
  data.dirty = true;

  if (data.enabled)
    apply();
}

const Vector3& Light::getDirection(void) const
{
  return data.direction;
}

void Light::setDirection(const Vector3& direction)
{
  data.direction = direction;
  data.dirty = true;

  if (data.enabled)
    apply();
}

void Light::setDefaults(void)
{
  data.setDefaults();
}

Light* Light::createInstance(const std::string& name)
{
  Ptr<Light> light = new Light(name);
  if (!light->init())
    return NULL;

  return light.detachObject();
}

void Light::invalidateCache(void)
{
  for (CacheList::iterator i = caches.begin();  i != caches.end();  i++)
    (*i).dirty = true;
}

Light::Light(const std::string& name):
  Managed<Light>(name),
  index(0)
{
}

bool Light::init(void)
{
  if (!Context::get())
  {
    Log::writeError("Cannot create light without OpenGL context");
    return false;
  }

  if (!caches.size())
  {
    unsigned int lightCount;
    glGetIntegerv(GL_MAX_LIGHTS, (GLint*) &lightCount);

    caches.resize(lightCount);
  }

  return true;
}

void Light::apply(void)
{
  Data& cache = caches[index];

  if (cache.dirty || cache.ambient != data.ambient)
  {
    ColorRGBA color(data.ambient, 1.f);
    glLightfv(getGLID(), GL_AMBIENT, color);
    cache.ambient = data.ambient;
  }

  if (cache.dirty || cache.diffuse != data.diffuse)
  {
    ColorRGBA color(data.diffuse, 1.f);
    glLightfv(getGLID(), GL_DIFFUSE, color);
    cache.diffuse = data.diffuse;
  }

  if (cache.dirty || cache.specular != data.specular)
  {
    ColorRGBA color(data.specular, 1.f);
    glLightfv(getGLID(), GL_SPECULAR, color);
    cache.specular = data.specular;
  }

  Vector4 dataPosition = makePosition(data);
  Vector4 cachePosition = makePosition(cache);

  if (cache.dirty || cachePosition != dataPosition)
  {
    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glLightfv(getGLID(), GL_POSITION, dataPosition);
    glPopMatrix();
    glPopAttrib();

    cache.position = data.position;
    cache.direction = data.direction;
    cache.type = data.type;
  }

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    Log::writeWarning("Error when applying light: %s", gluErrorString(error));
  
  data.dirty = cache.dirty = false;
}

Vector4 Light::makePosition(const Data& data)
{
  if (data.type == DIRECTIONAL)
    return Vector4(data.direction.x, data.direction.y, data.direction.z, 0.f);
  else
    return Vector4(data.position.x, data.position.y, data.position.z, 1.f);
}

Light::CacheList Light::caches;

///////////////////////////////////////////////////////////////////////

Light::Data::Data(void)
{
  setDefaults();
}

void Light::Data::setDefaults(void)
{
  dirty = true;
  type = POSITIONAL;
  enabled = false;
  ambient.set(0.f, 0.f, 0.f);
  diffuse.set(1.f, 1.f, 1.f);
  specular.set(1.f, 1.f, 1.f);
  position.set(0.f, 0.f, 0.f);
  direction.set(0.f, 0.f, 1.f);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
