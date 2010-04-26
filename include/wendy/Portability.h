///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_PORTABILITY_H
#define WENDY_PORTABILITY_H
///////////////////////////////////////////////////////////////////////

#if WENDY_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if WENDY_HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef _MSC_VER

// Don't consider the libc to be obsolete
#pragma warning( disable: 4996 )

#define _USE_MATH_DEFINES

#define snprintf _snprintf
#define strcasecmp stricmp

// This is only needed for versions below Visual C++ 2008
#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif

#define F_OK 0
#define W_OK 2
#define R_OK 4

#define S_ISDIR(x) ((x) & _S_IFDIR)
#define S_ISREG(x) ((x) & _S_IFREG)

typedef __int64 off64_t;

float fminf(float x, float y);
float fmaxf(float x, float y);
float log2f(float x);

#endif /*_MSC_VER*/

#if !WENDY_HAVE_VASPRINTF
int vasprintf(char** result, const char* format, va_list vl);
#endif /*WENDY_HAVE_VASPRINTF*/

#if !WENDY_HAVE_STRTOF
float strtof(const char* nptr, char** endptr);
#endif /*WENDY_HAVE_STRTOF*/

#if !WENDY_HAVE_SOCKLEN_T
/*typedef int socklen_t;*/
#endif /*WENDY_HAVE_SOCKLEN_T*/

#if !WENDY_HAVE_STRLCAT
size_t strlcat(char* target, const char* source, size_t size);
#endif /*WENDY_HAVE_STRLCAT*/

#if !WENDY_HAVE_STRLCPY
size_t strlcpy(char* target, const char* source, size_t size);
#endif /*WENDY_HAVE_STRLCPY*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_PORTABILITY_H*/
///////////////////////////////////////////////////////////////////////
