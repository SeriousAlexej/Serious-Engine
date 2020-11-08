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



#include <Engine/Base/Memory.h>
#include <Engine/Base/Translation.h>

#include <Engine/Base/ErrorReporting.h>
#include <cstdlib>
#include <cstring>
#include <new>

// if not enough memory
void NewHandler()
{
  // terminate program
  FatalError(TRANS("Not enough memory!"));
}

/* Static class used for initializing memory handlers. */
static class CMemHandlerInit {
public:
  // constructor
  CMemHandlerInit(void);
} MemHandlerInit;

CMemHandlerInit::CMemHandlerInit(void)
{
  // set our not-enough-memory handler
  std::set_new_handler(NewHandler);
}

#undef AllocMemory

void *AllocMemory(SLONG memsize )
{
  void *pmem;
  ASSERTMSG(memsize>0, "AllocMemory: Block size is less or equal zero.");

  pmem = std::malloc( memsize);
  // memory handler asures no null results here?!
  if (pmem==NULL) {
    FatalError(TRANS("Not enough memory (%d bytes needed)!"), memsize);
  }
  return pmem;
}

void *AllocMemoryAligned( SLONG memsize, SLONG slAlignPow2)
{
  void *pmem;
  ASSERTMSG(memsize>0, "AllocMemory: Block size is less or equal zero.");

  pmem = std::aligned_alloc(slAlignPow2, memsize);
  // memory handler asures no null results here?!
  if (pmem==NULL) {
    FatalError(TRANS("Not enough memory (%d bytes needed)!"), memsize);
  }
  return pmem;
}
void FreeMemoryAligned(void *memory)
{
  ASSERTMSG(memory!=NULL, "FreeMemory: NULL pointer input.");
  std::free(memory);
}

void FreeMemory(void *memory )
{
  ASSERTMSG(memory!=NULL, "FreeMemory: NULL pointer input.");
  std::free(memory);
}

void ResizeMemory( void **ppv, SLONG slSize )
{
  void *pv = realloc(*ppv, slSize);
  // memory handler asures no null results here?!
  if (pv==NULL) {
    FatalError(TRANS("Not enough memory (%d bytes needed)!"), slSize);
  }
  *ppv = pv;
}

void GrowMemory( void **ppv, SLONG newSize )
{
  ResizeMemory(ppv, newSize);
}

void ShrinkMemory( void **ppv, SLONG newSize )
{
  ResizeMemory(ppv, newSize);
}


/*
 * Allocate a copy of a string. - fatal error if not enough memory.
 */
char *StringDuplicate(const char *strOriginal) {
  // get the size
  SLONG slSize = std::strlen(strOriginal)+1;
  // allocate that much memory
  char *strCopy = (char *)AllocMemory(slSize);
  // copy it there
  std::memcpy(strCopy, strOriginal, slSize);
  // result is the pointer to the copied string
  return strCopy;
}

// return position where we encounter zero byte or iBytes
INDEX FindZero( UBYTE *pubMemory, INDEX iBytes)
{
  for( INDEX i=0; i<iBytes; i++) if( pubMemory[i]==0) return i;
  return iBytes;
}
