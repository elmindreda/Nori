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
#ifndef WENDY_GLPARTICLE_H
#define WENDY_GLPARTICLE_H
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

class ParticleSystem;

///////////////////////////////////////////////////////////////////////

/*! @brief %Particle descriptor.
 *  @ingroup renderer
 */
class Particle
{
public:
  /*! The world-space position of this particle.
   */
  Vector3 position;
  /*! The world-space velocity vector of this particle.
   */
  Vector3 velocity;
  /*! The color and transparency values of this particle.
   */
  ColorRGBA color;
  /*! The rotation, in radians, of this particle around the z-axis.
   */
  float angle;
  /*! The life time, in seconds, of this particle.
   */
  Time duration;
  /*! The amount of time, in seconds, that this particle has been alive.
   */
  Time elapsed;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Base class for particle emitters.
 *  @ingroup renderer
 *
 *  Inherit from this to participate in a particle system by emitting
 *  particles and controlling the rate of particle emission.
 */
class ParticleEmitter
{
  friend class ParticleSystem;
protected:
  /*! Constructor.
   */
  ParticleEmitter(void);
  /*! Destructor.
   */
  virtual ~ParticleEmitter(void);
  /*! Called by the particle system to update the emitter and enquire how many
   *  particles this emitter wishes to emit during the specified time frame.
   *  @param deltaTime The time elapsed since the previous update.
   *  @return The number of particles this emitter wishes to emit during the
   *  specified time period.
   *  @remarks Since the number of particles in a system is fixed, it may not
   *  be possible for the system to create the desired number of particles.
   */
  virtual unsigned int update(Time deltaTime) = 0;
  /*! Called by the particle system when the time elapsed is reset to zero.
   */
  virtual void restart(void);
  /*! Called by the particle system to initialize a newly emitted particle.
   *  @param particle The particle to initialize.
   *  @param particleIndex The global index of the particle.
   */
  virtual void createParticle(Particle& particle,
                              unsigned int particleIndex) = 0;
  /*! @return The particle system that this emitter is owned by, or @c NULL if
   *  this emitter isn't owned by any system.
   */
  ParticleSystem* getSystem(void) const;
private:
  ParticleEmitter(const ParticleEmitter& source);
  ParticleEmitter& operator = (const ParticleEmitter& source);
  ParticleSystem* system;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Base class for particle affectors.
 *  @ingroup renderer
 *
 *  Inherit from this to participate in a particle system by affecting
 *  each living particle on each update of the system.
 */
class ParticleAffector
{
  friend class ParticleSystem;
protected:
  /*! Constructor.
   */
  ParticleAffector(void);
  /*! Destructor.
   */
  virtual ~ParticleAffector(void);
  /*! Called by the particle system to update the affector.
   *  @param deltaTime The time elapsed since the previous update.
   */
  virtual void update(Time deltaTime);
  /*! Called by the particle system when the time elapsed is reset to zero.
   */
  virtual void restart(void);
  /*! Called by the particle system to affect a living particle for the
   *  specified time period.
   *  @param particle The particle to affect.
   *  @param particleIndex The global index of the particle.
   *  @param deltaTime The time period, in seconds, for which the particle is
   *  to be affected.
   */
  virtual void affectParticle(Particle& particle,
                              unsigned int particleIndex,
			      Time deltaTime) = 0; 
  /*! @return The particle system that this affector is owned by, or @c NULL if
   *  this affector isn't owned by any system.
   */
  ParticleSystem* getSystem(void) const;
private:
  ParticleAffector(const ParticleAffector& source);
  ParticleAffector& operator = (const ParticleAffector& source);
  ParticleSystem* system;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Central particle system base class.
 *  @ingroup renderer
 *
 *  This is the central class of a particle system.
 *
 *  It contains all functionality necessary for particle systems rendered with
 *  billboards. If you wish to render particles using another method, inherit
 *  from this class.
 */
class ParticleSystem : public Renderable, public Managed<ParticleSystem>
{
public:
  enum PeriodType
  {
    FIXED_PERIOD,
    VARIABLE_PERIOD,
  };
  ParticleSystem(const String& name = "");
  ~ParticleSystem(void);
  void enqueue(RenderQueue& queue, const Transform3& transform) const;
  void addEmitter(ParticleEmitter& emitter);
  void removeEmitter(ParticleEmitter& emitter);
  void addAffector(ParticleAffector& affector);
  void removeAffector(ParticleAffector& affector);
  unsigned int getParticleCount(void) const;
  void setParticleCount(unsigned int newCount);
  const Vector2& getParticleSize(void) const;
  void setParticleSize(const Vector2& newSize);
  Time getTimeElapsed(void) const;
  void setTimeElapsed(Time newTime);
  PeriodType getPeriodType(void) const;
  void setPeriodType(PeriodType newType);
  const String& getStyleName(void) const;
  void setStyleName(const String& newName);
  const Transform3& getTransform(void) const;
  void setTransform(const Transform3& newTransform);
protected:
  virtual void addedParticle(Particle& particle,
                             unsigned int particleIndex);
  virtual void removedParticle(Particle& particle,
                               unsigned int particleIndex);
  virtual void restart(void);
  typedef std::vector<Particle> ParticleList;
  typedef std::list<unsigned int> ParticlePool;
  typedef std::list<ParticleEmitter*> EmitterList;
  typedef std::list<ParticleAffector*> AffectorList;
  ParticleList particles;
  ParticlePool activeParticles;
  ParticlePool passiveParticles;
  EmitterList emitters;
  AffectorList affectors;
private:
  ParticleSystem(const ParticleSystem& source);
  ParticleSystem& operator = (const ParticleSystem& source);
  bool realizeVertices(VertexBufferRange& range,
                       const Vector3& camera) const;
  Time currentTime;
  String styleName;
  Vector2 particleSize;
  PeriodType periodType;
  Transform3 transform;
  Sphere bounds;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup default
 */
class DefaultParticleEmitter : public ParticleEmitter
{
public:
  DefaultParticleEmitter(void);
  unsigned int update(Time deltaTime);
  void restart(void);
  void createParticle(Particle& particle,
                      unsigned int particleIndex);
  float getEmissionRate(void) const;
  void setEmissionRate(float newRate);
  const RandomRGBA& getColorRange(void) const;
  void setColorRange(const RandomRGBA& newRange);
  const RandomRange& getVelocityRange(void) const;
  void setVelocityRange(const RandomRange& newRange);
  const RandomRange& getDurationRange(void) const;
  void setDurationRange(const RandomRange& newRange);
  const RandomRange& getAngleRange(void) const;
  void setAngleRange(const RandomRange& newRange);
private:
  float rate;
  float fraction;
  RandomRGBA colorRange;
  RandomRange velocityRange;
  RandomRange durationRange;
  RandomRange angleRange;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup default
 */
class PlanarGravityParticleAffector : public ParticleAffector
{
public:
  PlanarGravityParticleAffector(void);
  void affectParticle(Particle& particle,
                      unsigned int particleIndex,
		      Time deltaTime);
  const Vector3& getGravity(void) const;
  void setGravity(const Vector3& newGravity);
private:
  Vector3 gravity;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLPARTICLE_H*/
///////////////////////////////////////////////////////////////////////
