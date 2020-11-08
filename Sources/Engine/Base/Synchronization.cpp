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

#include <Engine/Base/Synchronization.h>

thread_local INDEX _iLastLockedMutex = 0;

CTCriticalSection::CTCriticalSection(void)
  : m_recurse_count(0)
  , cs_iIndex(-2)
{
}
CTCriticalSection::~CTCriticalSection(void)
{
}
INDEX CTCriticalSection::Lock(void)
{
  m_mutex.lock();
  m_recurse_count++;
  return m_recurse_count;
}
INDEX CTCriticalSection::TryToLock(void)
{
  if (m_mutex.try_lock())
  {
    m_recurse_count++;
    return m_recurse_count;
  }
  return 0;
}
INDEX CTCriticalSection::Unlock(void)
{
  m_recurse_count--;
  m_mutex.unlock();
  return m_recurse_count;
}

CTSingleLock::CTSingleLock(CTCriticalSection *pcs, BOOL bLock) : sl_cs(*pcs)
{
  // initially not locked
  sl_bLocked = FALSE;
  sl_iLastLockedIndex = -2;
  // critical section must have index assigned
  ASSERT(sl_cs.cs_iIndex>=1||sl_cs.cs_iIndex==-1);
  // if should lock immediately
  if (bLock) {
    Lock();
  }
}
CTSingleLock::~CTSingleLock(void)
{
  // if locked
  if (sl_bLocked) {
    // unlock
    Unlock();
  }
}
void CTSingleLock::Lock(void)
{
  // must not be locked
  ASSERT(!sl_bLocked);
  ASSERT(sl_iLastLockedIndex==-2);

  // if not locked
  if (!sl_bLocked) {
    // lock
    INDEX ctLocks = sl_cs.Lock();
    // if this mutex was not locked already
    if (ctLocks==1) {
      // check that locking in given order
      if (sl_cs.cs_iIndex!=-1) {
        ASSERT(_iLastLockedMutex<sl_cs.cs_iIndex);
        sl_iLastLockedIndex = _iLastLockedMutex;
        _iLastLockedMutex = sl_cs.cs_iIndex;
      }
    }
  }
  sl_bLocked = TRUE;
}

BOOL CTSingleLock::TryToLock(void)
{
  // must not be locked
  ASSERT(!sl_bLocked);
  // if not locked
  if (!sl_bLocked) {
    // if can lock
    INDEX ctLocks = sl_cs.TryToLock();
    if (ctLocks>=1) {
      sl_bLocked = TRUE;

      // if this mutex was not locked already
      if (ctLocks==1) {
        // check that locking in given order
        if (sl_cs.cs_iIndex!=-1) {
          ASSERT(_iLastLockedMutex<sl_cs.cs_iIndex);
          sl_iLastLockedIndex = _iLastLockedMutex;
          _iLastLockedMutex = sl_cs.cs_iIndex;
        }
      }
    }
  }
  return sl_bLocked;
}
BOOL CTSingleLock::IsLocked(void)
{
  return sl_bLocked;
}

void CTSingleLock::Unlock(void)
{
  // must be locked
  ASSERT(sl_bLocked);
  // if locked
  if (sl_bLocked) {
    // unlock
    INDEX ctLocks = sl_cs.Unlock();
    // if unlocked completely
    if (ctLocks==0) {
      // check that unlocking in exact reverse order
      if (sl_cs.cs_iIndex!=-1) {
        ASSERT(_iLastLockedMutex==sl_cs.cs_iIndex);
        _iLastLockedMutex = sl_iLastLockedIndex;
        sl_iLastLockedIndex = -2;
      }
    }
  }
  sl_bLocked = FALSE;
}
