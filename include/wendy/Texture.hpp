///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#pragma once

#include <wendy/Core.hpp>
#include <wendy/Rect.hpp>
#include <wendy/Path.hpp>
#include <wendy/Resource.hpp>
#include <wendy/Pixel.hpp>
#include <wendy/Image.hpp>

namespace wendy
{

class Texture;
class RenderContext;

/*! @brief %Texture sampler filtering mode enumeration.
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

/*! @brief %Texture sampler address wrapping mode enumeration.
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

/*! @brief %Texture type enumeration.
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

/*! @brief %Texture creation flags.
 */
enum TextureFlags
{
  TF_NONE      = 0x00,
  TF_MIPMAPPED = 0x01,
  TF_SRGB      = 0x02
};

/*! Cube map face enumeration.
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

/*! @brief %Texture creation data.
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

/*! @brief %Texture creation parameters.
 */
class TextureParams
{
public:
  TextureParams(TextureType type,
                uint flags,
                FilterMode filterMode = FILTER_BILINEAR,
                AddressMode addressMode = ADDRESS_WRAP,
                float maxAnisotropy = 1.f);
  TextureType type;
  FilterMode filterMode;
  AddressMode addressMode;
  float maxAnisotropy;
  uint flags;
};

/*! @brief %Texture image ID.
 */
class TextureImage
{
public:
  TextureImage(uint level = 0, CubeFace face = NO_CUBE_FACE);
  uint level;
  CubeFace face;
};

/*! @brief %Texture object.
 */
class Texture : public Resource, public RefObject
{
  friend class RenderContext;
  friend class TextureFramebuffer;
public:
  /*! Destructor.
   */
  ~Texture();
  /*! Updates an area within the specified texture image, at the specified
   *  coordinates and with a size matching the specified texture data, with the
   *  contents of that data.
   *  @param[in] data The data to copy from.
   *  @param[in] x The x-coordinate of the area within this image to update.
   *  @param[in] y The y-coordinate of the area within this image to update.
   *  @param[in] z The z-coordinate of the area within this image to update.
   */
  bool copyFrom(const TextureImage& image,
                const TextureData& data,
                uint x = 0, uint y = 0, uint z = 0);
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
  /*! @return @c true if this texture is a cubemap, otherwise @c false.
   */
  bool isCube() const;
  /*! @return @c true if this texture's dimensions are power of two, otherwise
   *  @c false.
   */
  bool isPOT() const;
  /*! @return @c true if this texture is mipmapped, otherwise @c false.
   */
  bool hasMipmaps() const { return m_levels > 1; }
  /*! @return The type of this texture.
   */
  TextureType type() const { return m_params.type; }
  /*! @param[in] level The desired mipmap level.
   *  @return The width, in pixels, of the specified mipmap level of this texture.
   */
  uint width(uint level = 0) const;
  /*! @param[in] level The desired mipmap level.
   *  @return The height, in pixels, of the specified mipmap level of this texture.
   */
  uint height(uint level = 0) const;
  /*! @param[in] level The desired mipmap level.
   *  @return The depth, in pixels, of the specified mipmap level of this texture.
   */
  uint depth(uint level = 0) const;
  /*! @return The size, in bytes, of the pixel data in this texture.
   */
  size_t size() const;
  /*! @return The number of mipmap levels of the specified image of this
   *  texture.
   */
  uint levelCount() const { return m_levels; }
  /*! Returns a copy the contents of the specified image of this texture image.
   *  @return An image object containing the pixel data.
   */
  Ref<Image> data(const TextureImage& image);
  /*! @return The sampler filter mode of this texture.
   */
  FilterMode filterMode() const { return m_params.filterMode; }
  /*! @return The sampler address wrapping mode of this texture.
   */
  AddressMode addressMode() const { return m_params.addressMode; }
  /*! @return The maximum anisotropy of this texture.
   */
  float maxAnisotropy() const { return m_params.maxAnisotropy; }
  /*! @return The image format of this texture.
   */
  const PixelFormat& format() const { return m_format; }
  /*! @return The context used to create this texture.
   */
  RenderContext& context() const { return m_context; }
  /*! Creates a texture from the specified image.
   *  @param[in] context The render context within which to create the
   *  texture.
   *  @param[in] params The creation parameters for the texture.
   *  @param[in] data The pixel data to use.
   *  @return The newly created texture object.
   */
  static Ref<Texture> create(const ResourceInfo& info,
                             RenderContext &context,
                             const TextureParams& params,
                             const TextureData& data);
  static Ref<Texture> read(RenderContext& context,
                           const TextureParams& params,
                           const std::string& imageName);
private:
  Texture(const ResourceInfo& info,
          RenderContext& context,
          const TextureParams& params);
  Texture(const Texture&) = delete;
  bool init(const TextureData& data);
  void attach(int attachment, const TextureImage& image, uint z);
  void detach(int attachment);
  Texture& operator = (const Texture&) = delete;
  RenderContext& m_context;
  const TextureParams m_params;
  uint m_textureID;
  uint m_width;
  uint m_height;
  uint m_depth;
  uint m_levels;
  PixelFormat m_format;
};

} /*namespace wendy*/

