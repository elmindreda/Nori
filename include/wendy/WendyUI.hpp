///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#if WENDY_INCLUDE_UI_SYSTEM

#include <wendy/Drawer.hpp>
#include <wendy/Layer.hpp>
#include <wendy/Widget.hpp>
#include <wendy/Scroller.hpp>
#include <wendy/Book.hpp>
#include <wendy/Canvas.hpp>
#include <wendy/Layout.hpp>
#include <wendy/Label.hpp>
#include <wendy/Progress.hpp>
#include <wendy/Button.hpp>
#include <wendy/Slider.hpp>
#include <wendy/Entry.hpp>
#include <wendy/Item.hpp>
#include <wendy/List.hpp>
#include <wendy/Menu.hpp>
#include <wendy/Popup.hpp>

#include <wendy/WidgetReader.hpp>

#else
#error "UI module not enabled"
#endif

