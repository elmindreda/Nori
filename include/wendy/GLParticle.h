///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <list>
#include <vector>

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
  float angle;
  Time duration;
  Time elapsed;
};

///////////////////////////////////////////////////////////////////////

class ParticleEmitter
{
protected:
  virtual unsigned int update(Time deltaTime);
  virtual void createParticle(Particle& particle,
                              unsigned int particleIndex) = 0;
  /*! @return The number of particles emitted per second.
   */
  float getEmissionRate(void) const;
  void setEmissionRate(float newRate);
private:
  float rate;
  float emitted;
};

///////////////////////////////////////////////////////////////////////

class ParticleAffector
{
protected:
  virtual void affectParticle(Particle& particle,
                              unsigned int particleIndex) = 0; 
};

///////////////////////////////////////////////////////////////////////

class ParticleSystem : public Managed<ParticleSystem>
{
public:
  ParticleSystem(unsigned int particleCount, const String& name = "");
  ~ParticleSystem(void);
  void enqueue(RenderQueue& queue, const Matrix4& transform) const;
  void render(void) const;
  Time getTimeElapsed(void) const;
  void setTimeElapsed(Time newTime);
protected:
  typedef std::vector<Particle> ParticleList;
  typedef std::list<Particle*> ActiveParticleList;
  ParticleList particles;
  ActiveParticleList activeParticles;
private:
  Time currentTime;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_PARTICLE_H*/
///////////////////////////////////////////////////////////////////////
