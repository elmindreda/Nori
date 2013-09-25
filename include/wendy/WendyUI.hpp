///////////////////////////////////////////////////////////////////////
// Wendy user interface library
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
#ifndef WENDY_WENDYUI_HPP
#define WENDY_WENDYUI_HPP
///////////////////////////////////////////////////////////////////////

/*! @defgroup ui User interface API
 */

///////////////////////////////////////////////////////////////////////

#if WENDY_INCLUDE_UI_SYSTEM

#include <wendy/UIDrawer.hpp>
#include <wendy/UILayer.hpp>
#include <wendy/UIWidget.hpp>
#include <wendy/UIScroller.hpp>
#include <wendy/UIBook.hpp>
#include <wendy/UICanvas.hpp>
#include <wendy/UILayout.hpp>
#include <wendy/UILabel.hpp>
#include <wendy/UIProgress.hpp>
#include <wendy/UIButton.hpp>
#include <wendy/UISlider.hpp>
#include <wendy/UIEntry.hpp>
#include <wendy/UIItem.hpp>
#include <wendy/UIList.hpp>
#include <wendy/UIMenu.hpp>
#include <wendy/UIPopup.hpp>

#else
#error "UI module not enabled"
#endif

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_WENDYUI_HPP*/
///////////////////////////////////////////////////////////////////////
