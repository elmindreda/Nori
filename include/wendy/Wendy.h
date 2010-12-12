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
 *  This is the reference documentation for the Wendy engine, a simple engine
 *  using OpenGL. It is the hope of the author that it also serves as a
 *  suitable introduction to engine design, due to its (relatively) small size
 *  and (hopefully) consistent design.
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

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Block.h>
#include <wendy/Bimap.h>
#include <wendy/Signal.h>
#include <wendy/Timer.h>

#include <wendy/Color.h>
#include <wendy/Vector.h>
#include <wendy/Matrix.h>
#include <wendy/Quaternion.h>
#include <wendy/Transform.h>
#include <wendy/Random.h>

#include <wendy/AABB.h>
#include <wendy/Segment.h>
#include <wendy/Rectangle.h>
#include <wendy/Plane.h>
#include <wendy/Ray.h>
#include <wendy/Sphere.h>
#include <wendy/Triangle.h>
#include <wendy/Frustum.h>
#include <wendy/Bezier.h>

#include <wendy/Pattern.h>
#include <wendy/Pixel.h>
#include <wendy/Vertex.h>

#include <wendy/Path.h>
#include <wendy/Resource.h>
#include <wendy/XML.h>

#include <wendy/Animation.h>
#include <wendy/Image.h>
#include <wendy/Mesh.h>

#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>
#include <wendy/GLState.h>

#include <wendy/Input.h>

#if WENDY_INCLUDE_RENDERER
#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderQueue.h>
#include <wendy/RenderPool.h>
#include <wendy/RenderSprite.h>
#include <wendy/RenderFont.h>
#include <wendy/RenderParticle.h>
#include <wendy/RenderMesh.h>

#include <wendy/Deferred.h>
#endif

#if WENDY_INCLUDE_SCENE_GRAPH
#include <wendy/SceneGraph.h>
#endif

#if WENDY_INCLUDE_UI_SYSTEM
#include <wendy/UIRender.h>
#include <wendy/UIDesktop.h>
#include <wendy/UIWidget.h>
#include <wendy/UIScroller.h>
#include <wendy/UIBook.h>
#include <wendy/UICanvas.h>
#include <wendy/UILayout.h>
#include <wendy/UILabel.h>
#include <wendy/UIProgress.h>
#include <wendy/UIButton.h>
#include <wendy/UISlider.h>
#include <wendy/UIEntry.h>
#include <wendy/UIItem.h>
#include <wendy/UIList.h>
#include <wendy/UIMenu.h>
#include <wendy/UIPopup.h>
#include <wendy/UIColor.h>
#endif

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
