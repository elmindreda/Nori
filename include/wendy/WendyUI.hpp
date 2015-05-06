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

/*! @defgroup ui User interface API
 */

#if NORI_INCLUDE_UI_SYSTEM

#include <nori/Drawer.hpp>
#include <nori/Layer.hpp>
#include <nori/Widget.hpp>
#include <nori/Scroller.hpp>
#include <nori/Book.hpp>
#include <nori/Canvas.hpp>
#include <nori/Layout.hpp>
#include <nori/Label.hpp>
#include <nori/Progress.hpp>
#include <nori/Button.hpp>
#include <nori/Slider.hpp>
#include <nori/Entry.hpp>
#include <nori/Item.hpp>
#include <nori/List.hpp>
#include <nori/Menu.hpp>
#include <nori/Popup.hpp>

#include <nori/WidgetReader.hpp>

#else
#error "UI module not enabled"
#endif

