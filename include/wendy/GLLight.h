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
#ifndef WEGLLIGHT_H
#define WEGLLIGHT_H
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

class Light : public Managed<Light>
{
public:
  enum Type { DIRECTIONAL, POSITIONAL };
  ~Light(void);
  GLenum getGLID(void) const;
  bool isEnabled(void) const;
  bool setEnabled(bool enabled);
  Type getType(void) const;
  void setType(Type type);
  const ColorRGB& getAmbientIntensity(void) const;
  void setAmbientIntensity(const ColorRGB& color);
  const ColorRGB& getDiffuseIntensity(void) const;
  void setDiffuseIntensity(const ColorRGB& color);
  const ColorRGB& getSpecularIntensity(void) const;
  void setSpecularIntensity(const ColorRGB& color);
  const Vector3& getPosition(void) const;
  void setPosition(const Vector3& position);
  const Vector3& getDirection(void) const;
  void setDirection(const Vector3& direction);
  void setDefaults(void);
  static Light* createInstance(const std::string& name);
  static void invalidateCache(void);
private:
  class Data
  {
  public:
    Data(void);
    void setDefaults(void);
    bool enabled;
    bool dirty;
    Type type;
    ColorRGB ambient;
    ColorRGB diffuse;
    ColorRGB specular;
    Vector3 position;
    Vector3 direction;
  };
  Light(const std::string& name);
  bool init(void);
  void apply(void);
  Vector4 makePosition(const Data& data);
  typedef std::vector<Data> CacheList;
  GLenum index;
  Data data;
  static CacheList caches;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WEGLLIGHT_H*/
///////////////////////////////////////////////////////////////////////
