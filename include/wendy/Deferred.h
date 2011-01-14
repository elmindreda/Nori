///////////////////////////////////////////////////////////////////////
// Wendy deferred renderer
// Copyright (c) 2010 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_DEFERRED_H
#define WENDY_DEFERRED_H
///////////////////////////////////////////////////////////////////////

#include <wendy/RenderQueue.h>
#include <wendy/RenderPool.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace deferred
  {

///////////////////////////////////////////////////////////////////////

class Config
{
public:
  Config(unsigned int width, unsigned int height);
  unsigned int width;
  unsigned int height;
};

///////////////////////////////////////////////////////////////////////

class Renderer
{
public:
  void render(const render::Queue& queue);
  void renderAmbientLight(const render::Camera& camera, const ColorRGB& color);
  void renderLight(const render::Camera& camera, const render::Light& light);
  GL::Texture& getColorTexture(void) const;
  GL::Texture& getNormalTexture(void) const;
  GL::Texture& getDepthTexture(void) const;
  static Renderer* create(render::GeometryPool& pool, const Config& config);
private:
  Renderer(render::GeometryPool& pool);
  bool init(const Config& config);
  void renderLightQuad(const render::Camera& camera);
  render::GeometryPool& pool;
  Ref<GL::ImageCanvas> canvas;
  GL::TextureRef depthTexture;
  GL::TextureRef colorTexture;
  GL::TextureRef normalTexture;
  GL::RenderState dirLightPass;
  GL::RenderState pointLightPass;
  GL::RenderState ambientLightPass;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace deferred*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_DEFERRED_H*/
///////////////////////////////////////////////////////////////////////
