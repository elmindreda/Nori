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

#include <moira/Moira.h>

#include <wendy/Config.h>
#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLLight.h>
#include <wendy/GLPass.h>
#include <wendy/GLRender.h>

#include <wendy/RenderCamera.h>
#include <wendy/RenderStyle.h>
#include <wendy/RenderQueue.h>
#include <wendy/RenderParticle.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

ParticleEmitter::ParticleEmitter(void):
  system(NULL)
{
}

ParticleEmitter::~ParticleEmitter(void)
{
}

void ParticleEmitter::restart(void)
{
}

ParticleSystem* ParticleEmitter::getSystem(void) const
{
  return system;
}

ParticleEmitter::ParticleEmitter(const ParticleEmitter& source)
{
  // NOTE: Not implemented.
}

ParticleEmitter& ParticleEmitter::operator = (const ParticleEmitter& source)
{
  // NOTE: Not implemented.

  return *this;
}

///////////////////////////////////////////////////////////////////////

ParticleAffector::ParticleAffector(void):
  system(NULL)
{
}

ParticleAffector::~ParticleAffector(void)
{
}

void ParticleAffector::update(Time deltaTime)
{
}

void ParticleAffector::restart(void)
{
}

ParticleSystem* ParticleAffector::getSystem(void) const
{
  return system;
}

ParticleAffector::ParticleAffector(const ParticleAffector& source)
{
  // NOTE: Not implemented.
}

ParticleAffector& ParticleAffector::operator = (const ParticleAffector& source)
{
  // NOTE: Not implemented.

  return *this;
}

///////////////////////////////////////////////////////////////////////

ParticleSystem::ParticleSystem(const String& name):
  Managed<ParticleSystem>(name),
  updateBounds(true),
  particleSize(1.f, 1.f),
  currentTime(0.0),
  periodType(VARIABLE_PERIOD)
{
}

ParticleSystem::~ParticleSystem(void)
{
  while (emitters.size())
  {
    delete emitters.front();
    emitters.pop_front();
  }

  while (affectors.size())
  {
    delete affectors.front();
    affectors.pop_front();
  } 
}

void ParticleSystem::enqueue(Queue& queue, const Transform3& transform) const
{
  if (!activeParticles.size())
    return;

  GL::VertexRange range;

  if (!realizeVertices(range, queue.getCamera().getTransform().position))
    return;

  Style* style = Style::findInstance(styleName);
  if (!style)
  {
    Log::writeError("Render style %s not found", styleName.c_str());
    return;
  }

  Technique* technique = style->getActiveTechnique();
  if (!technique)
  {
    Log::writeError("Render style %s has no active technique", style->getName().c_str());
    return;
  }

  Operation& operation = queue.createOperation();
  operation.vertexBuffer = range.getVertexBuffer();
  operation.start = range.getStart();
  operation.count = range.getCount();
  operation.technique = technique;
  operation.renderMode = GL_QUADS;
}

void ParticleSystem::addEmitter(ParticleEmitter& emitter)
{
  if (std::find(emitters.begin(), emitters.end(), &emitter) != emitters.end())
    return;

  if (emitter.system)
    emitter.system->removeEmitter(emitter);

  emitters.push_front(&emitter);
  emitter.system = this;
}

void ParticleSystem::removeEmitter(ParticleEmitter& emitter)
{
  emitters.remove(&emitter);
  emitter.system = NULL;
}

void ParticleSystem::addAffector(ParticleAffector& affector)
{
  if (std::find(affectors.begin(), affectors.end(), &affector) != affectors.end())
    return;

  if (affector.system)
    affector.system->removeAffector(affector);

  affectors.push_front(&affector);
  affector.system = this;
}

void ParticleSystem::removeAffector(ParticleAffector& affector)
{
  affectors.remove(&affector);
  affector.system = NULL;
}

bool ParticleSystem::isUpdatingBounds(void) const
{
  return updateBounds;
}

void ParticleSystem::setUpdatesBounds(bool newState)
{
  updateBounds = newState;
}

const Sphere& ParticleSystem::getBounds(void) const
{
  return bounds;
}

unsigned int ParticleSystem::getParticleCount(void) const
{
  return particles.size();
}

void ParticleSystem::setParticleCount(unsigned int newCount)
{
  const unsigned int oldCount = particles.size();

  if (newCount == oldCount)
    return;

  if (newCount < oldCount)
  {
    for (unsigned int i = newCount;  i < oldCount;  i++)
    {
      removedParticle(particles[i], i);
      passiveParticles.remove(i);
      activeParticles.remove(i);
    }

    restart();
  }

  particles.resize(newCount);

  if (newCount > oldCount)
  {
    for (unsigned int i = oldCount;  i < newCount;  i++)
    {
      addedParticle(particles[i], i);
      passiveParticles.push_front(i);
    }
  }
}

