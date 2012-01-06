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
#ifndef WENDY_RENDERSTYLE_H
#define WENDY_RENDERSTYLE_H
///////////////////////////////////////////////////////////////////////

#include <wendy/GLBuffer.h>
#include <wendy/GLTexture.h>
#include <wendy/GLProgram.h>
#include <wendy/GLState.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

///////////////////////////////////////////////////////////////////////

class System;

///////////////////////////////////////////////////////////////////////

/*! @brief Render phase enumeration.
 *  @ingroup renderer
 */
enum Phase
{
  /*! Normal forward rendering or G-buffer filling.
   */
  PHASE_DEFAULT,
  /*! Shadow map rendering.
   */
  PHASE_SHADOWMAP
};

///////////////////////////////////////////////////////////////////////

/*! @brief Render pass state object.
 *  @ingroup renderer
 */
class Pass : public GL::RenderState
{
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
typedef std::vector<Pass> PassList;

///////////////////////////////////////////////////////////////////////

/*! @brief Multipass %render technique.
 *  @ingroup renderer
 */
class Technique
{
public:
  /*! Creates a new %render pass in this technique.
   *  @return The newly created %render pass.
   *
   *  @remarks The passes are rendered in creation order.
   */
  Pass& createPass();
  /*! Removes the specified %render pass from this technique.
   */
  void destroyPass(Pass& pass);
  /*! Destroys all %render passes in this technique.
   */
  void destroyPasses();
  /*! @return The %render passes in this technique.
   */
  const PassList& getPasses() const;
private:
  PassList passes;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Multi-technique material descriptor.
 *  @ingroup renderer
 */
class Material : public Resource
{
public:
  /*! @return The technique for the specified phase.
   */
  Technique& getTechnique(Phase phase);
  /*! @return The technique for the specified phase.
   */
  const Technique& getTechnique(Phase phase) const;
  /*! Creates a material.
   *  @param[in] info The resource info for the texture.
   *  @param[in] system The OpenGL context within which to create the texture.
   *  @return The newly created material, or @c NULL if an error
   *  occurred.
   */
  static Ref<Material> create(const ResourceInfo& info, System& system);
  /*! Loads a material from the specified path using the specified system, or
   *  returns the already loaded material if it's already present in the
   *  resource cache of the system.
   *  @param[in] system The system to use.
   *  @param[in] path The path of the material.
   *  @return The loaded material, or @c NULL if an error occurred.
   */
  static Ref<Material> read(System& system, const String& name);
private:
  Material(const ResourceInfo& info);
  Technique techniques[2];
};

///////////////////////////////////////////////////////////////////////

/*! @brief Codec for XML format render materials.
 *  @ingroup renderer
 */
class MaterialReader : public ResourceReader<Material>
{
public:
  MaterialReader(System& system);
  using ResourceReader<Material>::read;
  Ref<Material> read(const String& name, const Path& path);
private:
  System& system;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERSTYLE_H*/
///////////////////////////////////////////////////////////////////////
