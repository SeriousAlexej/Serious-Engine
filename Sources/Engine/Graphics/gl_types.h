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

#ifndef SE_INCL_GL_TYPES_H
#define SE_INCL_GL_TYPES_H
#ifdef PRAGMA_ONCE
  #pragma once
#endif

using GLint      = std::int32_t;
using GLsizei    = std::int32_t;
using GLuint     = std::uint32_t;
using GLenum     = std::uint32_t;
using GLbitfield = std::uint32_t;
using GLubyte    = std::uint8_t;
using GLfloat    = float;
using GLdouble   = double;
using GLboolean  = unsigned char;
using GLvoid     = void;
using GLchar     = char;

#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_DEPTH_COMPONENT                0x1902
#define GL_FLOAT                          0x1406
#define GL_TEXTURE0                       0x84C0
#define GL_SCISSOR_TEST                   0x0C11
#define GL_RGB                            0x1907
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_TEXTURE_2D                     0x0DE1
#define GL_ALPHA_TEST                     0x0BC0
#define GL_COMPILE_STATUS                 0x8B81
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_REPEAT                         0x2901
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_RGBA                           0x1908
#define GL_RGBA8                          0x8058
#define GL_RGB8                           0x8051
#define GL_RGB5_A1                        0x8057
#define GL_RGB5                           0x8050
#define GL_RGB4                           0x804F
#define GL_RGBA4                          0x8056
#define GL_LUMINANCE8                     0x8040
#define GL_LUMINANCE_ALPHA                0x190A
#define GL_LUMINANCE8_ALPHA8              0x8045
#define GL_TEXTURE_INTERNAL_FORMAT        0x1003
#define GL_DITHER                         0x0BD0
#define GL_BLEND                          0x0BE2
#define GL_DEPTH_TEST                     0x0B71
#define GL_CLIP_PLANE0                    0x3000
#define GL_CULL_FACE                      0x0B44
#define GL_CW                             0x0900
#define GL_CCW                            0x0901
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308
#define GL_NONE                           0
#define GL_SMOOTH                         0x1D01
#define GL_BACK                           0x0405
#define GL_TEXTURE                        0x1702
#define GL_VERTEX_ARRAY                   0x8074
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03
#define GL_MAX_TEXTURE_UNITS_ARB          0x84E2
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_TEXTURE_LOD_BIAS_EXT       0x84FD
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#define GL_TEXTURE_RED_SIZE               0x805C
#define GL_TEXTURE_GREEN_SIZE             0x805D
#define GL_TEXTURE_BLUE_SIZE              0x805E
#define GL_TEXTURE_ALPHA_SIZE             0x805F
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400

#endif  /* include-once check. */