const Vector2& ParticleSystem::getParticleSize(void) const
{
  return particleSize;
}

void ParticleSystem::setParticleSize(const Vector2& newSize)
{
  particleSize = newSize;
}

Time ParticleSystem::getTimeElapsed(void) const
{
  return currentTime;
}

void ParticleSystem::setTimeElapsed(Time newTime)
{
  Time deltaTime = newTime - currentTime;

  // TODO: Implement fixed frequencey updating.

  if (deltaTime < 0.f)
  {
    restart();
    deltaTime = newTime;
  }

  if (deltaTime == 0.f)
    return;

  bounds.center = transform.position;
  bounds.radius = 0.f;

  // Emit particles created during this time frame
  // TODO: Affect particles by their position within the timeframe (lerp),
  //       instead of placing all of them at the beginning of it.

  for (EmitterList::const_iterator i = emitters.begin();  i != emitters.end();  i++)
  {
    unsigned int count = (*i)->update(deltaTime);
    if (count > passiveParticles.size())
      count = passiveParticles.size();

    for (unsigned int j = 0;  j < count;  j++)
    {
      activeParticles.push_front(passiveParticles.front());
      passiveParticles.pop_front();

      Particle& particle = particles[activeParticles.front()];
      (*i)->createParticle(particle, &particle - &(particles.front()));
    }
  }

  // Affect all living particles for this time frame

  for (ParticlePool::iterator p = activeParticles.begin();  p != activeParticles.end(); )
  {
    for (AffectorList::const_iterator a = affectors.begin();  a != affectors.end();  a++)
    {
      Particle& particle = particles[*p];

      (*a)->affectParticle(particle, &particle - &(particles.front()), deltaTime);

      particle.elapsed += deltaTime;
      if (particle.elapsed < particle.duration)
      {
	if (updateBounds)
	  bounds.envelop(particle.position);

	p++;
      }
      else
      {
	passiveParticles.push_front(*p);
	p = activeParticles.erase(p);
      }
    }
  }

  currentTime = newTime;
}

ParticleSystem::PeriodType ParticleSystem::getPeriodType(void) const
{
  return periodType;
}

void ParticleSystem::setPeriodType(PeriodType newType)
{
  periodType = newType;
}

const String& ParticleSystem::getStyleName(void) const
{
  return styleName;
}

void ParticleSystem::setStyleName(const String& newName)
{
  styleName = newName;
}

const Transform3& ParticleSystem::getTransform(void) const
{
  return transform;
}

void ParticleSystem::setTransform(const Transform3& newTransform)
{
  transform = newTransform;
}

void ParticleSystem::addedParticle(Particle& particle,
			           unsigned int particleIndex)
{
}

void ParticleSystem::removedParticle(Particle& particle,
			             unsigned int particleIndex)
{
}

ParticleSystem::ParticleSystem(const ParticleSystem& source):
  Managed<ParticleSystem>(source)
{
  // NOTE: Not implemented.
}

ParticleSystem& ParticleSystem::operator = (const ParticleSystem& source)
{
  // NOTE: Not implemented.

  return *this;
}

bool ParticleSystem::realizeVertices(GL::VertexRange& range,
		                     const Vector3& camera) const
{
  GL::Renderer* renderer = GL::Renderer::get();
  if (!renderer)
  {
    Log::writeError("Cannot render particles without a renderer");
    return false;
  }

  if (!renderer->allocateVertices(range,
                                  activeParticles.size() * 4, 
				  GL::Vertex4fc2ft3fv::format))
    return false;

  GL::Vertex4fc2ft3fv* vertices = (GL::Vertex4fc2ft3fv*) range.lock();
  if (!vertices)
    return false;

  const Vector2 offset(particleSize.x / 2.f, particleSize.y / 2.f);

  for (ParticlePool::const_iterator i = activeParticles.begin();  i != activeParticles.end();  i++)
  {  
    const Particle& particle = particles[*i];

    // TODO: Fix this (separate y-axis rotation and pivot)

    Vector3 direction = (camera - particle.position).normalize();

    Quaternion final;
    final.setVectorRotation(direction);

    if (particle.angle != 0.f)
    {
      Quaternion local;
      local.setAxisRotation(Vector3(0.f, 0.f, 1.f), particle.angle);

      final = final * local;
    }

    Vector3 positions[4];

    positions[0].set(-offset.x, -offset.y, 0.f);
    positions[1].set( offset.x, -offset.y, 0.f);
    positions[2].set( offset.x,  offset.y, 0.f);
    positions[3].set(-offset.x,  offset.y, 0.f);

    for (unsigned int i = 0;  i < 4;  i++)
    {
      final.rotateVector(positions[i]);
      positions[i] += particle.position;
    }

    vertices[0].color = particle.color;
    vertices[0].mapping.set(0.f, 0.f);
    vertices[0].position = positions[0];
    vertices[1].color = particle.color;
    vertices[1].mapping.set(1.f, 0.f);
    vertices[1].position = positions[1];
    vertices[2].color = particle.color;
    vertices[2].mapping.set(1.f, 1.f);
    vertices[2].position = positions[2];
    vertices[3].color = particle.color;
    vertices[3].mapping.set(0.f, 1.f);
    vertices[3].position = positions[3];

    vertices += 4;
  }

  range.unlock();

  return true;
}

