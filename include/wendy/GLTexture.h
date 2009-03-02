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
#ifndef WENDY_GLTEXTURE_H
#define WENDY_GLTEXTURE_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture image object.
 *  @ingroup opengl
 *
 *  @remarks This class intentionally has no public interface for
 *  making a texture object current.
 */
class Texture : public Resource<Texture>, public RefObject<Texture>
{
  friend class ShaderUniform;
public:
  enum
  {
    /*! The texture will be created with the original image dimensions,
     *  if possible.  Note that such textures are slower when rendering.
     */
    RECTANGULAR = 1,
    /*! The texture will be created with a mipmap chain.
     */
    MIPMAPPED = 2,
    /*! The texture is guaranteed not to be larger than the source image.
     */
    DONT_GROW = 4,
    /*! The default texture creation flags.
     */
    DEFAULT = MIPMAPPED,
  };
  /*! Destructor.
   */
  ~Texture(void);
  /*! Updates the area at the specified coordinates in the specified mipmap
   *  level with the specified image.
   */
  bool copyFrom(const Image& source,
                unsigned int x,
		unsigned int y,
		unsigned int level = 0);
  /*! @return The OpenGL target of this texture.
   */
  GLenum getTarget(void) const;
  /*! @return The width, in pixels, of the source for specified mipmap level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getWidth(unsigned int level = 0) const;
  /*! @return The height, in pixels, of the source for specified mipmap level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getHeight(unsigned int level = 0) const;
  /*! @return The depth, in pixels, of the source for the specified mipmap
   *  level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getDepth(unsigned int level = 0) const;
  /*! @return The width, in pixels, of the specified mipmap level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getPhysicalWidth(unsigned int level = 0) const;
  /*! @return The height, in pixels, of the specified mipmap level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getPhysicalHeight(unsigned int level = 0) const;
  /*! @return The depth, in pixels, of the specified mipmap level.
   *  @param[in] level The desired mipmap level.
   */
  unsigned int getPhysicalDepth(unsigned int level = 0) const;
  /*! @return The number of mipmap levels in this texture.
   */
  unsigned int getLevelCount(void) const;
  /*! @return The creation flags for this texture.
   */
  unsigned int getFlags(void) const;
  /*! @return The image format of this texture.
   */
  const ImageFormat& getFormat(void) const;
  /*! @param[in] level The desired mipmap level.
   *  @return The image data of the specified mipmap level.
   */
  Image* getImage(unsigned int level = 0) const;
  static Texture* readInstance(const String& name,
                               unsigned int flags = DEFAULT);
  /*! Creates a texture from the specified image file.
   *  @param[in] path The path of the image file to use.
   *  @param[in] flags The creation flags.
   *  @param[in] name The desired name of the texture.
   */
  static Texture* readInstance(const Path& path,
                               unsigned int flags = DEFAULT,
			       const String& name = "");
  /*! Creates a texture from the specified stream.
   *  @param[in] stream The source data stream to use.
   *  @param[in] flags The creation flags.
   *  @param[in] name The desired name of the texture.
   */
  static Texture* readInstance(Stream& stream,
                               unsigned int flags = DEFAULT,
			       const String& name = "");
  /*! Creates a texture from the specified image.
   *  @param[in] image The image data to use.
   *  @param[in] flags The creation flags.
   *  @param[in] name The desired name of the texture, or the empty string to
   *  automatically generate a name.
   */
  static Texture* createInstance(const Image& image,
                                 unsigned int flags = DEFAULT,
				 const String& name = "");
private:
  Texture(const String& name);
  Texture(const Texture& source);
  bool init(const Image& image, unsigned int flags);
  Texture& operator = (const Texture& source);
  GLenum textureTarget;
  GLuint textureID;
  GLint minFilter;
  GLint magFilter;
  GLint addressMode;
  unsigned int width;
  unsigned int height;
  unsigned int depth;
  unsigned int physicalWidth;
  unsigned int physicalHeight;
  unsigned int physicalDepth;
  unsigned int levelCount;
  unsigned int flags;
  ImageFormat format;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture unit state.
 *  @ingroup opengl
 *
 *  This class ties a specific texture (or no texture) to a specific texture
 *  unit, and can be said to represent a texture unit binding.
 *
 *  Note that it's valid for the texture to be @c NULL. This means that no
 *  texture is bound to the texture unit.
 *
 *  @remarks %Texture layers assigned to non-existent texture units will be
 *  silently ignored.
 *
 *  @remarks Unless you're writing your own custom renderer, you don't probably
 *  want to use this directly (especially the TextureLayer::apply method), but
 *  instead work through the Pass, render::Technique and render::Style classes.
 */
class TextureLayer
{
public:
  /*! Constructor.
   *  @param unit The texture unit to which the texture layer will apply.
   */
  TextureLayer(unsigned int unit);
  /*! Applies this texture layer to the texture unit it is bound to.
   */
  void apply(void) const;
  bool isCompatible(void) const;
  /*! @return The minification filter of this texture layer.
   */
  GLint getMinFilter(void) const;
  /*! @return The magnification filter of this texture layer.
   */
  GLint getMagFilter(void) const;
  /*! @return The addressing mode of this texture layer.
   */
  GLint getAddressMode(void) const;
  /*! @return The texture set for this texture layer, or @c NULL if no texture
   *  is set.
   */
  Texture* getTexture(void) const;
  /*! @return The name of the GLSL program sampler uniform that this texture
   *  layer binds to, or the empty string if it does not bind to any sampler
   *  uniform.
   *  @remarks This has no effect unless a GLSL program is in use.
   */
  const String& getSamplerName(void) const;
  /*! @return The texture unit to which this texture layer applies.
   */
  unsigned int getUnit(void) const;
  /*! Sets the minification and magnification filters for this texture layer.
   */
  void setFilters(GLint newMinFilter, GLint newMagFilter);
  /*! Sets the addressing mode for this texture layer.
   */
  void setAddressMode(GLint newMode);
  /*! Sets the texture used by this texture layer.
   *  @param[in] texture The texture used by this texture layer, or @c
   *  NULL to disable texturing for this layer.
   */
  void setTexture(Texture* texture);
  /*! Sets the name of the GLSL program sampler uniform that this texture layer
   *  binds to.
   *  @param newName The name of the sampler uniform, or the empty string to
   *  not bind to any sampler uniform.
   *  @remarks This has no effect unless a GLSL program is in use.
   */
  void setSamplerName(const String& newName);
  /*! Sets all values in this texture layer to their defaults.
   */
  void setDefaults(void);
  /*! @return The number of texture units supported by the current context,
   *  or zero if there is no current context.
   */
  static unsigned int getUnitCount(void);
private:
  void force(void) const;
  void applySampler(Texture& texture) const;
  void forceSampler(Texture& texture) const;
  void setBooleanState(GLenum state, bool value) const;
  static void onCreateContext(void);
  static void onDestroyContext(void);
  class Data
  {
  public:
    Data(void);
    void setDefaults(void);
    mutable bool dirty;
    Ref<Texture> texture;
    String samplerName;
    GLint minFilter;
    GLint magFilter;
    GLint addressMode;
  };
  typedef std::vector<Data> DataList;
  typedef std::vector<GLenum> TargetList;
  Data data;
  unsigned int unit;
  static DataList caches;
  static TargetList textureTargets;
  static unsigned int activeUnit;
  static unsigned int unitCount;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Texture layer stack object.
 *  @ingroup opengl
 *
 *  This class represents a state for the entire set of texture units (of which
 *  there may be only one, if multitexturing is unsupported).
 *
 *  @remarks You don't need to have as many texture layers as there are units
 *  in the current context.
 *
 *  @remarks Texture layers assigned to non-existent texture units will be
 *  silently ignored.
 *
 *  @remarks Unless you're writing your own custom renderer, you don't probably
 *  want to use this directly (especially the TextureStack::apply method), but
 *  instead work through the Pass class.
 */
class TextureStack
{
public:
  /*! Constructor.
   */
  TextureStack(void);
  /*! Applies this texture stack to the texture units for which it has layers.
   *  Additional texture units will be disabled.
   */
  void apply(void) const;
  /*! Creates a new texture layer in this layer stack.
   *  @return The newly created texture layer.
   */
  TextureLayer& createTextureLayer(void);
  /*! Destroys all texture layers in this layer stack.
   */
  void destroyTextureLayers(void);
  bool isCompatible(void) const;
  /*! @return The number of texture layers in this layer stack.
   */
  unsigned int getTextureLayerCount(void) const;
  /*! @param index The index of the desired texture layer.
   *  @return The texture layer at the specified index.
   */
  TextureLayer& getTextureLayer(unsigned int index);
  /*! @param index The index of the desired texture layer.
   *  @return The texture layer at the specified index.
   */
  const TextureLayer& getTextureLayer(unsigned int index) const;
private:
  typedef std::vector<TextureLayer> LayerList;
  LayerList layers;
  static LayerList defaults;
};

///////////////////////////////////////////////////////////////////////

class TextureCodec : public ResourceCodec<Texture>, public XML::Codec
{
public:
  TextureCodec(void);
  Texture* read(const Path& path, const String& name = "");
  Texture* read(Stream& stream, const String& name = "");
  bool write(const Path& path, const Texture& texture);
  bool write(Stream& stream, const Texture& texture);
private:
  bool onBeginElement(const String& name);
  bool onEndElement(const String& name);
  Ptr<Texture> texture;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLTEXTURE_H*/
///////////////////////////////////////////////////////////////////////
