///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

/*! @mainpage Reference Documentation
 *
 *  @section intro Introduction
 *
 *  This is the reference documentation for the Nori engine, a simple game
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

#include <nori/NoriCore.hpp>

#if NORI_INCLUDE_NETWORK
#include <nori/NoriNetwork.hpp>
#endif

#if NORI_INCLUDE_AUDIO
#include <nori/NoriAudio.hpp>
#endif

#if NORI_INCLUDE_RENDERER
#include <nori/NoriRender.hpp>
#endif

#if NORI_INCLUDE_UI_SYSTEM
#include <nori/NoriUI.hpp>
#endif

#if NORI_INCLUDE_DEBUG_UI
#include <nori/DebugUI.hpp>
#endif

#if NORI_INCLUDE_SQUIRREL
#include <nori/NoriSquirrel.hpp>
#endif

#if NORI_INCLUDE_BULLET
#include <nori/NoriBullet.hpp>
#endif

