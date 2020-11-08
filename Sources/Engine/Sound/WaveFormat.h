/* Copyright (c) 2002-2012 Croteam Ltd.
   Copyright (c) 2020 Oleksii Sierov
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

#ifndef SE_INCL_WAVE_FORMAT_H
#define SE_INCL_WAVE_FORMAT_H
#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Engine/Base/Types.h>

struct SAM_WAVEFORMATEX
{
  UWORD wFormatTag;         /* format type */
  UWORD nChannels;          /* number of channels (i.e. mono, stereo...) */
  ULONG nSamplesPerSec;     /* sample rate */
  ULONG nAvgBytesPerSec;    /* for buffer estimation */
  UWORD nBlockAlign;        /* block size of data */
  UWORD wBitsPerSample;     /* number of bits per sample of mono data */
  UWORD cbSize;             /* the count in bytes of the size of */
};

#endif  /* include-once check. */
