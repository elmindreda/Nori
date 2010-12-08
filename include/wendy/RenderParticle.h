///////////////////////////////////////////////////////////////////////
// Wendy default renderer
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
#ifndef WENDY_RENDERPARTICLE_H
#define WENDY_RENDERPARTICLE_H
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Vector.h>
#include <wendy/Sphere.h>
#include <wendy/Random.h>

#include <list>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

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
  Vec3 position;
  /*! The world-space velocity vector of this particle.
   */
  Vec3 velocity;
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
  /*! The 2D size, in units, of the particle.
   */
  Vec2 size;
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
class ParticleSystem : public Renderable, public RefObject
{
public:
  enum PeriodType
  {
    FIXED_PERIOD,
    VARIABLE_PERIOD,
  };
  /*! Constructor.
   */
  ParticleSystem(void);
  /*! Destructor.
   */
  ~ParticleSystem(void);
  void enqueue(Queue& queue, const Transform3& transform) const;
  /*! Adds the specified emitter to this particle system.
   *  @param[in] emitter The emitter to add.
   */
  void addEmitter(ParticleEmitter& emitter);
  /*! Removes the specified emitter from this particle system.
   *  @param[in] emitter The emitter to remove.
   */
  void removeEmitter(ParticleEmitter& emitter);
  /*! Adds the specified affector to this particle system.
   *  @param[in] affector The affector to add.
   */
  void addAffector(ParticleAffector& affector);
  /*! Removes the specified affector from this particle system.
   *  @param[in] affector The affector to remove.
   */
  void removeAffector(ParticleAffector& affector);
  /*! @return @c true if this system ensures its bounds contain all active
   *  particles on each update.
   */
  bool isUpdatingBounds(void) const;
  /*! Sets whether this system ensures its bounds contain all active particles
   *  on each update.
   *  @param[in] newState @c true to activate bounds updating, or @c false to
   *  deactivate it.
   */
  void setUpdatesBounds(bool newState);
  /*! @return The bounds of this particle system.
   */
  const Sphere& getBounds(void) const;
  /*! @return The number of particles in this particle system.
   */
  unsigned int getParticleCount(void) const;
  /*! Sets the specified number of particles in this particle system.
   *  @param[in] newCount The desired number of particles.
   */
  void setParticleCount(unsigned int newCount);
  /*! @return The time elapsed, in seconds, since this particle system was
   *  started.
   */
  Time getTimeElapsed(void) const;
  /*! Sets the total time elapsed, in seconds, for this particle system.
   */
  void setTimeElapsed(Time newTime);
  /*! @return The update period type of this particle system.
   */
  PeriodType getPeriodType(void) const;
  /*! Sets the update period type of this particle system.
   *  @param[in] newType The desired update period type.
   */
  void setPeriodType(PeriodType newType);
  /*! @return The render material of this particle system.
   */
  Material* getMaterial(void) const;
  /*! Sets the render material of this particle system.
   *  @param[in] newMaterial The desired render material.
   */
  void setMaterial(Material* newMaterial);
  /*! @return The local-to-world transform of this particle system.
   */
  const Transform3& getTransform(void) const;
  /*! Sets the local-to-world transform of this particle system.
   *  @param[in] newTransform The desired local-to-world transform.
   */
  void setTransform(const Transform3& newTransform);
protected:
  /*! Called for each particle added to this particle system.
   */
  virtual void addedParticle(Particle& particle,
                             unsigned int particleIndex);
  /*! Called for each particle remove from this particle system.
   */
  virtual void removedParticle(Particle& particle,
                               unsigned int particleIndex);
  /*! Called when the elapsed time for this particle system is reset.
   */
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
  bool realizeVertices(GL::VertexRange& vertices,
                       GL::IndexRange& indices,
		       const Vec3& camera) const;
  bool updateBounds;
  Time currentTime;
  Ref<Material> material;
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
  const RandomVolume& getOriginVolume(void) const;
  void setOriginVolume(const RandomVolume& newVolume);
  const RandomRange& getSizeRange(void) const;
  void setSizeRange(const RandomRange& newRange);
private:
  float rate;
  float fraction;
  RandomRGBA colorRange;
  RandomRange velocityRange;
  RandomRange durationRange;
  RandomRange angleRange;
  RandomVolume originVolume;
  RandomRange sizeRange;
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
  const Vec3& getGravity(void) const;
  void setGravity(const Vec3& newGravity);
private:
  Vec3 gravity;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup default
 */
class ColorFadeParticleAffector : public ParticleAffector
{
public:
  ColorFadeParticleAffector(void);
  void affectParticle(Particle& particle,
                      unsigned int particleIndex,
		      Time deltaTime);
  const ColorRGBA& getStartColor(void) const;
  void setStartColor(const ColorRGBA& newColor);
  const ColorRGBA& getEndColor(void) const;
  void setEndColor(const ColorRGBA& newColor);
private:
  ColorRGBA start;
  ColorRGBA end;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERPARTICLE_H*/
///////////////////////////////////////////////////////////////////////
