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

#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLState.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {

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
  /*! Technique type enumeration.
   */
  enum Type
  {
    /*! This technique is intended for the forward renderer.
     */
    FORWARD,
    /*! This technique is intended for the deferred renderer.
     */
    DEFERRED,
    /*! This technique is intended for rendering shadow maps.
     */
    SHADOWMAP,
  };
  /*! Constructor.
   *  @param[in] type The render type of the created technique.
   */
  Technique(Type type);
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
  /*! @return The type of this technique.
   */
  Type getType() const;
  /*! @return The quality of this technique.
   */
  float getQuality() const;
  /*! Sets the quality of this technique.
   *  @param[in] newQuality The quality to set.
   */
  void setQuality(float newQuality);
private:
  Type type;
  float quality;
  PassList passes;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup renderer
 */
typedef std::vector<Technique> TechniqueList;

///////////////////////////////////////////////////////////////////////

/*! @brief Multi-technique material descriptor.
 *  @ingroup renderer
 */
class Material : public Resource
{
public:
  /*! Constructor.
   */
  Material(const ResourceInfo& info);
  /*! Creates a technique of the specified type in this %render material.
   */
  Technique& createTechnique(Technique::Type type);
  /*! Destroys the specified technique.
   */
  void destroyTechnique(Technique& technique);
  /*! Destroys all techniques in this %render material.
   */
  void destroyTechniques();
  /*! Searches for the highest quality technique of the specified type.
   *  @param[in] type The desired technique type.
   *  @return The desired technique, or @c NULL if no technique exists of the
   *  specified type.
   */
  Technique* findBestTechnique(Technique::Type type);
  /*! Searches for the highest quality technique of the specified type.
   *  @param[in] type The desired technique type.
   *  @return The desired technique, or @c NULL if no technique exists of the
   *  specified type.
   */
  const Technique* findBestTechnique(Technique::Type type) const;
  /*! @return The techniques in this material.
   */
  TechniqueList& getTechniques();
  /*! @return The techniques in this material.
   */
  const TechniqueList& getTechniques() const;
  /*! Loads a material from the specified path using the specified context, or
   *  returns the already loaded material if it's already present in the
   *  resource cache of the context.
   *  @param[in] context The context to use.
   *  @param[in] path The path of the material.
   *  @return The loaded material, or @c NULL if an error occurred.
   */
  static Ref<Material> read(GL::Context& context, const Path& path);
private:
  TechniqueList techniques;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Codec for XML format render materials.
 *  @ingroup renderer
 */
class MaterialReader : ResourceReader
{
public:
  MaterialReader(GL::Context& context);
  Ref<Material> read(const Path& path);
private:
  GL::Context& context;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Codec for XML format render materials.
 *  @ingroup renderer
 */
class MaterialWriter
{
public:
  bool write(const Path& path, const Material& material);
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERSTYLE_H*/
///////////////////////////////////////////////////////////////////////
