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

#include <wendy/WendyCore.h>

#include <wendy/OpenGL.h>
#include <wendy/GLQuery.h>
#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>
#include <wendy/GLState.h>

#include <wendy/Input.h>

#if WENDY_INCLUDE_OPENAL
#include <wendy/OpenAL.h>
#include <wendy/ALContext.h>
#include <wendy/ALBuffer.h>
#include <wendy/ALSource.h>
#endif

#if WENDY_INCLUDE_RENDERER
#include <wendy/RenderState.h>
#include <wendy/RenderCamera.h>
#include <wendy/RenderMaterial.h>
#include <wendy/RenderLight.h>
#include <wendy/RenderScene.h>
#include <wendy/RenderPool.h>
#include <wendy/RenderSprite.h>
#include <wendy/RenderFont.h>
#include <wendy/RenderModel.h>

#include <wendy/Forward.h>
#include <wendy/Deferred.h>
#endif

#if WENDY_INCLUDE_SCENE_GRAPH
#include <wendy/SceneGraph.h>
#endif

#if WENDY_INCLUDE_UI_SYSTEM
#include <wendy/UIDrawer.h>
#include <wendy/UILayer.h>
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
#endif

#if WENDY_INCLUDE_DEBUG_UI
#include <wendy/DebugUI.h>
#endif

#if WENDY_INCLUDE_SQUIRREL
#include <wendy/Squirrel.h>
#endif

#if WENDY_INCLUDE_BULLET
#include <wendy/Bullet.h>
#endif

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_WENDY_H*/
///////////////////////////////////////////////////////////////////////
