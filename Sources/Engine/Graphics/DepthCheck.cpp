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

#include <Engine/Base/Console.h>
#include <Engine/Graphics/GfxLibrary.h>
#include <Engine/Graphics/Raster.h>
#include <Engine/Graphics/ViewPort.h>

#include <Engine/Templates/StaticArray.h>
#include <Engine/Templates/StaticStackArray.h>


extern INDEX gap_iOptimizeDepthReads;

static INDEX _iCheckIteration = 0;
static CTimerValue _tvLast[8];  // 8 is max mirror recursion

#define KEEP_BEHIND 8

// info of one point for delayed depth buffer lookup
struct DepthInfo {
  INDEX di_iID;               // unique identifier
  PIX   di_pixI, di_pixJ;     // last requested coordinates
  FLOAT di_fOoK;              // last requested depth
  INDEX di_iSwapLastRequest;  // index of swap when last requested
  INDEX di_iMirrorLevel;      // level of mirror recursion in which flare is
  BOOL  di_bVisible;          // whether the point was visible
};
CStaticStackArray<DepthInfo> _adiDelayed;  // active delayed points
// don't ask, these are  for D3D
CStaticStackArray<CTVERTEX> _avtxDelayed;  
CStaticStackArray<COLOR>    _acolDelayed;  

// read depth buffer and update visibility flag of depth points
static void UpdateDepthPointsVisibility( const CDrawPort *pdp, const INDEX iMirrorLevel,
                                         DepthInfo *pdi, const INDEX ctCount)
{
  const GfxAPIType eAPI = _pGfx->gl_eCurrentAPI;
  ASSERT(eAPI == GAT_OGL || eAPI == GAT_NONE);
  ASSERT( pdp!=NULL && ctCount>0);
  const CRaster *pra = pdp->dp_Raster;

  // OpenGL
  if( eAPI==GAT_OGL)
  {
    FLOAT fPointOoK;
    // for each stored point
    for( INDEX idi=0; idi<ctCount; idi++) {
      DepthInfo &di = pdi[idi];
      // skip if not in required mirror level or was already checked in this iteration
      if( iMirrorLevel!=di.di_iMirrorLevel || _iCheckIteration!=di.di_iSwapLastRequest) continue;
      const PIX pixJ = pra->ra_Height-1 - di.di_pixJ; // OpenGL has Y-inversed buffer!
      pglReadPixels( di.di_pixI, pixJ, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &fPointOoK);
      OGL_CHECKERROR;
      // it is visible if there is nothing nearer in z-buffer already
      di.di_bVisible = (di.di_fOoK<fPointOoK);
    }
    // done
    return;
  }
}



// check point against depth buffer
extern BOOL CheckDepthPoint( const CDrawPort *pdp, PIX pixI, PIX pixJ, FLOAT fOoK, INDEX iID, INDEX iMirrorLevel/*=0*/)
{
  // no raster?
  const CRaster *pra = pdp->dp_Raster;
  if( pra==NULL) return FALSE;
  // almoust out of raster?
  pixI += pdp->dp_MinI;
  pixJ += pdp->dp_MinJ;
  if( pixI<1 || pixJ<1 || pixI>pra->ra_Width-2 || pixJ>pra->ra_Height-2) return FALSE;

  // if shouldn't delay
  if( gap_iOptimizeDepthReads==0) {
    // just check immediately
    DepthInfo di = { iID, pixI, pixJ, fOoK, _iCheckIteration, iMirrorLevel, FALSE };
    UpdateDepthPointsVisibility( pdp, iMirrorLevel, &di, 1);
    return di.di_bVisible;
  }

  // for each stored point
  for( INDEX idi=0; idi<_adiDelayed.Count(); idi++) {
    DepthInfo &di = _adiDelayed[idi];
    // if same id
    if( di.di_iID == iID) {
      // remember parameters
      di.di_pixI = pixI;
      di.di_pixJ = pixJ;
      di.di_fOoK = fOoK;
      di.di_iSwapLastRequest = _iCheckIteration;
      // return visibility
      return di.di_bVisible;
    }
  }
  // if not found...

  // create new one
  DepthInfo &di = _adiDelayed.Push();
  // remember parameters
  di.di_iID  = iID;
  di.di_pixI = pixI;
  di.di_pixJ = pixJ;
  di.di_fOoK = fOoK;
  di.di_iSwapLastRequest = _iCheckIteration;
  di.di_iMirrorLevel = iMirrorLevel;
  di.di_bVisible = FALSE;
  // not visible by default
  return FALSE;
}


// check all delayed depth points
extern void CheckDelayedDepthPoints( const CDrawPort *pdp, INDEX iMirrorLevel/*=0*/)
{
  // skip if not delayed or mirror level is to high
  gap_iOptimizeDepthReads = Clamp( gap_iOptimizeDepthReads, 0, 2);
  if( gap_iOptimizeDepthReads==0 || iMirrorLevel>7) return; 
  ASSERT( pdp!=NULL && iMirrorLevel>=0);

  // check only if time lapse allows
  const CTimerValue tvNow = _pTimer->GetHighPrecisionTimer();
  const TIME tmDelta = (tvNow-_tvLast[iMirrorLevel]).GetSeconds();
  ASSERT( tmDelta>=0);
  if( gap_iOptimizeDepthReads==2 && tmDelta<0.1f) return;

  // prepare
  _tvLast[iMirrorLevel] = tvNow;
  INDEX ctPoints = _adiDelayed.Count();
  if( ctPoints==0) return; // done if no points in queue

  // for each point
  INDEX iPoint = 0;
  while( iPoint<ctPoints) {
    DepthInfo &di = _adiDelayed[iPoint];
    // if the point is not active any more
    if( iMirrorLevel==di.di_iMirrorLevel && di.di_iSwapLastRequest<_iCheckIteration-KEEP_BEHIND) {
      // delete it by moving the last one on its place
      di = _adiDelayed[ctPoints-1];
      ctPoints--;
    // if the point is still active
    } else {
      // go to next point
      iPoint++;
    }
  }

  // remove unused points at the end
  if( ctPoints==0) _adiDelayed.PopAll();
  else _adiDelayed.PopUntil(ctPoints-1);

  // ignore stalls
  if( tmDelta>1.0f) return;

  // check and upadete visibility of what has left
  ASSERT( ctPoints == _adiDelayed.Count());
  if( ctPoints>0) UpdateDepthPointsVisibility( pdp, iMirrorLevel, &_adiDelayed[0], ctPoints);
  // mark checking
  _iCheckIteration++;
}

