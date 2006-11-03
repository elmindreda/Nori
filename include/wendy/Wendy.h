///////////////////////////////////////////////////////////////////////
// Wendy library
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
#ifndef WENDY_WENDY_H
#define WENDY_WENDY_H
///////////////////////////////////////////////////////////////////////

/*! @mainpage Reference Documentation
 *
 *  @section intro Introduction
 *
 *  This is the reference documentation for the Wendy engine. The Wendy engine
 *  is a simple rendering engine built on top of OpenGL and intended for the
 *  production of demos and small games. It is also the hope of the author that
 *  it serves as a suitable introduction to rendering engine design, due to its
 *  small size and consistent design.
 *
 *  The Wendy engine is built on top of the Moira utility library, which
 *  provides resource management, math classes, animation, image and mesh data
 *  management, generalised I/O, signals and more.
 *
 *  @section overview Overview
 *
 *  Some notable features of Wendy are:
 *
 *  - Multipass @link wendy::GL::RenderStyle render style @endlink descriptors
 *    with support for multitexturing, ARB programs and GLSL shaders.
 *  - Extensible @link wendy::GL::ParticleSystem particle system @endlink with
 *    generic emitters and affectors.
 *  - @link wendy::GL::RenderQueue Render queue @endlink system with automatic
 *    sorting.
 *  - Basic @link scene scene graph @endlink management.
 *  - Hierarchial @link demo demo effect system @endlink with local time.
 *  - Extensible @link ui user interface library @endlink and supporting 2D
 *    rendering interface.
 *  - @link opengl OpenGL wrapper API @endlink, providing automatic resource
 *    management and many useful helper methods, in case you need to implement
 *    custom rendering.
 *
 *  @section license License
 *
 *  This software is provided 'as-is', without any express or implied warranty.
 *  In no event will the authors be held liable for any damages arising from
 *  the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *   -# The origin of this software must not be misrepresented; you
 *      must not claim that you wrote the original software. If you use
 *      this software in a product, an acknowledgment in the product
 *      documentation would be appreciated but is not required.
 *   -# Altered source versions must be plainly marked as such, and
 *      must not be misrepresented as being the original software.
 *   -# This notice may not be removed or altered from any source
 *      distribution.
 */

///////////////////////////////////////////////////////////////////////

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/Core.h>

#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLCamera.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLShader.h>
#include <wendy/GLLight.h>
#include <wendy/GLRender.h>
#include <wendy/GLFont.h>
#include <wendy/GLParticle.h>
#include <wendy/GLMesh.h>
#include <wendy/GLTerrain.h>
#include <wendy/GLSprite.h>
#include <wendy/GLScene.h>

#include <wendy/GLShaderIO.h>
#include <wendy/GLRenderIO.h>
#include <wendy/GLRenderStages.h>

#include <wendy/UIRender.h>
#include <wendy/UIWidget.h>
#include <wendy/UIWindow.h>
#include <wendy/UILayout.h>
#include <wendy/UILabel.h>
#include <wendy/UIButton.h>
#include <wendy/UISlider.h>
#include <wendy/UIEntry.h>
#include <wendy/UIList.h>

#include <wendy/DemoEffect.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief Initializes libraries and creates the default objects.
 *
 *  Initializes lower-level libraries and sets up the default objects,
 *  such as instances of all default resource codecs.  You must call
 *  this before creating an OpenGL context.
 *
 *  @remarks If you do call it, it's considered polite to call
 *  wendy::shutdown as well.
 */
bool initialize(void);

/*! @brief Destroys the default objects.
 *
 *  Destroys all default objects created by wendy::initialize.
 */
void shutdown(void);

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_WENDY_H*/
///////////////////////////////////////////////////////////////////////
