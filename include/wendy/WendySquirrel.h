///////////////////////////////////////////////////////////////////////
// Wendy Squirrel bindings - based on Sqrat
// Copyright (c) 2009 Brandon Jones
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
#ifndef WENDY_WENDYSQUIRREL_H
#define WENDY_WENDYSQUIRREL_H
///////////////////////////////////////////////////////////////////////

/*! @defgroup squirrel Squirrel bindings
 *
 *  These classes provide an easy way to bind C++ to Squirrel.  The design is
 *  based on Sqrat and borrows many clever tricks from it, but it is smaller
 *  and lacks some of the features of Sqrat.
 */

///////////////////////////////////////////////////////////////////////

#if WENDY_INCLUDE_SQUIRREL

#include <wendy/Squirrel.h>

#else
#error "Squirrel module not enabled"
#endif

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_WENDYSQUIRREL_H*/
///////////////////////////////////////////////////////////////////////
