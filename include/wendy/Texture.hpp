///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_TEXTURE_HPP
#define WENDY_TEXTURE_HPP
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.hpp>
#include <wendy/Rect.hpp>
#include <wendy/Path.hpp>
#include <wendy/Resource.hpp>
#include <wendy/Pixel.hpp>
#include <wendy/Image.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class Texture;
class RenderContext;

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture sampler filtering mode enumeration.
 *  @ingroup opengl
 */
enum FilterMode
{
  /*! Select the nearest texel of the nearest mipmap level.
   */
  FILTER_NEAREST,
  /*! Select the four nearest texels of the nearest mipmap level and
   *  interpolate.
   */
  FILTER_BILINEAR,
  /*! Select the four nearest texels of the two nearest mipmap levels and
   *  interpolate.
   */
  FILTER_TRILINEAR
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture sampler address wrapping mode enumeration.
 *  @ingroup opengl
 */
enum AddressMode
{
  /*! Addresses are wrapped to within the texture.
   */
  ADDRESS_WRAP,
  /*! Addresses are clamped to the edges of the texture.
   */
  ADDRESS_CLAMP
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture type enumeration.
 *  @ingroup opengl
 */
enum TextureType
{
  /*! %Texture has one-dimensional images with power-of-two dimensions.
   */
  TEXTURE_1D,
  /*! %Texture has two-dimensional images with power-of-two dimensions.
   */
  TEXTURE_2D,
  /*! %Texture has three-dimensional images with power-of-two dimensions.
   */
  TEXTURE_3D,
  /*! %Texture has a single two-dimensional image without power-of-two
   *  dimension restrictions.
   */
  TEXTURE_RECT,
  /*! %Texture has a cube of two-dimensional, square images with power-of-two
   *  dimensions.
   */
  TEXTURE_CUBE
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture creation flags.
 *  @ingroup opengl
 */
enum TextureFlags
{
  TF_NONE      = 0x00,
  TF_MIPMAPPED = 0x01,
  TF_SRGB      = 0x02
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
enum CubeFace
{
  CUBE_POSITIVE_X,
  CUBE_NEGATIVE_X,
  CUBE_POSITIVE_Y,
  CUBE_NEGATIVE_Y,
  CUBE_POSITIVE_Z,
  CUBE_NEGATIVE_Z,
  NO_CUBE_FACE
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture creation data.
 *  @ingroup opengl
 */
class TextureData
{
public:
  TextureData(const Image& image);
  TextureData(PixelFormat format,
              uint width,
              uint height = 1,
              uint depth = 1,
              const void* texels = nullptr);
  bool isPOT() const;
  uint dimensionCount() const;
  PixelFormat format;
  uint width;
  uint height;
  uint depth;
  const void* texels;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture creation parameters.
 *  @ingroup opengl
 */
class TextureParams
{
public:
  TextureParams(TextureType type, uint flags);
  TextureType type;
  uint flags;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture image object.
 *  @ingroup opengl
 */
class TextureImage : public RefObject
{
  friend class Texture;
  friend class TextureFramebuffer;
public:
  /*! Updates an area within this texture image, at the specified coordinates
   *  and with a size matching the specified image, with the contents of that
   *  image.
   *  @param[in] source The image to copy pixel data from.
   *  @param[in] x The x-coordinate of the area within this image to update.
   *  @param[in] y The y-coordinate of the area within this image to update.
   *  @param[in] z The z-coordinate of the area within this image to update.
   */
  bool copyFrom(const Image& source, uint x = 0, uint y = 0, uint z = 0);
  /*! Returns a copy the contents of this texture image.
   *  @return An image object containing the image data.
   */
  Ref<Image> data() const;
  uint width() const { return m_width; }
  uint height() const { return m_height; }
  uint depth() const { return m_depth; }
  /*! @return The size, in bytes, of the data in this image.
   */
  size_t size() const;
  /*! @return The cube face this image represents, or @c NO_CUBE_FACE if this
   *  image is not part of a cube map.
   */
  CubeFace face() const { return m_face; }
  /*! @return The texture containing this texture image.
   */
  Texture& texture() const { return m_texture; }
private:
  TextureImage(Texture& texture,
               uint level,
               uint width,
               uint height,
               uint depth,
               CubeFace face = NO_CUBE_FACE);
  TextureImage(const TextureImage&) = delete;
  void attach(int attachment, uint z);
  void detach(int attachment);
  TextureImage& operator = (const TextureImage&) = delete;
  Texture& m_texture;
  uint m_level;
  uint m_width;
  uint m_height;
  uint m_depth;
  CubeFace m_face;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture object.
 *  @ingroup opengl
 */
class Texture : public Resource, public RefObject
{
  friend class RenderContext;
  friend class TextureImage;
public:
  /*! Destructor.
   */
  ~Texture();
  /*! Generates mipmaps based on the top-level image.
   */
  void generateMipmaps();
  /*! @return @c true if this texture is one-dimensional, otherwise @c false.
   */
  bool is1D() const;
  /*! @return @c true if this texture is two-dimensional, otherwise @c false.
   */
  bool is2D() const;
  /*! @return @c true if this texture is three-dimensional, otherwise @c false.
   */
  bool is3D() const;
  /*! @return @c true if this texture's dimensions are power of two, otherwise
   *  @c false.
   */
  bool isPOT() const;
  /*! @return @c true if this texture is a cubemap, otherwise @c false.
   */
  bool isCube() const;
  /*! @return @c true if this texture is mipmapped, otherwise @c false.
   */
  bool hasMipmaps() const { return m_levels > 1; }
  /*! @return The type of this texture.
   */
  TextureType type() const { return m_type; }
  /*! @param[in] level The desired mipmap level.
   *  @return The width, in pixels, of the specified mipmap level of this texture.
   */
  uint width(uint level = 0) const { return image(level).width(); }
  /*! @param[in] level The desired mipmap level.
   *  @return The height, in pixels, of the specified mipmap level of this texture.
   */
  uint height(uint level = 0) const { return image(level).height(); }
  /*! @param[in] level The desired mipmap level.
   *  @return The depth, in pixels, of the specified mipmap level of this texture.
   */
  uint depth(uint level = 0) const { return image(level).depth(); }
  /*! @return The number of mipmap levels of this texture.
   */
  uint levelCount() const { return m_levels; }
  /*! @param[in] level The desired mipmap level.
   *  @param[in] face The desired cube map face if this texture is a cubemap,
   *  or @c NO_CUBE_FACE otherwise.
   */
  TextureImage& image(uint level = 0, CubeFace face = NO_CUBE_FACE);
  /*! @param[in] level The desired mipmap level.
   *  @param[in] face The desired cube map face if this texture is a cubemap,
   *  or @c NO_CUBE_FACE otherwise.
   */
  const TextureImage& image(uint level = 0, CubeFace face = NO_CUBE_FACE) const;
  /*! @return The sampler filter mode of this texture.
   */
  FilterMode filterMode() const { return m_filterMode; }
  /*! Sets the sampler filter mode of this texture.
   *  @param[in] newMode The desired filter mode.
   */
  void setFilterMode(FilterMode newMode);
  /*! @return The sampler address wrapping mode of this texture.
   */
  AddressMode addressMode() const { return m_addressMode; }
  /*! Sets the sampler address wrapping mode of this texture.
   *  @param[in] newMode The desired address wrapping mode.
   */
  void setAddressMode(AddressMode newMode);
  /*! @return The maximum anisotropy of this texture.
   */
  float maxAnisotropy() const { return m_maxAnisotropy; }
  /*! Sets the maximum anisotropy of this texture.
   *  @param[in] newMax The desired value.
   */
  void setMaxAnisotropy(float newMax);
  /*! @return The image format of this texture.
   */
  const PixelFormat& format() const { return m_format; }
  /*! @return The size, in bytes, of the data in all images of this texture.
   */
  size_t size() const;
  /*! @return The context used to create this texture.
   */
  RenderContext& context() const { return m_context; }
  /*! Creates a texture from the specified image.
   *  @param[in] context The OpenGL context within which to create the
   *  texture.
   *  @param[in] params The creation parameters for the texture.
   *  @param[in] data The image data to use.
   *  @return The newly created texture object.
   */
  static Ref<Texture> create(const ResourceInfo& info,
                             RenderContext &context,
                             const TextureParams& params,
                             const TextureData& data);
  static Ref<Texture> read(RenderContext& context,
                           const TextureParams& params,
                           const String& imageName);
private:
  Texture(const ResourceInfo& info, RenderContext& context);
  Texture(const Texture&) = delete;
  bool init(const TextureParams& params, const TextureData& data);
  void retrieveImages();
  uint retrieveTargetImages(uint target, CubeFace face);
  void applyDefaults();
  Texture& operator = (const Texture&) = delete;
  RenderContext& m_context;
  TextureType m_type;
  uint m_textureID;
  uint m_levels;
  FilterMode m_filterMode;
  AddressMode m_addressMode;
  float m_maxAnisotropy;
  PixelFormat m_format;
  std::vector<Ref<TextureImage>> m_images;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
typedef std::vector<Ref<Texture>> TextureList;

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_TEXTURE_HPP*/
///////////////////////////////////////////////////////////////////////
