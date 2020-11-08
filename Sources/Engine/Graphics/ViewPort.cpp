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

#include <Engine/Graphics/ViewPort.h>

#include <Engine/Graphics/GfxLibrary.h>
#include <Engine/Base/Statistics_Internal.h>


/*
 *   ViewPort functions
 */

CViewPort::CViewPort( PIX pixWidth, PIX pixHeight)
  : vp_fbWnd(0)
  , vp_Raster( pixWidth, pixHeight, 0)
{
  vp_ctDisplayChanges = 0;
  vp_Raster.ra_pvpViewPort = this;
}


CViewPort::~CViewPort(void)
{
  CloseCanvas(TRUE);
  // reset current viewport if needed
  if( _pGfx->gl_pvpActive==this) _pGfx->gl_pvpActive = NULL;
}

// open overlaid window for rendering context
void CViewPort::OpenCanvas(void)
{
  also compare vp_ctDisplayChanges = gl_ctDriverChanges;
  assert that gfx api is created here!!!
  // do nothing if not feasable
  if( vp_hWnd!=NULL || vp_hWndParent==NULL) return;

  // register class
  if( !_bClassRegistered) {
    WNDCLASSA wc;
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = CViewPortCLASS_WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = NULL;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor( NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CViewPortCLASS;
    RegisterClassA(&wc);
    _bClassRegistered = TRUE;
  }
  
  // determine window and desktopsize
	RECT rectWindow;
	GetClientRect( vp_hWndParent, &rectWindow);
	const PIX pixWinSizeI = rectWindow.right  - rectWindow.left;
	const PIX pixWinSizeJ = rectWindow.bottom - rectWindow.top;
  CDisplayMode dm;
  _pGfx->GetCurrentDisplayMode(dm);
  ASSERT( (dm.dm_pixSizeI==0 && dm.dm_pixSizeJ==0) || (dm.dm_pixSizeI!=0 && dm.dm_pixSizeJ!=0));
  const BOOL bFullScreen = (dm.dm_pixSizeI==pixWinSizeI && dm.dm_pixSizeJ==pixWinSizeJ);

  // set fullscreen attribs if window size is equal to screen size
  DWORD dwExStyle = NONE;
  DWORD dwStyle   = WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;

  // set child window
  vp_hWnd = ::CreateWindowExA(
	  dwExStyle,
	  CViewPortCLASS,
	  "",   // title
    dwStyle,
	  0,0,
	  0,0,  // window size
	  vp_hWndParent,
	  NULL,
	  (HINSTANCE)GetWindowLong(vp_hWndParent, GWL_HINSTANCE),
	  NULL);
  ASSERT( vp_hWnd!=NULL);

  // resize raster
  Resize();
  ShowWindow( vp_hWnd, SW_SHOW);
}


// close overlaid window
void CViewPort::CloseCanvas( BOOL bRelease/*=FALSE*/)
{
  also compare vp_ctDisplayChanges = gl_ctDriverChanges;
  assert that gfx api is created here!!!
  // destroy window
  if( vp_hWnd!=NULL && IsWindow(vp_hWnd)) { 
    BOOL bRes = DestroyWindow(vp_hWnd);
    ASSERT(bRes);
  }
  // mark
  vp_hWnd = NULL;
}


// Change size of this viewport, it's raster and all it's drawports
void CViewPort::Resize(void)
{
	PIX pixNewWidth, pixNewHeight;
	RECT rectWindow;

	// get the size of the window
	GetClientRect( vp_hWndParent, &rectWindow);
	pixNewWidth  = rectWindow.right  - rectWindow.left;
	pixNewHeight = rectWindow.bottom - rectWindow.top;

  // resize child window
  ASSERT( vp_hWnd!=NULL);
  SetWindowPos( vp_hWnd, NULL, 0,0, pixNewWidth, pixNewHeight, SWP_NOZORDER|SWP_NOMOVE);

  // resize the raster
  vp_Raster.Resize( pixNewWidth, pixNewHeight);

  // "resize" D3D surface (if any)
#ifdef SE1_D3D
  if( _pGfx->gl_eCurrentAPI==GAT_D3D && vp_pSwapChain!=NULL) {
    // release old surface
    ASSERT( vp_pSurfDepth!=NULL);
    D3DRELEASE( vp_pSwapChain, TRUE);
    D3DRELEASE( vp_pSurfDepth, TRUE);
    // create a new one and set it as current
    CreateSwapChain_D3D( this, pixNewWidth, pixNewHeight);
    SetAsRenderTarget_D3D(this);
  }
#endif // SE1_D3D
}


void CViewPort::SwapBuffers(void)
{
  // skip if child window not present
  if( vp_hWnd==NULL) return;

  // ask the current driver to swap buffers
  _sfStats.StartTimer(CStatForm::STI_SWAPBUFFERS);
  _pfGfxProfile.StartTimer( CGfxProfile::PTI_SWAPBUFFERS);
  _pfGfxProfile.IncrementAveragingCounter(1);
  _pGfx->SwapBuffers(this);
  _pfGfxProfile.StopTimer( CGfxProfile::PTI_SWAPBUFFERS);
  _sfStats.StopTimer(CStatForm::STI_SWAPBUFFERS);
}

