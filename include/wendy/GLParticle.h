///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2006 Camilla Berglund <elmindreda@home.se>
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
#ifndef WENDY_PARTICLE_H
#define WENDY_PARTICLE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class Particle
{
public:
  Vector3 position;
  Vector3 velocity;
  Vector3 acceleration;
};

///////////////////////////////////////////////////////////////////////

class ParticleSystem
{
public:
  virtual ~ParticleSystem(void);
  virtual void update(Time deltaTime);
  virtual void render(void) const;
  void addGravityPoint(const Vector3& point);
  void addGravityPlane(const Plane& plane);
  unsigned int getEmissionRate(void) const;
  void setEmissionRate(unsigned int newRate);
protected:
  ParticleSystem(void);
  typedef std::vector<Particle> ParticleList;
  ParticleList particles;
private:
  Time currentTime;
  unsigned int rate;
};

///////////////////////////////////////////////////////////////////////

class SpriteParticleSystem : public ParticleSystem
{
public:
  void update(Time deltaTime);
  void render(void) const;
  static ParticleSystem* createInstance(unsigned int particleCount);
private:
  Ptr<Shader> shader;
  Ptr<VertexBuffer> vertexBuffer;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_PARTICLE_H*/
///////////////////////////////////////////////////////////////////////
