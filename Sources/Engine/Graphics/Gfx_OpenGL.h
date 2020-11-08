/* Copyright (c) 2002-2012 Croteam Ltd. 
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef SE_INCL_OPENGL_H
#define SE_INCL_OPENGL_H
#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "gl_types.h"

#if defined(WIN32) && !defined(__CYGWIN__)
#  if defined(__MINGW32__) && defined(GL_NO_STDCALL) || defined(UNDER_CE)  /* The generated DLLs by MingW with STDCALL are not compatible with the ones done by Microsoft's compilers */
#    define APIENTRY
#  else
#    ifndef WIN32_LEAN_AND_MEAN
#      define WIN32_LEAN_AND_MEAN 1
#    endif
#    include <windows.h>
#  endif
#elif defined(__CYGWIN__) && defined(USE_OPENGL32) /* use native windows opengl32 */
#  define APIENTRY __stdcall
#elif (defined(__GNUC__) && __GNUC__ >= 4) || (defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590))
#  define APIENTRY
#endif /* WIN32 && !CYGWIN */

#define DLLFUNCTION(output, name, inputs) \
  extern output (APIENTRY *p##name) inputs
#include "gl_functions.h"
#undef DLLFUNCTION

#ifdef WIN32
// check windows errors always
extern void WIN_CheckError(BOOL bRes, const char *strDescription);
#define WIN_CHECKERROR(result, string)   WIN_CheckError(result, string);
#else
#define WIN_CHECKERROR(result, string) ;
#endif

#endif  /* include-once check. */

