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
#ifndef WENDY_MATERIAL_HPP
#define WENDY_MATERIAL_HPP
///////////////////////////////////////////////////////////////////////

#include <wendy/Texture.hpp>
#include <wendy/RenderBuffer.hpp>
#include <wendy/Program.hpp>

///////////////////////////////////////////////////////////////////////

namespace pugi { class xml_node; }

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief Render phase enumeration.
 *  @ingroup renderer
 */
enum RenderPhase
{
  /*! Normal forward rendering or G-buffer filling.
   */
  RENDER_DEFAULT,
  /*! Shadow map rendering.
   */
  RENDER_SHADOWMAP
};

///////////////////////////////////////////////////////////////////////

bool parsePass(RenderContext& context, Pass& pass, pugi::xml_node root);

///////////////////////////////////////////////////////////////////////

/*! @brief Multi-technique material descriptor.
 *  @ingroup renderer
 */
class Material : public Resource, public RefObject
{
public:
  /*! @return The pass for the specified render phase.
   */
  Pass& pass(RenderPhase phase) { return m_passes[phase]; }
  /*! @return The pass for the specified render phase.
   */
  const Pass& pass(RenderPhase phase) const { return m_passes[phase]; }
  /*! Creates a material.
   *  @param[in] info The resource info for the texture.
   *  @param[in] context The render context within which to create the texture.
   *  @return The newly created material, or @c nullptr if an error
   *  occurred.
   */
  static Ref<Material> create(const ResourceInfo& info, RenderContext& context);
  /*! Loads a material from the specified path using the specified render
   *  context, or returns the already loaded material if it's already present in
   *  the resource cache of the context.
   *  @param[in] context The render context to use.
   *  @param[in] path The path of the material.
   *  @return The loaded material, or @c nullptr if an error occurred.
   */
  static Ref<Material> read(RenderContext& context, const String& name);
private:
  Material(const ResourceInfo& info);
  Pass m_passes[2];
};

///////////////////////////////////////////////////////////////////////

/*! @brief Codec for XML format render materials.
 *  @ingroup renderer
 */
class MaterialReader : public ResourceReader<Material>
{
public:
  MaterialReader(RenderContext& context);
  using ResourceReader<Material>::read;
  Ref<Material> read(const String& name, const Path& path);
private:
  RenderContext& m_context;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_MATERIAL_HPP*/
///////////////////////////////////////////////////////////////////////