void ParticleSystem::restart(void)
{
  for (EmitterList::const_iterator i = emitters.begin();  i != emitters.end();  i++)
    (*i)->restart();

  for (AffectorList::const_iterator i = affectors.begin();  i != affectors.end();  i++)
    (*i)->restart();

  activeParticles.clear();
  currentTime = 0.f;
}

///////////////////////////////////////////////////////////////////////

DefaultParticleEmitter::DefaultParticleEmitter(void):
  rate(0.f),
  fraction(0.f)
{
}

unsigned int DefaultParticleEmitter::update(Time deltaTime)
{
  const float count = deltaTime * rate + fraction;
  fraction = count - floorf(count);
  return (unsigned int) count;
}

void DefaultParticleEmitter::restart(void)
{
  fraction = 0.f;
}

void DefaultParticleEmitter::createParticle(Particle& particle,
		                            unsigned int particleIndex)
{
  particle.color = colorRange.generate();
  particle.position = originVolume.generate();
  particle.velocity = Vector3(0.f, velocityRange.generate(), 0.f);
  particle.duration = durationRange.generate();
  particle.elapsed = 0.f;

  getSystem()->getTransform().transformVector(particle.position);

  Quaternion rotation;
  rotation.setAxisRotation(Vector3(1.f, 0.f, 0.f), angleRange.generate());
  rotation.rotateVector(particle.velocity);
  rotation.setAxisRotation(Vector3(0.f, 1.f, 0.f), M_PI * 2.f * normalizedRandom());
  rotation.rotateVector(particle.velocity);

  getSystem()->getTransform().rotateVector(particle.velocity);
}

float DefaultParticleEmitter::getEmissionRate(void) const
{
  return rate;
}

void DefaultParticleEmitter::setEmissionRate(float newRate)
{
  rate = newRate;
}

const RandomRGBA& DefaultParticleEmitter::getColorRange(void) const
{
  return colorRange;
}

void DefaultParticleEmitter::setColorRange(const RandomRGBA& newRange)
{
  colorRange = newRange;
}

const RandomRange& DefaultParticleEmitter::getVelocityRange(void) const
{
  return velocityRange;
}

void DefaultParticleEmitter::setVelocityRange(const RandomRange& newRange)
{
  velocityRange = newRange;
}

const RandomRange& DefaultParticleEmitter::getDurationRange(void) const
{
  return durationRange;
}

void DefaultParticleEmitter::setDurationRange(const RandomRange& newRange)
{
  durationRange = newRange;
}

const RandomRange& DefaultParticleEmitter::getAngleRange(void) const
{
  return angleRange;
}

void DefaultParticleEmitter::setAngleRange(const RandomRange& newRange)
{
  angleRange = newRange;
}

const RandomVolume& DefaultParticleEmitter::getOriginVolume(void) const
{
  return originVolume;
}

void DefaultParticleEmitter::setOriginVolume(const RandomVolume& newVolume)
{
  originVolume = newVolume;
}

///////////////////////////////////////////////////////////////////////

PlanarGravityParticleAffector::PlanarGravityParticleAffector(void):
  gravity(0.f, -9.81f, 0.f)
{
}

void PlanarGravityParticleAffector::affectParticle(Particle& particle,
                                                   unsigned int particleIndex,
		                                   Time deltaTime)
{
  // TODO: Replace with verlet or better integrator.

  particle.position += particle.velocity * deltaTime +
                       gravity * deltaTime * deltaTime;
  particle.velocity += gravity * deltaTime;
}

const Vector3& PlanarGravityParticleAffector::getGravity(void) const
{
  return gravity;
}

void PlanarGravityParticleAffector::setGravity(const Vector3& newGravity)
{
  gravity = newGravity;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
