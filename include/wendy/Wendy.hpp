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
#ifndef WENDY_WENDY_HPP
#define WENDY_WENDY_HPP
///////////////////////////////////////////////////////////////////////

/*! @mainpage Reference Documentation
 *
 *  @section intro Introduction
 *
 *  This is the reference documentation for the Wendy engine, a simple game
 *  engine.
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

#include <wendy/WendyCore.hpp>

#include <wendy/WendyGL.hpp>

#include <wendy/Window.hpp>

#if WENDY_INCLUDE_NETWORK
#include <wendy/WendyNetwork.hpp>
#endif

#if WENDY_INCLUDE_AUDIO
#include <wendy/WendyAudio.hpp>
#endif

#if WENDY_INCLUDE_RENDERER
#include <wendy/WendyRender.hpp>
#endif

#if WENDY_INCLUDE_SCENE_GRAPH
#include <wendy/SceneGraph.hpp>
#endif

#if WENDY_INCLUDE_UI_SYSTEM
#include <wendy/WendyUI.hpp>
#endif

#if WENDY_INCLUDE_DEBUG_UI
#include <wendy/DebugUI.hpp>
#endif

#if WENDY_INCLUDE_SQUIRREL
#include <wendy/WendySquirrel.hpp>
#endif

#if WENDY_INCLUDE_BULLET
#include <wendy/WendyBullet.hpp>
#endif

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_WENDY_HPP*/
///////////////////////////////////////////////////////////////////////
