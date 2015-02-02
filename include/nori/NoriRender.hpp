///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
// Copyright (c) 2011 Camilla Berglund <elmindreda@elmindreda.org>
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

/*! @defgroup renderer 3D rendering pipeline
 */

#if NORI_INCLUDE_RENDERER

#include <nori/Window.hpp>

#include <nori/Query.hpp>
#include <nori/Texture.hpp>
#include <nori/RenderBuffer.hpp>
#include <nori/Program.hpp>
#include <nori/RenderContext.hpp>
#include <nori/Pass.hpp>
#include <nori/Material.hpp>
#include <nori/RenderQueue.hpp>
#include <nori/Sprite.hpp>
#include <nori/Model.hpp>
#include <nori/Scene.hpp>
#include <nori/Renderer.hpp>

#else
#error "Render module not enabled"
#endif

